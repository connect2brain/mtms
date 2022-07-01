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
  init_serialized_message();

  fpga_interfaces::msg::ChargeEvent charge_event = request->charge_event;

  uint8_t channel = charge_event.channel;

  /* Serialize event info. */

  fpga_interfaces::msg::EventInfo event_info = charge_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t wait_for_trigger = event_info.wait_for_trigger;
  uint64_t time_us = event_info.time_us;
  uint32_t delay_us = event_info.delay_us;

  add_uint16_to_serialized_message(event_id);
  add_byte_to_serialized_message(wait_for_trigger);
  add_uint64_to_serialized_message(time_us);
  add_uint32_to_serialized_message(delay_us);

  /* Serialize charge event. */

  uint16_t target_voltage = charge_event.target_voltage;
  add_uint16_to_serialized_message(target_voltage);

  finalize_serialized_message();

  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     charge_fifo));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       charge_fifo,
                       serialized_message,
                       length,
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < length; i++) {
    RCLCPP_INFO(rclcpp::get_logger("charge_event_handler"), "%d,  %d", i - 1, serialized_message[i]);
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
