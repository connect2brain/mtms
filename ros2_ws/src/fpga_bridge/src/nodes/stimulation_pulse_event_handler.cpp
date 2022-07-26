#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"

const NiFpga_mTMS_HostToTargetFifoU8 channel_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetStimulationpulseFIFO;

void send_stimulation_pulse_event(const std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Response> response)
{
  fpga_interfaces::msg::StimulationPulseEvent stimulation_pulse_event = request->stimulation_pulse_event;

  uint8_t channel = stimulation_pulse_event.channel;

  /* Serialize event info. */
  auto serialized_message = SerializedMessage(channel);

  fpga_interfaces::msg::EventInfo event_info = stimulation_pulse_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t wait_for_trigger = event_info.wait_for_trigger;
  uint64_t time_us = event_info.time_us;
  uint32_t delay_us = event_info.delay_us;

  serialized_message.add_uint16(event_id);
  serialized_message.add_byte(wait_for_trigger);
  serialized_message.add_uint64(time_us);
  serialized_message.add_uint32(delay_us);

  /* Serialize stimulation pulse. */

  uint8_t n_pieces = (uint8_t) stimulation_pulse_event.pieces.size();
  serialized_message.add_byte(n_pieces);

  for (uint8_t i = 0; i < n_pieces; i++) {
    fpga_interfaces::msg::StimulationPulsePiece piece = stimulation_pulse_event.pieces[i];

    serialized_message.add_byte(piece.mode);
    serialized_message.add_uint16(piece.duration_in_ticks);
  }

  serialized_message.finalize();

  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     channel_pulse_fifo));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       channel_pulse_fifo,
                       serialized_message.serialized_message,
                       serialized_message.get_length(),
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < serialized_message.get_length(); i++) {
    RCLCPP_INFO(rclcpp::get_logger("fpga"), "%d,  %d", i - 1, serialized_message.serialized_message[i]);
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
