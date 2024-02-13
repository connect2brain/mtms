#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/discharge.hpp"
#include "event_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 discharge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class DischargeHandler : public rclcpp::Node {
public:
  DischargeHandler()
      : Node("discharge_handler") {

    this->declare_parameter<bool>("safe-mode", false);
    this->get_parameter("safe-mode", safe_mode);

    auto callback = [this](const std::shared_ptr<event_interfaces::msg::Discharge> discharge) -> void {
      /* Unpack discharge message. */
      uint8_t channel = discharge->channel;
      event_interfaces::msg::EventInfo event_info = discharge->event_info;

      uint16_t id = event_info.id;
      uint8_t execution_condition = event_info.execution_condition.value;
      double_t execution_time = event_info.execution_time;
      double_t delay = event_info.delay;

      /* Log discharge message. */
      RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Discharging channel %d to %d V (id: %d, execution_condition: %d, execution_time: %.4f s, delay: %.4f s)",
                  discharge->channel,
                  discharge->target_voltage,
                  discharge->event_info.id,
                  discharge->event_info.execution_condition.value,
                  discharge->event_info.execution_time,
                  discharge->event_info.delay);

      /* Check if FPGA is OK. */
      if (!is_fpga_ok()) {
        RCLCPP_WARN(rclcpp::get_logger("discharge_handler"), "Tried to discharge (id: %d) but FPGA is not in OK state", id);
        return;
      }

      /* Check that execution time and delay are non-negative. */

      /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
      if (execution_time < 0.0) {
        RCLCPP_ERROR(rclcpp::get_logger("charge_handler"), "Execution time cannot be negative, aborting discharge (id: %d)", id);
        return;
      }
      if (delay < 0.0) {
        RCLCPP_ERROR(rclcpp::get_logger("charge_handler"), "Delay cannot be negative, aborting discharge (id: %d)", id);
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

      /* Serialize discharge parameter. */

      uint16_t target_voltage = discharge->target_voltage;
      serialized_message.add_uint16(target_voltage);

      serialized_message.finalize();

      if (this->safe_mode) {
        RCLCPP_WARN(rclcpp::get_logger("discharge_handler"), "Safe mode is enabled, aborting discharge (id: %d)", id);
        return;
      }
      NiFpga_MergeStatus(&status,
                         NiFpga_StartFifo(session,
                                          discharge_fifo));

      NiFpga_MergeStatus(&status,
                          NiFpga_WriteFifoU8(session,
                                            discharge_fifo,
                                            serialized_message.serialized_message.data(),
                                            serialized_message.get_length(),
                                            NiFpga_InfiniteTimeout,
                                            NULL));
    };
    serialized_message = SerializedMessage();
    send_discharge_subscriber_ = this->create_subscription<event_interfaces::msg::Discharge>(
        "/event/send/discharge", 10, callback);
  }

private:
  rclcpp::Subscription<event_interfaces::msg::Discharge>::SharedPtr send_discharge_subscriber_;
  SerializedMessage serialized_message;
  bool safe_mode;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<DischargeHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Discharge handler ready.");

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
