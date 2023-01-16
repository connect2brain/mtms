#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/srv/send_pulse.hpp"
#include "event_interfaces/msg/waveform_piece.hpp"
#include "event_interfaces/msg/pulse.hpp"
#include "event_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 channel_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetPulseFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class PulseHandler : public rclcpp::Node {
public:
  PulseHandler()
      : Node("pulse_handler") {

    auto service_callback = [this](
        const std::shared_ptr<event_interfaces::srv::SendPulse::Request> request,
        std::shared_ptr<event_interfaces::srv::SendPulse::Response> response) -> void {
      event_interfaces::msg::Pulse pulse = request->pulse;

      uint8_t channel = pulse.channel;

      /* Serialize event info. */

      event_interfaces::msg::EventInfo event_info = pulse.event_info;

      uint16_t id = event_info.id;
      uint8_t execution_condition = event_info.execution_condition.value;
      double_t time = event_info.time;
      uint64_t time_ticks = (uint64_t)(time * CLOCK_FREQUENCY_HZ);

      serialized_message.init(channel);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_ticks);

      /* Serialize stimulation pulse. */

      uint8_t n_waveform = (uint8_t) pulse.waveform.size();
      serialized_message.add_byte(n_waveform);

      for (uint8_t i = 0; i < n_waveform; i++) {
        event_interfaces::msg::WaveformPiece piece = pulse.waveform[i];

        serialized_message.add_byte(piece.waveform_phase.value);
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
    send_pulse_service_ = this->create_service<event_interfaces::srv::SendPulse>(
        "/event/send_pulse", service_callback);
  }

private:
  rclcpp::Service<event_interfaces::srv::SendPulse>::SharedPtr send_pulse_service_;
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
