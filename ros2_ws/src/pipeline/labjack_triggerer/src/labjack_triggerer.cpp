#include <chrono>
#include <thread>

#include <LabJackM.h>
#include "LJM_Utilities.h"

#include "labjack_triggerer.h"

#include "memory_utils.h"
#include "scheduling_utils.h"

using namespace std::placeholders;

const std::string LABJACK_TRIGGER_SERVICE = "/labjack/trigger";

LabjackTriggerer::LabjackTriggerer() : Node("labjack_triggerer"), logger(rclcpp::get_logger("labjack_triggerer")) {
  /* Subscriber for mTMS device healthcheck. */
  this->mtms_device_healthcheck_subscriber = create_subscription<system_interfaces::msg::Healthcheck>(
    "/mtms_device/healthcheck",
    10,
    std::bind(&LabjackTriggerer::handle_mtms_device_healthcheck, this, _1));

  /* Service for trigger request. */
  this->trigger_request_service = create_service<system_interfaces::srv::RequestTrigger>(
    LABJACK_TRIGGER_SERVICE,
    std::bind(&LabjackTriggerer::request_trigger, this, _1, _2));

  /* Attempt initial connection to LabJack. */
  attempt_labjack_connection();

  /* Set up a timer to attempt reconnection every second. */
  timer = this->create_wall_timer(
    std::chrono::seconds(1),
    std::bind(&LabjackTriggerer::attempt_labjack_connection, this));
}

LabjackTriggerer::~LabjackTriggerer() {
  if (labjack_handle != -1) {
    CloseOrDie(labjack_handle);
  }
}

void LabjackTriggerer::attempt_labjack_connection() {
  if (labjack_handle == -1) {
    /* Attempt to open a connection to the LabJack device. */
    int err = LJM_Open(LJM_dtANY, LJM_ctANY, "LJM_idANY", &labjack_handle);
    if (err != LJME_NOERROR) {
      /* Ensure that handle is marked as invalid. */
      labjack_handle = -1;
      RCLCPP_WARN(logger, "Failed to connect to LabJack. Error code: %d", err);
    } else {
      PrintDeviceInfoFromHandle(labjack_handle);
      RCLCPP_INFO(logger, "Successfully connected to LabJack.");
    }
  }
}

bool LabjackTriggerer::safe_error_check(int err, const char* action) {
  if (err != LJME_NOERROR) {
    RCLCPP_ERROR(logger, "%s failed with error code: %d", action, err);
    if (err == LJME_RECONNECT_FAILED) {
      /* Mark as disconnected. */
      labjack_handle = -1;
      RCLCPP_WARN(logger, "LabJack connection lost. Will attempt to reconnect.");
    }
    return false;
  }
  return true;
}

void LabjackTriggerer::handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg) {
  this->mtms_device_available = msg->status.value == system_interfaces::msg::HealthcheckStatus::READY;
}

void LabjackTriggerer::request_trigger(
    [[maybe_unused]] const std::shared_ptr<system_interfaces::srv::RequestTrigger::Request> request,
    std::shared_ptr<system_interfaces::srv::RequestTrigger::Response> response) {

  RCLCPP_INFO(logger, "LabJack trigger requested.");

  if (labjack_handle == -1) {
    RCLCPP_WARN(logger, "LabJack is not connected. Unable to trigger.");
    response->success = false;
    return;
  }

  const char* name = "FIO4";

  /* Set output port state to high. */
  int err = LJM_eWriteName(labjack_handle, name, 1);
  if (!safe_error_check(err, "Setting digital output on LabJack")) {
    response->success = false;
    return;
  }

  /* Wait for one millisecond. */
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  /* Set output port state to low. */
  err = LJM_eWriteName(labjack_handle, name, 0);
  if (!safe_error_check(err, "Setting digital output on LabJack")) {
    response->success = false;
    return;
  }

  RCLCPP_INFO(logger, "Triggered LabJack on output port %s", name);
  response->success = true;
}

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("labjack_triggerer"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<LabjackTriggerer>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("labjack_triggerer"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();

  return 0;
}
