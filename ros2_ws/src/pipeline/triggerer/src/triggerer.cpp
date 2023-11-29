#include <libserialport.h>
#include <chrono>
#include <thread>

#include "triggerer.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

using namespace std::placeholders;

const std::string TRIGGER_TOPIC = "/event/trigger";

Triggerer::Triggerer() : Node("triggerer"), logger(rclcpp::get_logger("triggerer")) {
  /* Subscriber for mTMS device healthcheck. */
  this->mtms_device_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/mtms_device/healthcheck",
    10,
    std::bind(&Triggerer::handle_mtms_device_healthcheck, this, _1));

  /* Subscriber for event trigger. */
  this->event_trigger_subscriber = create_subscription<event_interfaces::msg::EventTrigger>(
    TRIGGER_TOPIC,
    10,
    std::bind(&Triggerer::process_event_trigger, this, _1));

  /* Initialize the serial port. */
  sp_get_port_by_name("/dev/ttyUSB0", &port);
  sp_open(port, SP_MODE_READ_WRITE);

  sp_start_break(port);
}

Triggerer::~Triggerer() {
  sp_close(port);
  sp_free_port(port);
}

void Triggerer::handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  this->mtms_device_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;
}

void Triggerer::process_event_trigger(const std::shared_ptr<event_interfaces::msg::EventTrigger> msg) {
  RCLCPP_INFO(logger, "Event trigger received.");

  sp_end_break(port);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  sp_start_break(port);
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("triggerer"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<Triggerer>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("triggerer"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
