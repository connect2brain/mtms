#include <chrono>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_event_trigger.hpp"

#include "NiFpga_board_control.h"

using namespace std::chrono_literals;
using std::placeholders::_1;

NiFpga_Session session;
NiFpga_Status status;
bool fpga_opened = false;

bool init_fpga(void) {
  /* must be called before any other calls */
  status = NiFpga_Initialize();
  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "FPGA could not be initialized, exiting.");
    return false;
  }

  /* opens a session, downloads the bitstream, and runs the FPGA */

  /* TODO: Remove hardcoded bitfile. */
  NiFpga_MergeStatus(&status, NiFpga_Open("C:\\Users\\mTMS\\mtms\\bitfiles\\NiFpga_board_control_0_1_3.lvbitx",
          NiFpga_board_control_Signature,
          "PXI1Slot4",
          NiFpga_OpenAttribute_NoRun,
          &session));

  if (NiFpga_IsError(status)) {
      RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "FPGA bitfile could not be loaded, exiting.");
      return false;
  }

  fpga_opened = true;
  return true;
}

bool close_fpga(void) {
  if (fpga_opened) {
    /* must close if we successfully opened */
    NiFpga_MergeStatus(&status, NiFpga_Close(session, 0));
  }

  /* must be called after all other calls */
  NiFpga_MergeStatus(&status, NiFpga_Finalize());

  return true;
}

void send_event_trigger(const std::shared_ptr<fpga_interfaces::srv::SendEventTrigger::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendEventTrigger::Response> response)
{
  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_board_control_ControlBool_stimulation__event_trigger,
                                      true));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Sent the event trigger");
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
