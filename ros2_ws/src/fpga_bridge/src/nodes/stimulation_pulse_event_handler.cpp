#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 channel_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetStimulationpulseFIFO;

class StimulationPulseEventHandler : public rclcpp::Node {
public:
  StimulationPulseEventHandler()
      : Node("stimulation_pulse_event_handler") {

    auto service_callback = [this](
        const std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Request> request,
        std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Response> response) -> void {
      fpga_interfaces::msg::StimulationPulseEvent stimulation_pulse_event = request->stimulation_pulse_event;

      uint8_t channel = stimulation_pulse_event.channel;

      /* Serialize event info. */

      fpga_interfaces::msg::EventInfo event_info = stimulation_pulse_event.event_info;

      uint16_t event_id = event_info.event_id;
      uint8_t wait_for_trigger = event_info.wait_for_trigger;
      uint64_t time_us = event_info.time_us;
      uint32_t delay_us = event_info.delay_us;

      serialized_message.init(channel);
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
                                            serialized_message.serialized_message.data(),
                                            serialized_message.get_length(),
                                            NiFpga_InfiniteTimeout,
                                            NULL));

      for (uint8_t i = 0; i < serialized_message.get_length(); i++) {
        RCLCPP_INFO(rclcpp::get_logger("fpga"), "%d,  %d", i - 1, serialized_message.serialized_message[i]);
      }

      response->success = true;
    };

    serialized_message = SerializedMessage();
    send_stimulation_pulse_event_service_ = this->create_service<fpga_interfaces::srv::SendStimulationPulseEvent>(
        "/fpga/send_stimulation_pulse_event", service_callback);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendStimulationPulseEvent>::SharedPtr send_stimulation_pulse_event_service_;
  SerializedMessage serialized_message;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

  auto node = std::make_shared<StimulationPulseEventHandler>();
  RCLCPP_INFO(rclcpp::get_logger("stimulation_pulse_event_handler"), "Stimulation pulse event handler ready.");

#ifdef ON_UNIX
  //set_default_thread_stacksize(1024 * 50); //50 MB, default in unix is 8 MB
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
