//
// Created by alqio on 12.12.2022.
//

#ifndef EEG_BRIDGE_EEG_BRIDGE_H
#define EEG_BRIDGE_EEG_BRIDGE_H

#include <netinet/in.h>
#include <cstdlib>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "eeg_interfaces/msg/eeg_datapoint.hpp"
#include "eeg_interfaces/msg/trigger.hpp"
#include "mtms_device_interfaces/msg/system_state.hpp"
#include "mtms_device_interfaces/msg/experiment_state.hpp"

#define BUFFER_LENGTH 250
#define MAX_NUMBER_OF_CHANNELS 80


using namespace std::chrono_literals;


class EegBridge : public rclcpp::Node {

public:
  EegBridge();

  void set_channel_types();

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

  void publish_eeg_datapoint(double_t time_since_trigger);

  void handle_sync_trigger(double_t sync_time);

  void reset_sync();

private:

  mtms_device_interfaces::msg::ExperimentState experiment_state;

  double_t sync_interval;
  uint16_t sync_index;
  double_t time_correction;
  bool first_trigger_received;

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<eeg_interfaces::msg::EegDatapoint>::SharedPtr publisher_data_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_streaming_;
  rclcpp::Publisher<eeg_interfaces::msg::Trigger>::SharedPtr publisher_trigger_;

  rclcpp::Subscription<mtms_device_interfaces::msg::SystemState>::SharedPtr subscription_system_state;

  double_t first_trigger_timestamp_;

  bool measurement_start_packet_received_;
  uint16_t n_channels_;
  uint16_t n_channels_excluding_trigger_;

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
  bool first_sample_of_experiment_;

  enum ChannelType {
    EEG, EMG
  };
  ChannelType channel_types[MAX_NUMBER_OF_CHANNELS];
  bool send_trigger_as_channel;

  uint8_t eeg_channels_primary_amplifier_;
  uint8_t emg_channels_primary_amplifier_;
  uint8_t eeg_channels_secondary_amplifier_;
  uint8_t emg_channels_secondary_amplifier_;
};


#endif //EEG_BRIDGE_EEG_BRIDGE_H
