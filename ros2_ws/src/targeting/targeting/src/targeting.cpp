#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "targeting_interfaces/srv/get_channel_voltages.hpp"
#include "targeting_interfaces/srv/get_maximum_intensity.hpp"

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

/* HACK: Instead of hard-coding this, it should probably be received from the mTMS device via a ROS message. */
const uint16_t MAX_VOLTAGE = 1490;

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

    auto get_channel_voltages_callback = [this](
        const std::shared_ptr<targeting_interfaces::srv::GetChannelVoltages::Request> request,
        std::shared_ptr<targeting_interfaces::srv::GetChannelVoltages::Response> response) -> void {
      int8_t displacement_x = request->displacement_x;
      int8_t displacement_y = request->displacement_y;
      uint16_t rotation_angle = request->rotation_angle;
      uint8_t intensity = request->intensity;

      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Request received for channel voltages: (x, y, angle, intensity) = (%d, %d, %d, %d)",
        displacement_x, displacement_y, rotation_angle, intensity);

      response->success = validate_channel_voltages_request(displacement_x, displacement_y, rotation_angle, intensity);

      if (response->success) {
        Target target = targets[displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle];

        double* voltages = target.get_voltages();
        bool* reversed_polarities = target.get_reversed_polarities();

        double scaled_voltages[N_CHANNELS];
        for (int i = 0; i < N_CHANNELS; i++) {
          double scaled_voltage = voltages[i] * intensity;
          if (scaled_voltage > MAX_VOLTAGE) {
            response->success = false;
            RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Maximum voltage (%d) exceeded: %.1f on channel %d.",
              MAX_VOLTAGE, scaled_voltage, i + 1);
            break;
          }
          scaled_voltages[i] = scaled_voltage;
        }

        if (response->success) {
          for (int i = 0; i < N_CHANNELS; i++) {
            response->voltages.push_back(scaled_voltages[i]);
            response->reversed_polarities.push_back(reversed_polarities[i]);
          }
        }
      }
      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Responded to channel voltage request.");
    };

    auto get_maximum_intensity_callback = [this](
        const std::shared_ptr<targeting_interfaces::srv::GetMaximumIntensity::Request> request,
        std::shared_ptr<targeting_interfaces::srv::GetMaximumIntensity::Response> response) -> void {
      int8_t displacement_x = request->displacement_x;
      int8_t displacement_y = request->displacement_y;
      uint16_t rotation_angle = request->rotation_angle;

      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Request received for maximum intensity: (x, y, angle) = (%d, %d, %d)",
        displacement_x, displacement_y, rotation_angle);

      response->success = validate_maximum_intensity_request(displacement_x, displacement_y, rotation_angle);

      if (response->success) {
        Target target = targets[displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle];

        double max_intensity = std::numeric_limits<double>::infinity();
        double* voltages = target.get_voltages();

        for (int i = 0; i < N_CHANNELS; i++) {
          double max_intensity_for_channel = MAX_VOLTAGE / voltages[i];
          max_intensity = min(max_intensity, max_intensity_for_channel);
        }
        response->maximum_intensity = max_intensity;
      }
      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Responded to maximum intensity request.");
    };

    get_channel_voltages = this->create_service<targeting_interfaces::srv::GetChannelVoltages>(
        "/targeting/get_channel_voltages", get_channel_voltages_callback);

    get_maximum_intensity = this->create_service<targeting_interfaces::srv::GetMaximumIntensity>(
        "/targeting/get_maximum_intensity", get_maximum_intensity_callback);

    initialize_lookup_table();
    validate_lookup_table();
  }

private:
  bool validate_displacement(int8_t displacement_x, int8_t displacement_y) {
    if (abs(displacement_x) > MAX_ABSOLUTE_DISPLACEMENT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large absolute displacement in x-direction.");
      return false;
    }
    if (abs(displacement_y) > MAX_ABSOLUTE_DISPLACEMENT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large absolute displacement in y-direction.");
      return false;
    }
    return true;
  }
  bool validate_rotation_angle(int16_t rotation_angle) {
    if (rotation_angle > MAX_ROTATION_ANGLE) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large rotation angle.");
      return false;
    }
    return true;
  }
  bool validate_intensity(int8_t intensity) {
    if (intensity > MAX_INTENSITY) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large intensity.");
      return false;
    }
    return true;
  }

  bool validate_channel_voltages_request(int8_t displacement_x, int8_t displacement_y, uint16_t rotation_angle, uint8_t intensity) {
    return validate_displacement(displacement_x, displacement_y) &&
           validate_rotation_angle(rotation_angle) &&
           validate_intensity(intensity);
  }

  bool validate_maximum_intensity_request(int8_t displacement_x, int8_t displacement_y, uint16_t rotation_angle) {
    return validate_displacement(displacement_x, displacement_y) &&
           validate_rotation_angle(rotation_angle);
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
  rclcpp::Service<targeting_interfaces::srv::GetMaximumIntensity>::SharedPtr get_maximum_intensity;
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
