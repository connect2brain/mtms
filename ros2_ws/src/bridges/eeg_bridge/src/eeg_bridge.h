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

#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"
#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

#define BUFFER_LENGTH 250
#define MAX_NUMBER_OF_CHANNELS 80


using namespace std::chrono_literals;


enum EegBridgeState {
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

  void update_healthcheck(uint8_t status, std::string status_message, std::string actionable_message);
  void publish_healthcheck();

  void subscribe_to_session();
  void wait_for_session();

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

  system_interfaces::msg::SessionState session_state;
  bool session_been_stopped;
  bool session_received;

  uint16_t sync_index;
  double_t time_correction;
  bool first_trigger_received;

  rclcpp::TimerBase::SharedPtr healthcheck_publisher_timer;
  rclcpp::Publisher<eeg_interfaces::msg::EegSample>::SharedPtr eeg_sample_publisher;
  rclcpp::Publisher<eeg_interfaces::msg::Trigger>::SharedPtr trigger_publisher;
  rclcpp::Publisher<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_publisher;
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr publisher_healthcheck_;

  rclcpp::Subscription<system_interfaces::msg::Session>::SharedPtr session_subscriber;

  double_t first_trigger_timestamp_;

  bool measurement_start_packet_received_;
  uint16_t num_of_channels_;
  uint16_t num_of_channels_excluding_trigger_;

  uint32_t sampling_frequency_;
  uint32_t sample_packets_received_ = 0;

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

  /* Healthcheck */
  uint8_t status = system_interfaces::msg::HealthcheckStatus::NOT_READY;
  std::string status_message = "";
  std::string actionable_message = "";
};

#endif //EEG_BRIDGE_EEG_BRIDGE_H
