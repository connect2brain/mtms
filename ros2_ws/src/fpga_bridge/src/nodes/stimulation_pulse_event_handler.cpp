#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

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
      uint8_t execution_condition = event_info.execution_condition;
      uint64_t time_us = event_info.time_us;

      serialized_message.init(channel);
      serialized_message.add_uint16(event_id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_us);

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

      RCLCPP_INFO(rclcpp::get_logger("stimulation_pulse_event_handler"), "Sent stimulation request for channel %d",
                  channel);

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

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
