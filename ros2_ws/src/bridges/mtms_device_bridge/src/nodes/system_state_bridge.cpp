#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "mtms_device_interfaces/msg/version.hpp"

#include "mtms_device_interfaces/msg/channel_state.hpp"
#include "mtms_device_interfaces/msg/channel_error.hpp"
#include "mtms_device_interfaces/msg/device_state.hpp"
#include "mtms_device_interfaces/msg/session_state.hpp"
#include "mtms_device_interfaces/msg/startup_error.hpp"
#include "mtms_device_interfaces/msg/system_state.hpp"
#include "mtms_device_interfaces/msg/system_error.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

using namespace std::chrono;
using namespace std::chrono_literals;

const uint8_t CHANNEL_COUNT = 5;
const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL = 20ms;
const milliseconds SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE = 5ms;

#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

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
    NiFpga_mTMS_IndicatorU16_Channel1Errors,
    NiFpga_mTMS_IndicatorU16_Channel2Errors,
    NiFpga_mTMS_IndicatorU16_Channel3Errors,
    NiFpga_mTMS_IndicatorU16_Channel4Errors,
    NiFpga_mTMS_IndicatorU16_Channel5Errors
};

NiFpga_mTMS_IndicatorU16 cumulative_error_indicator = NiFpga_mTMS_IndicatorU16_Cumulativeerrors;
NiFpga_mTMS_IndicatorU16 current_error_indicator = NiFpga_mTMS_IndicatorU16_Currenterrors;
NiFpga_mTMS_IndicatorU16 emergency_error_indicator = NiFpga_mTMS_IndicatorU16_Emergencyerrors;

NiFpga_mTMS_IndicatorU8 startup_error_indicator = NiFpga_mTMS_IndicatorU8_Startuperror;

NiFpga_mTMS_IndicatorU8 device_state_indicator = NiFpga_mTMS_IndicatorU8_Devicestate;
NiFpga_mTMS_IndicatorU8 session_state_indicator = NiFpga_mTMS_IndicatorU8_Sessionstate;

NiFpga_mTMS_IndicatorU64 time_indicator = NiFpga_mTMS_IndicatorU64_Time;

class SystemStateBridge : public rclcpp::Node {
public:
  SystemStateBridge()
      : Node("system_state_bridge") {

    const auto DEADLINE_NS = std::chrono::nanoseconds(SYSTEM_STATE_PUBLISHING_INTERVAL + SYSTEM_STATE_PUBLISHING_INTERVAL_TOLERANCE);

    auto qos = rclcpp::QoS(rclcpp::KeepLast(1))
        .reliability(RMW_QOS_POLICY_RELIABILITY_RELIABLE)
        .durability(RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL)
        .deadline(DEADLINE_NS)
        .lifespan(DEADLINE_NS);

    system_state_publisher_ = this->create_publisher<mtms_device_interfaces::msg::SystemState>(
        "/mtms_device/system_state", qos);
    timer_ = this->create_wall_timer(SYSTEM_STATE_PUBLISHING_INTERVAL, std::bind(&SystemStateBridge::publish_system_state, this));
  }

private:
  mtms_device_interfaces::msg::SystemError system_error_to_msg(uint16_t error) {
    auto msg = mtms_device_interfaces::msg::SystemError();

    msg.heartbeat_error = CHECK_BIT(error, 0);
    msg.latched_fault_error = CHECK_BIT(error, 1);
    msg.powersupply_error = CHECK_BIT(error, 2);
    msg.safety_bus_error = CHECK_BIT(error, 3);
    msg.coil_error = CHECK_BIT(error, 4);
    msg.emergency_button_error = CHECK_BIT(error, 5);
    msg.door_error = CHECK_BIT(error, 6);
    msg.charger_overvoltage_error = CHECK_BIT(error, 7);
    msg.charger_overtemperature_error = CHECK_BIT(error, 8);
    msg.monitored_voltage_over_maximum_error = CHECK_BIT(error, 9);
    msg.patient_safety_error = CHECK_BIT(error, 10);
    msg.device_safety_error = CHECK_BIT(error, 11);
    msg.charger_powerup_error = CHECK_BIT(error, 12);
    msg.opto_error = CHECK_BIT(error, 13);
    msg.charger_power_enabled_twice_error = CHECK_BIT(error, 14);

    return msg;
  }

  mtms_device_interfaces::msg::ChannelError channel_error_to_msg(uint16_t error) {
    auto msg = mtms_device_interfaces::msg::ChannelError();

    msg.overvoltage_error = CHECK_BIT(error, 0);
    msg.emergency_discharge_backup_power_error = CHECK_BIT(error, 1);
    msg.safety_bus_error = CHECK_BIT(error, 2);
    msg.powersupply_error = CHECK_BIT(error, 3);
    msg.safety_bus_startup_error = CHECK_BIT(error, 4);
    msg.acceptable_voltage_not_reached_startup_error = CHECK_BIT(error, 5);
    msg.maximum_safe_voltage_exceeded_startup_error = CHECK_BIT(error, 6);

    return msg;
  }

  void publish_system_state() {
    if (!is_fpga_ok()) {
      RCLCPP_WARN(rclcpp::get_logger("system_state_bridge"), "FPGA not in OK state while attempting to read system state");
      return;
    }

    mtms_device_interfaces::msg::SystemState state = mtms_device_interfaces::msg::SystemState();

    uint16_t error;

    for (auto i = 0; i < CHANNEL_COUNT; i++) {
      mtms_device_interfaces::msg::ChannelState channel_state = mtms_device_interfaces::msg::ChannelState();
      channel_state.channel_index = i;

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU16(
                             session,
                             voltage_indicators[i],
                             &channel_state.voltage
                         ));

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU16(
                             session,
                             temperature_indicators[i],
                             &channel_state.temperature
                         ));

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU32(
                             session,
                             pulse_count_indicators[i],
                             &channel_state.pulse_count
                         ));

      NiFpga_MergeStatus(&status,
                         NiFpga_ReadU16(
                             session,
                             channel_error_indicators[i],
                             &error
                         ));

      channel_state.channel_error = channel_error_to_msg(error);

      state.channel_states.push_back(channel_state);
    }

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            current_error_indicator,
                            &error
                        ));

    state.system_error_current = system_error_to_msg(error);

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            cumulative_error_indicator,
                            &error
                        ));

    state.system_error_cumulative = system_error_to_msg(error);

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU16(
                            session,
                            emergency_error_indicator,
                            &error
                        ));

    state.system_error_emergency = system_error_to_msg(error);

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU8(
                            session,
                            startup_error_indicator,
                            &state.startup_error.value
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU8(
                            session,
                            device_state_indicator,
                            &state.device_state.value
                        ));

    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU8(
                            session,
                            session_state_indicator,
                            &state.session_state.value
                        ));

    uint64_t time;
    NiFpga_MergeStatus(&status,
                        NiFpga_ReadU64(
                            session,
                            time_indicator,
                            &time
                        ));

    state.time = (double)time / CLOCK_FREQUENCY_HZ;

    system_state_publisher_->publish(state);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<mtms_device_interfaces::msg::SystemState>::SharedPtr system_state_publisher_;
};

int main(int argc, char **argv) {
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
