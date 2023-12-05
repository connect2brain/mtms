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

#include "eeg_interfaces/msg/sample.hpp"
#include "eeg_interfaces/msg/eeg_info.hpp"
#include "eeg_interfaces/msg/trigger.hpp"

#include "system_interfaces/msg/session.hpp"
#include "system_interfaces/msg/session_state.hpp"

#include "system_interfaces/msg/healthcheck.hpp"
#include "system_interfaces/msg/healthcheck_status.hpp"

/* The maximum length of the UDP packet, as mentioned in the manual of Bittium NeurOne. */
#define BUFFER_LENGTH 1472

/* The maximum number of channels as supported by four amplifiers (40 channels per amplifier). */
#define MAX_NUMBER_OF_CHANNELS 160


using namespace std::chrono_literals;

const uint16_t UNSET_SAMPLING_FREQUENCY = 0;
const double_t UNSET_PREVIOUS_TIME = std::numeric_limits<double_t>::quiet_NaN();

enum EegBridgeState {
  WAITING_FOR_MEASUREMENT_START,
  WAITING_FOR_MEASUREMENT_STOP,
  WAITING_FOR_SESSION_STOP,
  WAITING_FOR_SESSION_START,
  STREAMING,
  ERROR_OUT_OF_SYNC,
  ERROR_SAMPLES_DROPPED
};

class EegBridge : public rclcpp::Node {

public:
  EegBridge();

  void create_publishers();

  void update_healthcheck(uint8_t status, std::string status_message, std::string actionable_message);
  void publish_healthcheck();

  void handle_mtms_device_healthcheck(const std::shared_ptr<system_interfaces::msg::Healthcheck> msg);

  void subscribe_to_session();
  void reset_session();
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

  void check_dropped_samples(double_t sample_time);

  int get_trigger_data_from_sample_packet();
  void handle_trigger_in_sample_packet(double_t time);
  void publish_eeg_sample(double_t time);

  void handle_sync_trigger(double_t sync_time);

private:
  EegBridgeState eeg_bridge_state;

  system_interfaces::msg::SessionState session_state;
  bool session_been_stopped;
  bool session_received;

  uint16_t num_of_sync_triggers_received;
  double_t time_correction;
  bool sync_trigger_received;

  rclcpp::Subscription<system_interfaces::msg::Healthcheck>::SharedPtr mtms_device_healthcheck_subscriber;

  rclcpp::TimerBase::SharedPtr healthcheck_publisher_timer;

  rclcpp::Publisher<eeg_interfaces::msg::Sample>::SharedPtr eeg_sample_publisher;
  rclcpp::Publisher<eeg_interfaces::msg::Trigger>::SharedPtr trigger_publisher;
  rclcpp::Publisher<eeg_interfaces::msg::EegInfo>::SharedPtr eeg_info_publisher;
  rclcpp::Publisher<system_interfaces::msg::Healthcheck>::SharedPtr healthcheck_publisher;

  rclcpp::Subscription<system_interfaces::msg::Session>::SharedPtr session_subscriber;

  double_t first_sync_trigger_timestamp;

  bool measurement_start_packet_received_;

  uint8_t num_of_eeg_channels;
  uint8_t num_of_emg_channels;

  uint16_t num_of_channels_;
  uint16_t num_of_channels_excluding_trigger_;

  uint32_t sampling_frequency = UNSET_SAMPLING_FREQUENCY;
  uint32_t sample_packets_received_ = 0;

  double_t sampling_period;

  double_t previous_time = UNSET_PREVIOUS_TIME;

  uint16_t port_;
  int socket_;
  sockaddr_in socket_own;
  sockaddr_in socket_other;
  socklen_t socket_length;
  uint8_t buffer[BUFFER_LENGTH];
  bool first_sample_of_session;

  enum ChannelType {
    EEG, EMG
  };
  ChannelType channel_types[MAX_NUMBER_OF_CHANNELS];
  bool send_trigger_as_channel;

  bool upcoming_trigger = false;
  double_t upcoming_trigger_time;

  /* Healthcheck */
  uint8_t status = system_interfaces::msg::HealthcheckStatus::NOT_READY;
  std::string status_message = "";
  std::string actionable_message = "";

  /* mTMS device healthcheck */
  bool mtms_device_available = false;

  /* When determining if samples have been dropped by comparing the timestamps of two consecutive
     samples, allow some tolerance to account for finite precision of floating point numbers. */
  static constexpr double_t TOLERANCE_S = 2 * pow(10, -5);
};

#endif //EEG_BRIDGE_EEG_BRIDGE_H
