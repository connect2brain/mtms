#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/trigger_out.hpp"
#include "event_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 trigger_out_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetTriggerOutFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class TriggerOutHandler : public rclcpp::Node {
public:
  TriggerOutHandler()
      : Node("trigger_out_handler") {

    auto callback = [this](const std::shared_ptr<event_interfaces::msg::TriggerOut> trigger_out) -> void {
      /* Unpack trigger out message. */
      uint8_t port = trigger_out->port;

      event_interfaces::msg::EventInfo event_info = trigger_out->event_info;
      uint16_t id = event_info.id;
      uint8_t execution_condition = event_info.execution_condition.value;
      double_t execution_time = event_info.execution_time;
      double_t delay = event_info.delay;

      /* Log trigger out message. */
      RCLCPP_INFO(rclcpp::get_logger("trigger_out_handler"), "Sending trigger out to port %d (id: %d, execution_condition: %d, execution_time: %.4f s, delay: %.4f s)",
                  port,
                  trigger_out->event_info.id,
                  trigger_out->event_info.execution_condition.value,
                  trigger_out->event_info.execution_time,
                  trigger_out->event_info.delay);

      /* Check if FPGA is OK. */
      if (!is_fpga_ok()) {
        RCLCPP_WARN(rclcpp::get_logger("trigger_out_handler"), "FPGA is not in OK state, aborting sending trigger out event (id: %d)", id);
        return;
      }

      /* Check that execution time and delay are non-negative. */

      /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
      if (execution_time < 0.0) {
        RCLCPP_ERROR(rclcpp::get_logger("trigger_out_handler"), "Execution time cannot be negative, aborting trigger out (id: %d)", id);
        return;
      }
      if (delay < 0.0) {
        RCLCPP_ERROR(rclcpp::get_logger("trigger_out_handler"), "Delay cannot be negative, aborting trigger out (id: %d)", id);
        return;
      }

      /* Serialize event info. */

      uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);
      uint32_t delay_ticks = (uint32_t)(delay * CLOCK_FREQUENCY_HZ);

      serialized_message.init(port);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(execution_time_ticks);
      serialized_message.add_uint32(delay_ticks);

      /* Serialize trigger out parameters. */

      uint32_t duration_us = trigger_out->duration_us;
      uint32_t duration_ticks = duration_us * (CLOCK_FREQUENCY_HZ / 1e6);

      serialized_message.add_uint32(duration_ticks);

      serialized_message.finalize();

      /* For consistency with channel indexing, start trigger out indexing from 1. */
      NiFpga_MergeStatus(&status,
                         NiFpga_StartFifo(session,
                                          trigger_out_fifo));

      NiFpga_MergeStatus(&status,
                         NiFpga_WriteFifoU8(session,
                                            trigger_out_fifo,
                                            serialized_message.serialized_message.data(),
                                            serialized_message.get_length(),
                                            NiFpga_InfiniteTimeout,
                                            NULL));
    };

    serialized_message = SerializedMessage();
    send_trigger_out_subscriber_ = this->create_subscription<event_interfaces::msg::TriggerOut>(
        "/event/send/trigger_out", 10, callback);
  }

private:
  rclcpp::Subscription<event_interfaces::msg::TriggerOut>::SharedPtr send_trigger_out_subscriber_;
  SerializedMessage serialized_message;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("trigger_out_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<TriggerOutHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("trigger_out_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif
  RCLCPP_INFO(rclcpp::get_logger("trigger_out_handler"), "Trigger out handler ready.");

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
