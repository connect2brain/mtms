#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/charge.hpp"
#include "event_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 charge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class ChargeHandler : public rclcpp::Node {
public:
  ChargeHandler()
      : Node("charge_handler") {

    auto callback = [this](const std::shared_ptr<event_interfaces::msg::Charge> charge) -> void {
      uint8_t channel = charge->channel;

      /* Serialize event. */
      event_interfaces::msg::EventInfo event_info = charge->event_info;

      uint16_t id = event_info.id;
      uint8_t execution_condition = event_info.execution_condition.value;
      double_t execution_time = event_info.execution_time;
      uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);

      serialized_message.init();
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(execution_time_ticks);

      /* Serialize charge. */

      // Charge requires the channel here instead of the beginning of the message
      serialized_message.add_byte(channel);

      uint16_t target_voltage = charge->target_voltage;
      serialized_message.add_uint16(target_voltage);

      serialized_message.finalize();

      NiFpga_MergeStatus(&status,
                         NiFpga_StartFifo(session,
                                          charge_fifo));

      NiFpga_MergeStatus(&status,
                         NiFpga_WriteFifoU8(session,
                                            charge_fifo,
                                            serialized_message.serialized_message.data(),
                                            serialized_message.get_length(),
                                            NiFpga_InfiniteTimeout,
                                            NULL));


      RCLCPP_INFO(rclcpp::get_logger("charge_handler"), "Sent charge to channel %d", channel);
    };

    serialized_message = SerializedMessage();

    send_charge_subscriber_ = this->create_subscription<event_interfaces::msg::Charge>("/event/send/charge", 10, callback);
  }

private:
  rclcpp::Subscription<event_interfaces::msg::Charge>::SharedPtr send_charge_subscriber_;
  SerializedMessage serialized_message;

};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("charge_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<ChargeHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("charge_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("charge_handler"), "Charge handler ready.");

  rclcpp::spin(node);

  rclcpp::shutdown();

  close_fpga();
}
