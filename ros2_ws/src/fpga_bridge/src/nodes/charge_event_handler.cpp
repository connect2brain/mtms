#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_charge_event.hpp"
#include "fpga_interfaces/msg/charge_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 charge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO;

class ChargeEventHandler : public rclcpp::Node {
public:
  ChargeEventHandler()
      : Node("charge_event_handler") {

    auto service_callback = [this](const std::shared_ptr<fpga_interfaces::srv::SendChargeEvent::Request> request,
                                   std::shared_ptr<fpga_interfaces::srv::SendChargeEvent::Response> response) -> void {
      fpga_interfaces::msg::ChargeEvent charge_event = request->charge_event;

      uint8_t channel = charge_event.channel;

      /* Serialize event info. */
      fpga_interfaces::msg::EventInfo event_info = charge_event.event_info;

      uint16_t event_id = event_info.event_id;
      uint8_t execution_condition = event_info.execution_condition;
      uint64_t time_us = event_info.time_us;

      serialized_message.init();
      serialized_message.add_uint16(event_id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_us);

      /* Serialize charge event. */

      // Charge event requires the channel here instead of the beginning of the message
      serialized_message.add_byte(channel);

      uint16_t target_voltage = charge_event.target_voltage;
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


      RCLCPP_INFO(rclcpp::get_logger("charge_event_handler"), "Sent charge request for channel %d", channel);

      response->success = true;
    };

    serialized_message = SerializedMessage();
    send_charge_event_service_ = this->create_service<fpga_interfaces::srv::SendChargeEvent>("/fpga/send_charge_event",
                                                                                             service_callback);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendChargeEvent>::SharedPtr send_charge_event_service_;
  SerializedMessage serialized_message;

};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

  auto node = std::make_shared<ChargeEventHandler>();

  RCLCPP_INFO(rclcpp::get_logger("charge_event_handler"), "Charge event handler ready.");

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_SCHEDULING_PRIORITY);
#endif

  rclcpp::spin(node);

  rclcpp::shutdown();

  close_fpga();
}
