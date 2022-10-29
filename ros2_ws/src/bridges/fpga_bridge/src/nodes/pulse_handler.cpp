#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_pulse.hpp"
#include "fpga_interfaces/msg/pulse_piece.hpp"
#include "fpga_interfaces/msg/pulse.hpp"
#include "fpga_interfaces/msg/event.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 channel_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetStimulationpulseFIFO;

class PulseHandler : public rclcpp::Node {
public:
  PulseHandler()
      : Node("pulse_handler") {

    auto service_callback = [this](
        const std::shared_ptr<fpga_interfaces::srv::SendPulse::Request> request,
        std::shared_ptr<fpga_interfaces::srv::SendPulse::Response> response) -> void {
      fpga_interfaces::msg::Pulse pulse = request->pulse;

      uint8_t channel = pulse.channel;

      /* Serialize event info. */

      fpga_interfaces::msg::Event event = pulse.event;

      uint16_t id = event.id;
      uint8_t execution_condition = event.execution_condition.value;
      uint64_t time_us = event.time_us;

      serialized_message.init(channel);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_us);

      /* Serialize stimulation pulse. */

      uint8_t n_pieces = (uint8_t) pulse.pieces.size();
      serialized_message.add_byte(n_pieces);

      for (uint8_t i = 0; i < n_pieces; i++) {
        fpga_interfaces::msg::PulsePiece piece = pulse.pieces[i];

        serialized_message.add_byte(piece.current_mode.value);
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

      RCLCPP_INFO(rclcpp::get_logger("pulse_handler"), "Sent pulse to channel %d", channel);

      response->success = true;
    };

    serialized_message = SerializedMessage();
    send_pulse_service_ = this->create_service<fpga_interfaces::srv::SendPulse>(
        "/fpga/send_pulse", service_callback);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendPulse>::SharedPtr send_pulse_service_;
  SerializedMessage serialized_message;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("pulse_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<PulseHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("pulse_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("pulse_handler"), "Pulse handler ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
