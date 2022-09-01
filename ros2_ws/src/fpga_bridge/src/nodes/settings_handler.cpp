#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_settings.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

void send_settings(const std::shared_ptr<fpga_interfaces::srv::SendSettings::Request> request,
                   std::shared_ptr<fpga_interfaces::srv::SendSettings::Response> response) {

  auto settings = request->settings;


  NiFpga_MergeStatus(&status,
                     NiFpga_WriteU16(session,
                                     NiFpga_mTMS_ControlU16_Maximumrisingfallingdifferenceticks,
                                     settings.maximum_rising_falling_difference_ticks));


  NiFpga_MergeStatus(&status,
                     NiFpga_WriteU16(session,
                                     NiFpga_mTMS_ControlU16_Maximumpulsedurationticks,
                                     settings.maximum_pulse_duration_ticks));


  NiFpga_MergeStatus(&status,
                     NiFpga_WriteU16(session,
                                     NiFpga_mTMS_ControlU16_Maximumpulsespertimetimems,
                                     settings.time_in_maximum_pulses_per_time_ms));

  NiFpga_MergeStatus(&status,
                     NiFpga_WriteU8(session,
                                    NiFpga_mTMS_ControlU8_Maximumpulsespertimepulses,
                                    settings.pulses_in_maximum_pulses_per_time));

  response->success = true;
  RCLCPP_INFO(rclcpp::get_logger("settings_handler"),
              "Sent settings: maximum_rising_falling_difference_ticks %d, maximum_pulse_duration_ticks %d, time_in_maximum_pulses_per_time_ms %d, pulses_in_maximum_pulses_per_time %d",
              settings.maximum_rising_falling_difference_ticks,
              settings.maximum_pulse_duration_ticks,
              settings.time_in_maximum_pulses_per_time_ms,
              settings.pulses_in_maximum_pulses_per_time);
}

class SettingsHandler : public rclcpp::Node {
public:
  SettingsHandler()
      : Node("settings_handler") {
    send_settings_service_ = this->create_service<fpga_interfaces::srv::SendSettings>("/fpga/send_settings",
                                                                                      send_settings);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendSettings>::SharedPtr send_settings_service_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);
  auto node = std::make_shared<SettingsHandler>();
  RCLCPP_INFO(rclcpp::get_logger("settings_handler"), "Settings handler ready.");
#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif
  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
