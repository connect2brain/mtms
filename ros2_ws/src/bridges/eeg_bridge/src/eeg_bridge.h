//
// Created by alqio on 12.12.2022.
//

#ifndef EEG_BRIDGE_EEG_BRIDGE_H
#define EEG_BRIDGE_EEG_BRIDGE_H

#include <netinet/in.h>
#include <cstdlib>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/string.hpp"
#include "eeg_interfaces/msg/eeg_sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"
#include "eeg_interfaces/msg/trigger.hpp"
#include "mtms_device_interfaces/msg/system_state.hpp"
#include "mtms_device_interfaces/msg/device_state.hpp"
#include "mtms_device_interfaces/msg/session_state.hpp"
#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#define BUFFER_LENGTH 250
#define MAX_NUMBER_OF_CHANNELS 80


using namespace std::chrono_literals;


enum EegBridgeState {
  WAITING_FOR_MTMS_DEVICE_START,
  WAITING_FOR_MEASUREMENT_START,
  WAITING_FOR_MEASUREMENT_STOP,
  WAITING_FOR_SESSION_STOP,
  WAITING_FOR_SESSION_START,
  STREAMING,
  ERROR_OUT_OF_SYNC
};

class EegBridge : public rclcpp::Node {

public:
  EegBridge();

  void create_publishers();
  void publish_healthcheck(uint8_t status_value, std::string status_message, std::string actionable_message);

  void subscribe_to_system_state();
  void wait_for_system_state();
  void spin();
  void init_socket();
  void err(const char *message);

  bool read_eeg_data_from_socket();
  double_t read_time_from_buffer(uint8_t index);
  void handle_trigger_packet();
  void handle_sample_packet();
  void handle_measurement_start_packet();
  void handle_eeg_data_packet();

  int get_trigger_package_from_buffer();
  void publish_trigger_from_buffer(double_t time);
  void publish_eeg_sample(double_t time_since_trigger);

  void handle_sync_trigger(double_t sync_time);
  void reset_session();

private:
  EegBridgeState eeg_bridge_state;

  mtms_device_interfaces::msg::DeviceState device_state;
  mtms_device_interfaces::msg::SessionState session_state;
  bool session_been_stopped;
  bool system_state_received;

  uint16_t sync_index;
  double_t time_correction;
  bool first_trigger_received;

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<eeg_interfaces::msg::EegSample>::SharedPtr publisher_data_;
  rclcpp::Publisher<eeg_interfaces::msg::Trigger>::SharedPtr publisher_trigger_;
  rclcpp::Publisher<eeg_interfaces::msg::EegInfo>::SharedPtr publisher_eeg_info_;
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr publisher_healthcheck_;

  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr subscription_system_state;

  double_t first_trigger_timestamp_;

  bool measurement_start_packet_received_;
  uint16_t num_of_channels_;
  uint16_t num_of_channels_excluding_trigger_;

  /* TODO: Sampling frequency is unused for now. It could be published either as ROS message
   *   or as metadata of EEG data.
   */
  uint32_t sampling_frequency_;

  uint16_t port_;
  int socket_;
  sockaddr_in socket_own;
  sockaddr_in socket_other;
  socklen_t socket_length;
  uint8_t buffer[BUFFER_LENGTH];
  bool first_sample_of_session_;

  enum ChannelType {
    EEG, EMG
  };
  ChannelType channel_types[MAX_NUMBER_OF_CHANNELS];
  bool send_trigger_as_channel;
};

#endif //EEG_BRIDGE_EEG_BRIDGE_H
