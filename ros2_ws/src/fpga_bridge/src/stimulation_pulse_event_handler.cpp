#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_board_control.h"
#include "fpga.h"
#include "serdes.h"

const uint8_t channel_pulse_fifos[3] = {
  NiFpga_board_control_HostToTargetFifoU8_Channel1PulseFIFO,
  0,
  0
//  NiFpga_board_control_HostToTargetFifoU8_Channel2PulseFIFO,
//  NiFpga_board_control_HostToTargetFifoU8_Channel3PulseFIFO
};

void send_stimulation_pulse_event(const std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Response> response)
{
  init_serialized_message();

  fpga_interfaces::msg::StimulationPulseEvent stimulation_pulse_event = request->stimulation_pulse_event;

  uint8_t channel = stimulation_pulse_event.channel;

  /* Serialize event info. */

  fpga_interfaces::msg::EventInfo event_info = stimulation_pulse_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t wait_for_trigger = event_info.wait_for_trigger;
  uint64_t time_us = event_info.time_us;

  add_uint16_to_serialized_message(event_id);
  add_byte_to_serialized_message(wait_for_trigger);
  add_uint64_to_serialized_message(time_us);

  /* Serialize stimulation pulse. */

  uint8_t n_pieces = (uint8_t) stimulation_pulse_event.pieces.size();
  add_byte_to_serialized_message(n_pieces);

  for (uint8_t i = 0; i < n_pieces; i++) {
    fpga_interfaces::msg::StimulationPulsePiece piece = stimulation_pulse_event.pieces[i];

    add_byte_to_serialized_message(piece.mode);
    add_uint16_to_serialized_message(piece.duration_in_ticks);
  }

  finalize_serialized_message();

  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     channel_pulse_fifos[channel - 1]));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       channel_pulse_fifos[channel - 1],
                       serialized_message,
                       length,
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < length; i++) {
    RCLCPP_INFO(rclcpp::get_logger("fpga"), "%d,  %d", i - 1, serialized_message[i]);
  }

  response->success = true;
}

class StimulationPulseEventHandler : public rclcpp::Node
{
  public:
    StimulationPulseEventHandler()
    : Node("stimulation_pulse_event_handler")
    {
      send_stimulation_pulse_event_service_ = this->create_service<fpga_interfaces::srv::SendStimulationPulseEvent>("/fpga/send_stimulation_pulse_event", send_stimulation_pulse_event);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SendStimulationPulseEvent>::SharedPtr send_stimulation_pulse_event_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("stimulation_pulse_event_handler"), "Stimulation pulse event handler ready.");

  rclcpp::spin(std::make_shared<StimulationPulseEventHandler>());
  rclcpp::shutdown();

  close_fpga();
}
