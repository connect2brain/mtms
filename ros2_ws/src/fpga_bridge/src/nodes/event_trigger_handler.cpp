#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_event_trigger.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"

void send_event_trigger(const std::shared_ptr<fpga_interfaces::srv::SendEventTrigger::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendEventTrigger::Response> response)
{
  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_mTMS_ControlBool_Eventtrigger,
                                      true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Sent event trigger");
}

class SendEventTriggerHandler : public rclcpp::Node
{
  public:
    SendEventTriggerHandler()
    : Node("event_trigger_handler")
    {
      send_event_trigger_service_ = this->create_service<fpga_interfaces::srv::SendEventTrigger>("/fpga/send_event_trigger", send_event_trigger);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SendEventTrigger>::SharedPtr send_event_trigger_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Event trigger handler ready.");

  rclcpp::spin(std::make_shared<SendEventTriggerHandler>());
  rclcpp::shutdown();

  close_fpga();
}
