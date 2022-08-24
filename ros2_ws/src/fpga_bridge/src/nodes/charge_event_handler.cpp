#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_charge_event.hpp"
#include "fpga_interfaces/msg/charge_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"

const NiFpga_mTMS_HostToTargetFifoU8 charge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO;

void send_charge_event(const std::shared_ptr<fpga_interfaces::srv::SendChargeEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendChargeEvent::Response> response)
{
  fpga_interfaces::msg::ChargeEvent charge_event = request->charge_event;

  uint8_t channel = charge_event.channel;

  /* Serialize event info. */
  auto serialized_message = SerializedMessage();
  serialized_message.init_without_channel();

  fpga_interfaces::msg::EventInfo event_info = charge_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t execution_condition = event_info.execution_condition;
  uint64_t time_us = event_info.time_us;
  uint32_t delay_us = event_info.delay_us;

  serialized_message.add_uint16(event_id);
  serialized_message.add_byte(execution_condition);
  serialized_message.add_uint64(time_us);
  serialized_message.add_uint32(delay_us);

  /* Serialize charge event. */

  //Charge event requires the channel here instead of the beginning of the message
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
                       serialized_message.serialized_message,
                       serialized_message.get_length(),
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < serialized_message.get_length(); i++) {
    RCLCPP_INFO(rclcpp::get_logger("charge_event_handler"), "%d,  %d", i - 1, serialized_message.serialized_message[i]);
  }

  response->success = true;
}

class ChargeEventHandler : public rclcpp::Node
{
  public:
    ChargeEventHandler()
    : Node("charge_event_handler")
    {
      send_charge_event_service_ = this->create_service<fpga_interfaces::srv::SendChargeEvent>("/fpga/send_charge_event", send_charge_event);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SendChargeEvent>::SharedPtr send_charge_event_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("charge_event_handler"), "Charge event handler ready.");

  rclcpp::spin(std::make_shared<ChargeEventHandler>());
  rclcpp::shutdown();

  close_fpga();
}
