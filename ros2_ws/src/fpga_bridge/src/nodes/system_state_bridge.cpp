#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/version.hpp"

#include "fpga_interfaces/msg/system_state.hpp"
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

NiFpga_mTMS_IndicatorU32 pulse_count_indicators[CHANNEL_COUNT] = {
    NiFpga_mTMS_IndicatorU32_Coil1Pulsecount,
    NiFpga_mTMS_IndicatorU32_Coil2Pulsecount,
    NiFpga_mTMS_IndicatorU32_Coil3Pulsecount,
    NiFpga_mTMS_IndicatorU32_Coil4Pulsecount,
    NiFpga_mTMS_IndicatorU32_Coil5Pulsecount
};

NiFpga_mTMS_IndicatorU16 channel_error_indicators[CHANNEL_COUNT] = {
    NiFpga_mTMS_IndicatorU16_Channel1ErrorsDC,
    NiFpga_mTMS_IndicatorU16_Channel2ErrorsDC,
    NiFpga_mTMS_IndicatorU16_Channel3ErrorsDC,
    NiFpga_mTMS_IndicatorU16_Channel4ErrorsDC,
    NiFpga_mTMS_IndicatorU16_Channel5ErrorsDC
};

NiFpga_mTMS_IndicatorU16 cumulative_error_indicator = NiFpga_mTMS_IndicatorU16_CumulativeerrorsSM;
NiFpga_mTMS_IndicatorU16 current_error_indicator = NiFpga_mTMS_IndicatorU16_CurrenterrorsSM;
NiFpga_mTMS_IndicatorU16 emergency_error_indicator = NiFpga_mTMS_IndicatorU16_EmergencyerrorsSM;

NiFpga_mTMS_IndicatorU16 startup_sequence_error_indicator = NiFpga_mTMS_IndicatorU16_Startupsequenceerror;

NiFpga_mTMS_IndicatorU8 device_state_indicator = NiFpga_mTMS_IndicatorU8_Devicestate;
NiFpga_mTMS_IndicatorU8 experiment_state_indicator = NiFpga_mTMS_IndicatorU8_Experimentstate;

NiFpga_mTMS_IndicatorU64 time_indicator = NiFpga_mTMS_IndicatorU64_time;

class SystemStateBridge : public rclcpp::Node {
public:
  SystemStateBridge()
      : Node("system_state_bridge") {
    system_state_publisher_ = this->create_publisher<fpga_interfaces::msg::SystemState>(
        "/fpga/system_state", 10);
    timer_ = this->create_wall_timer(20ms, std::bind(&SystemStateBridge::publish_system_state, this));
  }

private:
  void publish_system_state() {

    fpga_interfaces::msg::SystemState state = fpga_interfaces::msg::SystemState();

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

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU32(
                             session,
                             pulse_count_indicators[i],
                             &channel_status.pulse_count
                         ));

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU16(
                             session,
                             channel_error_indicators[i],
                             &channel_status.error
                         ));

      state.channel_statuses.push_back(channel_status);
    }

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            cumulative_error_indicator,
                            &state.cumulative_error
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            current_error_indicator,
                            &state.current_error
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            emergency_error_indicator,
                            &state.emergency_error
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            startup_sequence_error_indicator,
                            &state.startup_sequence_error
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU8(
                            session,
                            device_state_indicator,
                            &state.device_state
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU8(
                            session,
                            experiment_state_indicator,
                            &state.experiment_state
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU64(
                            session,
                            time_indicator,
                            &state.time
                        ));

    system_state_publisher_->publish(state);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<fpga_interfaces::msg::SystemState>::SharedPtr system_state_publisher_;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("system_state_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<SystemStateBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("system_state_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("system_state_bridge"), "System state bridge ready.");


  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
