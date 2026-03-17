#include "rclcpp/rclcpp.hpp"
#include "targeting_services/srv/reverse_polarity.hpp"
#include "waveform_msgs/msg/waveform_phase.hpp"
#include "waveform_msgs/msg/waveform_piece.hpp"
#include "waveform_msgs/msg/waveform.hpp"

using namespace std;

const uint16_t WAVEFORM_PHASE_MAPPING[][2] = {
  {waveform_msgs::msg::WaveformPhase::NON_CONDUCTIVE, waveform_msgs::msg::WaveformPhase::NON_CONDUCTIVE},
  {waveform_msgs::msg::WaveformPhase::RISING, waveform_msgs::msg::WaveformPhase::FALLING},
  {waveform_msgs::msg::WaveformPhase::FALLING, waveform_msgs::msg::WaveformPhase::RISING},

  /* HOLD is mapped to ALTERNATIVE_HOLD and vice versa so that the IGBT bridges for both are used roughly
     equally, making them wear out at the same rate. */
  {waveform_msgs::msg::WaveformPhase::HOLD, waveform_msgs::msg::WaveformPhase::ALTERNATIVE_HOLD},
  {waveform_msgs::msg::WaveformPhase::ALTERNATIVE_HOLD, waveform_msgs::msg::WaveformPhase::HOLD}
};

class ReversePolarity : public rclcpp::Node {

public:
  ReversePolarity() : Node("reverse_polarity") {

    auto service_callback = [this](
        const std::shared_ptr<targeting_services::srv::ReversePolarity::Request> request,
        std::shared_ptr<targeting_services::srv::ReversePolarity::Response> response) -> void {

      RCLCPP_INFO(rclcpp::get_logger("reverse_polarity"), "Request received: Reverse polarity.");

      waveform_msgs::msg::Waveform waveform = request->waveform;
      waveform_msgs::msg::WaveformPiece piece;

      uint8_t num_of_pieces = std::size(waveform.pieces);
      for (uint8_t i = 0; i < num_of_pieces; i++) {
        piece.duration_in_ticks = waveform.pieces[i].duration_in_ticks;

        /* TODO: Does not check that the current mode is valid, i.e., that the value is in the correct range.
         *   This will be checked by ROS2 once WaveformPhase ROS message type is a proper enum.
         */
        uint8_t waveform_phase = waveform.pieces[i].waveform_phase.value;
        uint8_t new_waveform_phase;

        for (uint8_t j = 0; j < std::size(WAVEFORM_PHASE_MAPPING); j++) {
          if (waveform_phase == WAVEFORM_PHASE_MAPPING[j][0]) {
            new_waveform_phase = WAVEFORM_PHASE_MAPPING[j][1];
          }
        }
        piece.waveform_phase.value = new_waveform_phase;

        response->waveform.pieces.push_back(piece);
      }

      response->success = true;

      RCLCPP_INFO(rclcpp::get_logger("reverse_polarity"), "Responded to request.");
    };

    reverse_polarity_service = this->create_service<targeting_services::srv::ReversePolarity>(
        "/waveforms/reverse_polarity", service_callback);
  }

private:
  rclcpp::Service<targeting_services::srv::ReversePolarity>::SharedPtr reverse_polarity_service;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  auto node = std::make_shared<ReversePolarity>();

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
