#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_discharge_event.hpp"
#include "fpga_interfaces/msg/discharge_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_board_control.h"
#include "fpga.h"
#include "serdes.h"

const uint8_t discharge_fifos[5] = {
  0, /* Not in use. */ //NiFpga_board_control_HostToTargetFifoU8_Channel1DischargeFIFO,
  0, /* Not in use. */
  0, /* Not in use. */
  0, /* Not in use. */
  0  /* Not in use. */
};

void send_discharge_event(const std::shared_ptr<fpga_interfaces::srv::SendDischargeEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendDischargeEvent::Response> response)
{
  init_serialized_message();

  fpga_interfaces::msg::DischargeEvent discharge_event = request->discharge_event;

  uint8_t channel = discharge_event.channel;

  /* Serialize event info. */

  fpga_interfaces::msg::EventInfo event_info = discharge_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t wait_for_trigger = event_info.wait_for_trigger;
  uint64_t time_us = event_info.time_us;
  uint32_t delay_us = event_info.delay_us;

  add_uint16_to_serialized_message(event_id);
  add_byte_to_serialized_message(wait_for_trigger);
  add_uint64_to_serialized_message(time_us);
  add_uint32_to_serialized_message(delay_us);

  /* Serialize discharge event. */

  uint16_t target_voltage = discharge_event.target_voltage;
  add_uint16_to_serialized_message(target_voltage);

  finalize_serialized_message();

  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     discharge_fifos[channel - 1]));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       discharge_fifos[channel - 1],
                       serialized_message,
                       length,
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < length; i++) {
    RCLCPP_INFO(rclcpp::get_logger("discharge_event_handler"), "%d,  %d", i - 1, serialized_message[i]);
  }

  response->success = true;
}

class DischargeEventHandler : public rclcpp::Node
{
  public:
    DischargeEventHandler()
    : Node("discharge_event_handler")
    {
      send_discharge_event_service_ = this->create_service<fpga_interfaces::srv::SendDischargeEvent>("/fpga/send_discharge_event", send_discharge_event);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SendDischargeEvent>::SharedPtr send_discharge_event_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("discharge_event_handler"), "Discharge event handler ready.");

  rclcpp::spin(std::make_shared<DischargeEventHandler>());
  rclcpp::shutdown();

  close_fpga();
}
