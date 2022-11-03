#include "scheduling_utils.h"
#include "memory_utils.h"

#include "rclcpp/rclcpp.hpp"
#include "targeting_interfaces/srv/reverse_polarity.hpp"
#include "fpga_interfaces/msg/waveform_phase.hpp"
#include "fpga_interfaces/msg/waveform_piece.hpp"

using namespace std;

const uint16_t WAVEFORM_PHASE_MAPPING[][2] = {
  {fpga_interfaces::msg::WaveformPhase::NON_CONDUCTIVE, fpga_interfaces::msg::WaveformPhase::NON_CONDUCTIVE},
  {fpga_interfaces::msg::WaveformPhase::RISING, fpga_interfaces::msg::WaveformPhase::FALLING},
  {fpga_interfaces::msg::WaveformPhase::FALLING, fpga_interfaces::msg::WaveformPhase::RISING},
  {fpga_interfaces::msg::WaveformPhase::HOLD, fpga_interfaces::msg::WaveformPhase::ALTERNATIVE_HOLD},
  {fpga_interfaces::msg::WaveformPhase::ALTERNATIVE_HOLD, fpga_interfaces::msg::WaveformPhase::HOLD}
};

class ReversePolarity : public rclcpp::Node {

public:
  ReversePolarity() : Node("reverse_polarity") {

    auto service_callback = [this](
        const std::shared_ptr<targeting_interfaces::srv::ReversePolarity::Request> request,
        std::shared_ptr<targeting_interfaces::srv::ReversePolarity::Response> response) -> void {

      RCLCPP_INFO(rclcpp::get_logger("reverse_polarity"), "Request received: Reverse polarity.");

      fpga_interfaces::msg::WaveformPiece piece;

      uint8_t n_pieces = std::size(request->waveform);
      for (uint8_t i = 0; i < n_pieces; i++) {
        piece.duration_in_ticks = request->waveform[i].duration_in_ticks;

        /* TODO: Does not check that the current mode is valid, i.e., that the value is in the correct range.
         *   This will be checked by ROS2 once WaveformPhase ROS message type is a proper enum.
         */
        uint8_t waveform_phase = request->waveform[i].waveform_phase.value;
        uint8_t new_waveform_phase;

        for (uint8_t j = 0; j < std::size(WAVEFORM_PHASE_MAPPING); j++) {
          if (waveform_phase == WAVEFORM_PHASE_MAPPING[j][0]) {
            new_waveform_phase = WAVEFORM_PHASE_MAPPING[j][1];
          }
        }
        piece.waveform_phase.value = new_waveform_phase;

        response->waveform.push_back(piece);
      }

      response->success = true;

      RCLCPP_INFO(rclcpp::get_logger("reverse_polarity"), "Responded to request.");
    };

    reverse_polarity_service = this->create_service<targeting_interfaces::srv::ReversePolarity>(
        "/waveforms/reverse_polarity", service_callback);
  }

private:
  rclcpp::Service<targeting_interfaces::srv::ReversePolarity>::SharedPtr reverse_polarity_service;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("reverse_polarity"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<ReversePolarity>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("reverse_polarity"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
