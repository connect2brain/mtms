#include <chrono>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/empty.hpp"
#include "mtms_targeting_interfaces/srv/get_default_waveform.hpp"
#include "mtms_waveform_interfaces/msg/waveform_phase.hpp"
#include "mtms_waveform_interfaces/msg/waveform_piece.hpp"

using namespace std;

const std::string HEARTBEAT_TOPIC = "/mtms/waveform_utils/get_default_waveform/heartbeat";
constexpr std::chrono::milliseconds HEARTBEAT_PUBLISH_PERIOD{500};

const uint16_t DEFAULT_WAVEFORM[][2] = {
  {mtms_waveform_interfaces::msg::WaveformPhase::RISING, 2400},
  {mtms_waveform_interfaces::msg::WaveformPhase::HOLD, 1200},
  {mtms_waveform_interfaces::msg::WaveformPhase::FALLING, 0}
};

class GetDefaultWaveform : public rclcpp::Node {

public:
  GetDefaultWaveform() : Node("get_default_waveform") {
    /* Read ramp-down timings from parameters. */
    this->declare_parameter<std::string>("ramp_down_timings", "");
    ramp_down_timings = parse_ramp_down_timings(
        this->get_parameter("ramp_down_timings").as_string());

    /* Log ramp-down timings. */
    std::string timings_log;
    for (size_t i = 0; i < ramp_down_timings.size(); i++) {
      if (i > 0) timings_log += ", ";
      timings_log += std::to_string(ramp_down_timings[i]);
    }
    RCLCPP_INFO(this->get_logger(), "Ramp-down timings (ticks): [%s]", timings_log.c_str());

    /* Define service callback. */
    auto service_callback = [this](
        const std::shared_ptr<mtms_targeting_interfaces::srv::GetDefaultWaveform::Request> request,
        std::shared_ptr<mtms_targeting_interfaces::srv::GetDefaultWaveform::Response> response) -> void {

      int8_t channel = request->channel;

      RCLCPP_INFO(rclcpp::get_logger("get_default_waveform"), "Request received: Default waveform for channel %d", channel);

      /* Validate channel.*/
      if (channel < 0 || static_cast<size_t>(channel) >= ramp_down_timings.size()) {
        RCLCPP_WARN(rclcpp::get_logger("get_default_waveform"), "Invalid channel: %d.", channel);

        response->success = false;
        return;
      }

      /* Construct waveform. */
      mtms_waveform_interfaces::msg::WaveformPiece piece;
      for (uint8_t i = 0; i < std::size(DEFAULT_WAVEFORM); i++) {
        piece.waveform_phase.value = DEFAULT_WAVEFORM[i][0];

        if (i == std::size(DEFAULT_WAVEFORM) - 1) {
          piece.duration_in_ticks = ramp_down_timings[channel];
        } else {
          piece.duration_in_ticks = DEFAULT_WAVEFORM[i][1];
        }

        response->waveform.pieces.push_back(piece);
      }

      response->success = true;

      RCLCPP_INFO(rclcpp::get_logger("get_default_waveform"), "Responded to request.");
    };

    get_default_waveform_service = this->create_service<mtms_targeting_interfaces::srv::GetDefaultWaveform>(
        "/mtms/waveforms/get_default", service_callback);

    auto heartbeat_publisher = this->create_publisher<std_msgs::msg::Empty>(HEARTBEAT_TOPIC, 10);
    this->create_wall_timer(HEARTBEAT_PUBLISH_PERIOD, [heartbeat_publisher]() {
      heartbeat_publisher->publish(std_msgs::msg::Empty());
    });
  }

private:
  static std::vector<uint16_t> parse_ramp_down_timings(const std::string& timings_str) {
    if (timings_str.empty()) {
      throw std::runtime_error("ramp_down_timings parameter is empty");
    }
    std::vector<uint16_t> timings;
    std::stringstream ss(timings_str);
    std::string token;
    while (std::getline(ss, token, ',')) {
      unsigned long value = std::stoul(token);  // throws on invalid input
      if (value == 0 || value > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error("ramp_down_timings value out of range (1–65535): " + token);
      }
      timings.push_back(static_cast<uint16_t>(value));
    }
    return timings;
  }

  std::vector<uint16_t> ramp_down_timings;
  rclcpp::Service<mtms_targeting_interfaces::srv::GetDefaultWaveform>::SharedPtr get_default_waveform_service;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<GetDefaultWaveform>();

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
