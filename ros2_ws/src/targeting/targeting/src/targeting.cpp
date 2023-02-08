#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "targeting_interfaces/srv/get_channel_voltages.hpp"

#include <fstream>
#include <string>

using namespace std;

const uint8_t N_CHANNELS = 5;

/* Allow targeting inside the area x \in [-15, ..., 15], y \in [-15, ..., 15] (in millimeters) and
 * rotating the e-field within the angles [0, ..., 359].
 *
 * The maximum stimulation intensity is 120 V/m - an arbitrary value which can be changed if desired,
 * but gives some added safety, compared to not having the targeting restrict the stimulation intensity.
 * (The maximum capacitor voltages constrain the stimulation intensity regardless of this additional
 * constraint implemented by the targeting.)
 */

const uint8_t MAX_ABSOLUTE_DISPLACEMENT = 15;
const uint16_t MAX_ROTATION_ANGLE = 359;
const uint16_t MAX_INTENSITY = 120;

const uint8_t N_DISPLACEMENTS = 2 * MAX_ABSOLUTE_DISPLACEMENT + 1;
const uint16_t N_ROTATION_ANGLES = MAX_ROTATION_ANGLE + 1;

class Target {

public:
  Target() : initialized(false) {}

  void initialize(double voltages[]) {
    for (int i = 0; i < N_CHANNELS; i++) {
      this->voltages[i] = abs(voltages[i]);
      this->reversed_polarities[i] = voltages[i] < 0;
    }
    this->initialized = true;
  }

  double* get_voltages() {
    return voltages;
  }

  bool* get_reversed_polarities() {
    return reversed_polarities;
  }

  bool is_initialized() {
    return initialized;
  }

private:
  double voltages[N_CHANNELS];
  bool reversed_polarities[N_CHANNELS];
  bool initialized;
};

class Targeting : public rclcpp::Node {

public:
  Targeting() : Node("targeting") {

    auto service_callback = [this](
        const std::shared_ptr<targeting_interfaces::srv::GetChannelVoltages::Request> request,
        std::shared_ptr<targeting_interfaces::srv::GetChannelVoltages::Response> response) -> void {
      int8_t displacement_x = request->displacement_x;
      int8_t displacement_y = request->displacement_y;
      uint16_t rotation_angle = request->rotation_angle;
      uint8_t intensity = request->intensity;

      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Request received: (x, y, angle, intensity) = (%d, %d, %d, %d)",
        displacement_x, displacement_y, rotation_angle, intensity);

      response->success = validate_request(displacement_x, displacement_y, rotation_angle, intensity);

      if (response->success) {
        Target target = targets[displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle];

        double* voltages = target.get_voltages();
        bool* reversed_polarities = target.get_reversed_polarities();

        for (int i = 0; i < N_CHANNELS; i++) {
          response->voltages.push_back(voltages[i] * intensity);
          response->reversed_polarities.push_back(reversed_polarities[i]);
        }
      }
      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Responded to request.");
    };

    get_channel_voltages = this->create_service<targeting_interfaces::srv::GetChannelVoltages>(
        "/targeting/get_channel_voltages", service_callback);

    initialize_lookup_table();
    validate_lookup_table();
  }

private:
  bool validate_request(int8_t displacement_x, int8_t displacement_y, uint16_t rotation_angle, uint8_t intensity) {
    if (abs(displacement_x) > MAX_ABSOLUTE_DISPLACEMENT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large absolute displacement in x-direction.");
      return false;
    }
    if (abs(displacement_y) > MAX_ABSOLUTE_DISPLACEMENT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large absolute displacement in y-direction.");
      return false;
    }
    if (rotation_angle > MAX_ROTATION_ANGLE) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large rotation angle.");
      return false;
    }
    if (intensity > MAX_INTENSITY) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large intensity.");
      return false;
    }
    return true;
  }

  void initialize_lookup_table() {
    string line, value_str;

    RCLCPP_INFO(rclcpp::get_logger("targeting"), "Initializing lookup table...");

    ifstream file("targeting.csv", ios::in);

    if (!file.is_open()) {
      RCLCPP_ERROR(rclcpp::get_logger("targeting"), "Could not open file.");
      assert(false);
    }

    while (getline(file, line)) {
      stringstream str(line);

      getline(str, value_str, ',');
      int8_t displacement_x = stoi(value_str);

      getline(str, value_str, ',');
      int8_t displacement_y = stoi(value_str);

      getline(str, value_str, ',');
      uint16_t rotation_angle = stoi(value_str);

      double voltages[N_CHANNELS];
      for (uint8_t i = 0; i < N_CHANNELS; i++) {
        getline(str, value_str, ',');
        voltages[i] = stof(value_str);
      }

      targets[displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle].initialize(voltages);
    }

    RCLCPP_INFO(rclcpp::get_logger("targeting"), "Done.");
  }

  void validate_lookup_table() {
    for (uint8_t x = 0; x < N_DISPLACEMENTS; x++) {
      for (uint8_t y = 0; y < N_DISPLACEMENTS; y++) {
        for (uint16_t angle = 0; angle < N_ROTATION_ANGLES; angle++) {
          if (!targets[x][y][angle].is_initialized()) {
            RCLCPP_ERROR(rclcpp::get_logger("targeting"), "Lookup table does not contain value for (x, y, angle) = (%d, %d, %d).",
              x - MAX_ABSOLUTE_DISPLACEMENT, y - MAX_ABSOLUTE_DISPLACEMENT, angle);
            assert(false);
          }
        }
      }
    }
  }

  Target targets[N_DISPLACEMENTS][N_DISPLACEMENTS][N_ROTATION_ANGLES];
  rclcpp::Service<targeting_interfaces::srv::GetChannelVoltages>::SharedPtr get_channel_voltages;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("targeting"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<Targeting>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("targeting"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
