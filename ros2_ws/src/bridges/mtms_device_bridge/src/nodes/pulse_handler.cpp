#include "rclcpp/rclcpp.hpp"

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
  PulseHandler() : Node("pulse_handler") {

    this->declare_parameter<bool>("safe-mode", false);
    this->get_parameter("safe-mode", safe_mode);

    auto callback = [this](const std::shared_ptr<event_interfaces::msg::Pulse> pulse) -> void {
      /* Unpack pulse message. */
      uint8_t channel = pulse->channel;

      event_interfaces::msg::EventInfo event_info = pulse->event_info;
      uint16_t id = event_info.id;
      uint8_t execution_condition = event_info.execution_condition.value;
      double_t execution_time = event_info.execution_time;
      double_t delay = event_info.delay;

      /* Log pulse message. */
      RCLCPP_INFO(rclcpp::get_logger("pulse_handler"), "Executing pulse on channel %d (id: %d, execution_condition: %d, execution_time: %.4f s, delay: %.4f s)",
                  pulse->channel,
                  pulse->event_info.id,
                  pulse->event_info.execution_condition.value,
                  pulse->event_info.execution_time,
                  pulse->event_info.delay);

      /* Check if FPGA is OK. */
      if (!is_fpga_ok()) {
        RCLCPP_WARN(rclcpp::get_logger("pulse_handler"), "Tried to execute pulse (id: %d) but FPGA is not in OK state", id);
        return;
      }

      /* Check that execution time and delay are non-negative. */

      /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
      if (execution_time < 0.0) {
        RCLCPP_ERROR(rclcpp::get_logger("pulse_handler"), "Execution time cannot be negative, aborting pulse (id: %d)", id);
        return;
      }
      if (delay < 0.0) {
        RCLCPP_ERROR(rclcpp::get_logger("pulse_handler"), "Delay cannot be negative, aborting pulse (id: %d)", id);
        return;
      }

      /* Serialize event info. */

      uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);
      uint32_t delay_ticks = (uint32_t)(delay * CLOCK_FREQUENCY_HZ);

      /* XXX: Note that LabVIEW starts indexing from 1. Hence, do the conversion from 0-based
           indexing here. It would rather be the responsibility of FPGA to do the conversion;
           move the logic there eventually. */
      serialized_message.init(channel + 1);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(execution_time_ticks);
      serialized_message.add_uint32(delay_ticks);

      /* Serialize pulse parameters. */

      uint8_t n_waveform = (uint8_t) pulse->waveform.size();
      serialized_message.add_byte(n_waveform);

      for (uint8_t i = 0; i < n_waveform; i++) {
        event_interfaces::msg::WaveformPiece piece = pulse->waveform[i];

        serialized_message.add_byte(piece.waveform_phase.value);
        serialized_message.add_uint16(piece.duration_in_ticks);
      }

      serialized_message.finalize();

      if (this->safe_mode) {
        RCLCPP_WARN(rclcpp::get_logger("pulse_handler"), "Safe mode is enabled, aborting pulse (id: %d)", id);
        return;
      }

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
    };

    serialized_message = SerializedMessage();
    send_pulse_subscriber_ = this->create_subscription<event_interfaces::msg::Pulse>(
        "/event/send/pulse", 10, callback);
  }

private:
  rclcpp::Subscription<event_interfaces::msg::Pulse>::SharedPtr send_pulse_subscriber_;
  SerializedMessage serialized_message;
  bool safe_mode;
};

int main(int argc, char **argv) {
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

  init_fpga();

  auto timer = node->create_wall_timer(
      std::chrono::milliseconds(FPGA_OK_CHECK_INTERVAL_MS),
      [&]() {
          if (!is_fpga_ok()) {
              close_fpga();
              init_fpga();
          }
      }
  );
  rclcpp::spin(node);

  close_fpga();
  rclcpp::shutdown();
}
