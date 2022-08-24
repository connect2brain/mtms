#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_discharge_event.hpp"
#include "fpga_interfaces/msg/discharge_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"

const NiFpga_mTMS_HostToTargetFifoU8 discharge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO;

void send_discharge_event(const std::shared_ptr<fpga_interfaces::srv::SendDischargeEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendDischargeEvent::Response> response)
{
  fpga_interfaces::msg::DischargeEvent discharge_event = request->discharge_event;

  uint8_t channel = discharge_event.channel;

  /* Serialize event info. */
  auto serialized_message = SerializedMessage(channel);

  fpga_interfaces::msg::EventInfo event_info = discharge_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t execution_condition = event_info.execution_condition;
  uint64_t time_us = event_info.time_us;
  uint32_t delay_us = event_info.delay_us;

  serialized_message.add_uint16(event_id);
  serialized_message.add_byte(execution_condition);
  serialized_message.add_uint64(time_us);
  serialized_message.add_uint32(delay_us);

  /* Serialize discharge event. */

  uint16_t target_voltage = discharge_event.target_voltage;
  serialized_message.add_uint16(target_voltage);

  serialized_message.finalize();

  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     discharge_fifo));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       discharge_fifo,
                       serialized_message.serialized_message,
                       serialized_message.get_length(),
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < serialized_message.get_length(); i++) {
    RCLCPP_INFO(rclcpp::get_logger("discharge_event_handler"), "%d,  %d", i - 1, serialized_message.serialized_message[i]);
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
