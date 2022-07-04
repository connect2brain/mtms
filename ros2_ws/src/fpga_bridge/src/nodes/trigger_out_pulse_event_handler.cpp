#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_trigger_out_pulse_event.hpp"
#include "fpga_interfaces/msg/trigger_out_pulse_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"

const NiFpga_mTMS_HostToTargetFifoU8 trigger_out_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetSignalOutFIFO;

void send_trigger_out_pulse_event(const std::shared_ptr<fpga_interfaces::srv::SendTriggerOutPulseEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendTriggerOutPulseEvent::Response> response)
{
  fpga_interfaces::msg::TriggerOutPulseEvent trigger_out_pulse_event = request->trigger_out_pulse_event;

  uint8_t index = trigger_out_pulse_event.index;

  /* Serialize event info. */
  auto serialized_message = SerializedMessage(index);

  fpga_interfaces::msg::EventInfo event_info = trigger_out_pulse_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t wait_for_trigger = event_info.wait_for_trigger;
  uint64_t time_us = event_info.time_us;
  uint32_t delay_us = event_info.delay_us;

  serialized_message.add_uint16(event_id);
  serialized_message.add_byte(wait_for_trigger);
  serialized_message.add_uint64(time_us);
  serialized_message.add_uint32(delay_us);

  /* Serialize trigger out pulse. */

  uint32_t duration_us = (uint8_t) trigger_out_pulse_event.duration_us;
  serialized_message.add_uint32(duration_us);

  serialized_message.finalize();

  /* For consistency with channel indexing, start trigger out indexing from 1. */
  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     trigger_out_pulse_fifo));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       trigger_out_pulse_fifo,
                       serialized_message.serialized_message,
                       serialized_message.get_length(),
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i <  serialized_message.get_length(); i++) {
    RCLCPP_INFO(rclcpp::get_logger("fpga"), "%d,  %d", i - 1, serialized_message.serialized_message[i]);
  }

  response->success = true;
}

class TriggerOutPulseEventHandler : public rclcpp::Node
{
  public:
    TriggerOutPulseEventHandler()
    : Node("trigger_out_pulse_event_handler")
    {
      send_trigger_out_pulse_event_service_ = this->create_service<fpga_interfaces::srv::SendTriggerOutPulseEvent>("/fpga/send_trigger_out_pulse_event", send_trigger_out_pulse_event);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SendTriggerOutPulseEvent>::SharedPtr send_trigger_out_pulse_event_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("trigger_out_pulse_event_handler"), "Trigger out pulse event handler ready.");

  rclcpp::spin(std::make_shared<TriggerOutPulseEventHandler>());
  rclcpp::shutdown();

  close_fpga();
}
