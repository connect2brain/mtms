#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/event_trigger.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void callback([[maybe_unused]] const std::shared_ptr<event_interfaces::msg::EventTrigger> event_trigger) {
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_trigger_handler"), "FPGA not in OK state while attempting to trigger events.");
    return;
  }

  NiFpga_MergeStatus(&status,
                     NiFpga_WriteBool(session,
                                      NiFpga_mTMS_ControlBool_Eventtrigger,
                                      true));

  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Events triggered.");
}

class EventTriggerHandler : public rclcpp::Node {
public:
  EventTriggerHandler() : Node("event_trigger_handler") {

    event_trigger_subscriber_ = this->create_subscription<event_interfaces::msg::EventTrigger>(
        "/event/trigger", 10, callback);
  }

private:
  rclcpp::Subscription<event_interfaces::msg::EventTrigger>::SharedPtr event_trigger_subscriber_;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EventTriggerHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("event_trigger_handler"), "Event trigger handler ready.");

  init_fpga();

  auto timer = node->create_wall_timer(
      std::chrono::milliseconds(FPGA_OK_CHECK_INTERVAL_MS),
      [&]() {
          if (!is_fpga_ok()) {
              close_fpga();
              init_fpga();
          }
      }
  );
  rclcpp::spin(node);

  close_fpga();
  rclcpp::shutdown();
}
