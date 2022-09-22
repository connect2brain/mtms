#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/version.hpp"

#include "fpga_interfaces/msg/status_monitor_state.hpp"
#include "fpga_interfaces/msg/channel_status.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

#define CHANNEL_COUNT 5

using namespace std::chrono_literals;

NiFpga_mTMS_IndicatorU16 voltage_indicators[CHANNEL_COUNT] = {
    NiFpga_mTMS_IndicatorU16_Channel1Capacitorvoltage,
    NiFpga_mTMS_IndicatorU16_Channel2Capacitorvoltage,
    NiFpga_mTMS_IndicatorU16_Channel3Capacitorvoltage,
    NiFpga_mTMS_IndicatorU16_Channel4Capacitorvoltage,
    NiFpga_mTMS_IndicatorU16_Channel5Capacitorvoltage
};

NiFpga_mTMS_IndicatorU16 temperature_indicators[CHANNEL_COUNT] = {
    NiFpga_mTMS_IndicatorU16_Coil1Temperature,
    NiFpga_mTMS_IndicatorU16_Coil2Temperature,
    NiFpga_mTMS_IndicatorU16_Coil3Temperature,
    NiFpga_mTMS_IndicatorU16_Coil4Temperature,
    NiFpga_mTMS_IndicatorU16_Coil5Temperature
};

class StatusMonitorBridge : public rclcpp::Node {
public:
  StatusMonitorBridge()
      : Node("status_monitor_bridge") {
    status_monitor_publisher_ = this->create_publisher<fpga_interfaces::msg::StatusMonitorState>(
        "/fpga/status_monitor_state", 10);
    timer_ = this->create_wall_timer(20ms, std::bind(&StatusMonitorBridge::publish_status_monitor_state, this));
  }

private:
  void publish_status_monitor_state() {

    fpga_interfaces::msg::StatusMonitorState state = fpga_interfaces::msg::StatusMonitorState();

    for (auto i = 0; i < CHANNEL_COUNT; i++) {
      fpga_interfaces::msg::ChannelStatus channel_status = fpga_interfaces::msg::ChannelStatus();
      channel_status.channel_index = i;

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU16(
                             session,
                             voltage_indicators[i],
                             &channel_status.voltage
                         ));

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU16(
                             session,
                             temperature_indicators[i],
                             &channel_status.temperature
                         ));

      state.channel_statuses.push_back(channel_status);
    }

    status_monitor_publisher_->publish(state);

  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<fpga_interfaces::msg::StatusMonitorState>::SharedPtr status_monitor_publisher_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("status_monitor_bridge"), "Setting thread scheduling and memory lock");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<StatusMonitorBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("status_monitor_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("status_monitor_bridge"), "Status monitor bridge ready.");


  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
