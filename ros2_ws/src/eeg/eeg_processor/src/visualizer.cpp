#include "rclcpp/rclcpp.hpp"

#include "mtms_interfaces/msg/event.hpp"

#include "processor_node.h"
#include "memory_utils.h"
#include "scheduling_utils.h"


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("visualizer"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<ProcessorNode<mtms_interfaces::msg::Event, mtms_interfaces::msg::Event>>(
      "visualizer",
      "/visualizer/events",
      "/mtms/events"
  );

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("visualizer"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
