#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_charge.hpp"
#include "fpga_interfaces/msg/charge.hpp"
#include "fpga_interfaces/msg/event.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 charge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO;

class ChargeHandler : public rclcpp::Node {
public:
  ChargeHandler()
      : Node("charge_handler") {

    auto service_callback = [this](const std::shared_ptr<fpga_interfaces::srv::SendCharge::Request> request,
                                   std::shared_ptr<fpga_interfaces::srv::SendCharge::Response> response) -> void {
      fpga_interfaces::msg::Charge charge = request->charge;

      uint8_t channel = charge.channel;

      /* Serialize event. */
      fpga_interfaces::msg::Event event = charge.event;

      uint16_t id = event.id;
      uint8_t execution_condition = event.execution_condition.value;
      uint64_t time_us = event.time_us;

      serialized_message.init();
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_us);

      /* Serialize charge. */

      // Charge requires the channel here instead of the beginning of the message
      serialized_message.add_byte(channel);

      uint16_t target_voltage = charge.target_voltage;
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


      RCLCPP_INFO(rclcpp::get_logger("charge"), "Sent charge to channel %d", channel);

      response->success = true;
    };

    serialized_message = SerializedMessage();
    send_charge_service_ = this->create_service<fpga_interfaces::srv::SendCharge>("/fpga/send_charge", service_callback);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendCharge>::SharedPtr send_charge_service_;
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
