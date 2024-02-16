#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "targeting_interfaces/msg/targeting_algorithm.hpp"
#include "targeting_interfaces/srv/get_target_voltages.hpp"
#include "targeting_interfaces/srv/get_maximum_intensity.hpp"

#include <fstream>
#include <string>

using namespace std;

const uint8_t NUM_OF_CHANNELS = 5;

/* Allow targeting inside the area x \in [-18, ..., 18], y \in [-18, ..., 18] (in millimeters) and
 * rotating the e-field within the angles [0, ..., 359].
 *
 * The upper limit for stimulation intensity is 120 V/m - an arbitrary value which can be changed if desired,
 * but gives some added safety, compared to not having the targeting restrict the stimulation intensity.
 * (The maximum capacitor voltages constrain the stimulation intensity regardless of this additional
 * constraint implemented by the targeting.)
 */

const uint8_t MAX_ABSOLUTE_DISPLACEMENT = 18;
const uint16_t MAX_ROTATION_ANGLE = 359;

const uint8_t INTENSITY_LIMIT = 120;

/* HACK: Instead of hard-coding this, it should probably be received from the mTMS device via a ROS message. */
const uint16_t VOLTAGE_LIMIT = 1490;

const uint8_t NUM_OF_DISPLACEMENTS = 2 * MAX_ABSOLUTE_DISPLACEMENT + 1;
const uint16_t NUM_OF_ROTATION_ANGLES = MAX_ROTATION_ANGLE + 1;

const uint8_t NUM_OF_ALGORITHMS = 2;

std::map<uint8_t, std::string> algorithmMap = {
    {targeting_interfaces::msg::TargetingAlgorithm::LEAST_SQUARES, "least-squares"},
    {targeting_interfaces::msg::TargetingAlgorithm::GENETIC, "genetic"}
};

class Target {

public:
  Target() : initialized(false) {}

  void initialize(double voltages[]) {
    for (int i = 0; i < NUM_OF_CHANNELS; i++) {
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
  double voltages[NUM_OF_CHANNELS];
  bool reversed_polarities[NUM_OF_CHANNELS];
  bool initialized;
};

class Targeting : public rclcpp::Node {

public:
  Targeting() : Node("targeting") {
    /* Read ROS parameters. */
    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};

    descriptor.description = "Coil array";
    descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_STRING;
    this->declare_parameter("coil_array", "", descriptor);

    this->coil_array = this->get_parameter("coil_array").get_parameter_value().get<std::string>();

    /* Define callbacks for service calls. */

    auto get_target_voltages_callback = [this](
        const std::shared_ptr<targeting_interfaces::srv::GetTargetVoltages::Request> request,
        std::shared_ptr<targeting_interfaces::srv::GetTargetVoltages::Response> response) -> void {

      int8_t displacement_x = request->target.displacement_x;
      int8_t displacement_y = request->target.displacement_y;
      uint16_t rotation_angle = request->target.rotation_angle;
      uint8_t algorithm = request->target.algorithm.value;
      uint8_t intensity = request->intensity;

      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Request received for channel voltages: (x, y, angle, intensity) = (%d, %d, %d, %d) with algorithm %s",
        displacement_x,
        displacement_y,
        rotation_angle,
        intensity,
        algorithmMap[algorithm].c_str());

      response->success = validate_channel_voltages_request(displacement_x, displacement_y, rotation_angle, intensity);
      if (!response->success) {
        RCLCPP_WARN(rclcpp::get_logger("targeting"), "Failed to respond to channel voltage request.");
        return;
      }

      Target target = targets[algorithm][displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle];

      double* voltages = target.get_voltages();
      bool* reversed_polarities = target.get_reversed_polarities();

      double scaled_voltages[NUM_OF_CHANNELS];
      for (int i = 0; i < NUM_OF_CHANNELS; i++) {
        double scaled_voltage = voltages[i] * intensity;
        if (scaled_voltage > VOLTAGE_LIMIT) {
          response->success = false;
          RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Voltage limit (%d) exceeded: %.1f on channel %d.",
            VOLTAGE_LIMIT, scaled_voltage, i + 1);
          break;
        }
        scaled_voltages[i] = scaled_voltage;
      }
      if (!response->success) {
        RCLCPP_WARN(rclcpp::get_logger("targeting"), "Failed to respond to channel voltage request.");
        return;
      }

      for (int i = 0; i < NUM_OF_CHANNELS; i++) {
        response->voltages.push_back(scaled_voltages[i]);
        response->reversed_polarities.push_back(reversed_polarities[i]);
      }
      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Successfully responded to channel voltage request.");
    };

    auto get_maximum_intensity_callback = [this](
        const std::shared_ptr<targeting_interfaces::srv::GetMaximumIntensity::Request> request,
        std::shared_ptr<targeting_interfaces::srv::GetMaximumIntensity::Response> response) -> void {
      int8_t displacement_x = request->target.displacement_x;
      int8_t displacement_y = request->target.displacement_y;
      uint16_t rotation_angle = request->target.rotation_angle;
      uint8_t algorithm = request->target.algorithm.value;

      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Request received for maximum intensity: (x, y, angle) = (%d, %d, %d) with algorithm %s",
        displacement_x,
        displacement_y,
        rotation_angle,
        algorithmMap[algorithm].c_str());

      response->success = validate_maximum_intensity_request(displacement_x, displacement_y, rotation_angle);
      if (!response->success) {
        RCLCPP_WARN(rclcpp::get_logger("targeting"), "Failed to respond to maximum intensity request.");
        return;
      }

      Target target = targets[algorithm][displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle];

      double max_intensity = std::numeric_limits<double>::infinity();

      /* Compute the highest intensity that does not exceed the voltage limit for channel capacitors (defined in volts). */

      double* voltages = target.get_voltages();

      for (int i = 0; i < NUM_OF_CHANNELS; i++) {
        double max_intensity_for_channel = VOLTAGE_LIMIT / voltages[i];
        max_intensity = min(max_intensity, max_intensity_for_channel);
      }

      /* Cap the maximum intensity to the intensity limit (defined in V/m). */

      response->maximum_intensity = min(INTENSITY_LIMIT, (uint8_t) max_intensity);

      RCLCPP_INFO(rclcpp::get_logger("targeting"), "Successfully responded to maximum intensity request with %d V/m", response->maximum_intensity);
    };

    get_target_voltages = this->create_service<targeting_interfaces::srv::GetTargetVoltages>(
        "/targeting/get_target_voltages", get_target_voltages_callback);

    get_maximum_intensity = this->create_service<targeting_interfaces::srv::GetMaximumIntensity>(
        "/targeting/get_maximum_intensity", get_maximum_intensity_callback);

    initialize_lookup_table();

    /* TODO: Disable validation for now, as we don't have yet computed both genetic and least-squares lookup tables. */
    //validate_lookup_table();
  }

private:
  bool validate_displacement(int8_t displacement_x, int8_t displacement_y) {
    if (abs(displacement_x) > MAX_ABSOLUTE_DISPLACEMENT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large absolute displacement in x-direction, the limit is: %d.", MAX_ABSOLUTE_DISPLACEMENT);
      return false;
    }
    if (abs(displacement_y) > MAX_ABSOLUTE_DISPLACEMENT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large absolute displacement in y-direction, the limit is: %d.", MAX_ABSOLUTE_DISPLACEMENT);
      return false;
    }
    return true;
  }
  bool validate_rotation_angle(int16_t rotation_angle) {
    if (rotation_angle > MAX_ROTATION_ANGLE) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Too large rotation angle, the limit is: %d.", MAX_ROTATION_ANGLE);
      return false;
    }
    return true;
  }
  bool validate_intensity(int8_t intensity) {
    if (intensity > INTENSITY_LIMIT) {
      RCLCPP_WARN(rclcpp::get_logger("targeting"), "Invalid request: Intensity limit (%d V/m) exceeded.", INTENSITY_LIMIT);
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

    for (uint8_t i = 0; i < NUM_OF_ALGORITHMS; i++) {
      std::string algorithm = algorithmMap[i];

      std::string filePath = "data/" + coil_array + "_" + algorithm + ".csv";
      std::ifstream file(filePath.c_str(), std::ios::in);

      if (!file.is_open()) {
        RCLCPP_ERROR(rclcpp::get_logger("targeting"), "Could not open file: %s", filePath.c_str());
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

        double voltages[NUM_OF_CHANNELS];
        for (uint8_t j = 0; j < NUM_OF_CHANNELS; j++) {
          getline(str, value_str, ',');
          voltages[j] = stof(value_str);
        }

        targets[i][displacement_x + MAX_ABSOLUTE_DISPLACEMENT][displacement_y + MAX_ABSOLUTE_DISPLACEMENT][rotation_angle].initialize(voltages);
      }
    }

    RCLCPP_INFO(rclcpp::get_logger("targeting"), "Done.");
  }

  void validate_lookup_table() {
    for (uint8_t i = 0; i < NUM_OF_ALGORITHMS; i++) {
      for (uint8_t x = 0; x < NUM_OF_DISPLACEMENTS; x++) {
        for (uint8_t y = 0; y < NUM_OF_DISPLACEMENTS; y++) {
          for (uint16_t angle = 0; angle < NUM_OF_ROTATION_ANGLES; angle++) {
            if (!targets[i][x][y][angle].is_initialized()) {
              RCLCPP_ERROR(rclcpp::get_logger("targeting"), "Lookup table for algorithm %s does not contain value for (x, y, angle) = (%d, %d, %d).",
                algorithmMap[i].c_str(),
                x - MAX_ABSOLUTE_DISPLACEMENT,
                y - MAX_ABSOLUTE_DISPLACEMENT,
                angle);
              assert(false);
            }
          }
        }
      }
    }
  }

  Target targets[NUM_OF_ALGORITHMS][NUM_OF_DISPLACEMENTS][NUM_OF_DISPLACEMENTS][NUM_OF_ROTATION_ANGLES];
  rclcpp::Service<targeting_interfaces::srv::GetTargetVoltages>::SharedPtr get_target_voltages;
  rclcpp::Service<targeting_interfaces::srv::GetMaximumIntensity>::SharedPtr get_maximum_intensity;

  std::string coil_array;
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
