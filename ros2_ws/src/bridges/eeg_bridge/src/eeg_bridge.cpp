#include <chrono>
#include <functional>
#include <memory>
#include <cmath>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include "scheduling_utils.h"
#include "memory_utils.h"

#define MAX_NUMBER_OF_CHANNELS 80

#define BUFFER_LENGTH 250

#define SIGNED_MAX pow(2,23)
#define UNSIGNED_MAX pow(2,24)
#define DEFAULT_FREQUENCY_VALUE 500.0 // Hz
#define DC_MODE_SCALE 100
#define AC_MODE_SCALE 20
#define NANO_TO_MICRO_CONVERSION 1000

#define MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX 4
#define MEASUREMENT_START_PACKET_N_CHANNELS_INDEX 16
#define MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX 18

/* HACK: If source channel matches the value below, it indicates the existence
 *  of trigger in the sample packet. This is documented in NeurOne's manual.
 *  However, it would be cleaner to have a separate field in measurement start
 *  packet to indicate the existence of trigger. */
#define SOURCE_CHANNEL_FOR_TRIGGER 65535

#define SAMPLE_PACKET_N_BUNDLES_INDEX 10

#define FIRST_CHANNEL_INDEX 28
#define SAMPLE_PACKET_FIRST_TIME_INDEX 20
#define TRIGGER_PACKET_FIRST_TIME_INDEX 8
#define TRIGGER_PORT_INDEX 24

#define MEASUREMENT_START_PACKET_ID 1
#define SAMPLE_PACKET_ID 2
#define TRIGGER_PACKET_ID 3

#define TRIGGER_A_IN 2
#define TRIGGER_B_IN 8

#define VERBOSE 0

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"
#include "mtms_interfaces/msg/trigger.hpp"

using namespace std::chrono_literals;


class EegBridge : public rclcpp::Node {

public:
  EegBridge() : Node("eeg_bridge") {

    static const rmw_qos_profile_t qos_profile = {
        RMW_QOS_POLICY_HISTORY_KEEP_LAST,
        1,
        RMW_QOS_POLICY_RELIABILITY_RELIABLE,
        RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL,
        RMW_QOS_DEADLINE_DEFAULT,
        RMW_QOS_LIFESPAN_DEFAULT,
        RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT,
        RMW_QOS_LIVELINESS_LEASE_DURATION_DEFAULT,
        false
    };

    auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, qos_profile.depth), qos_profile);

    publisher_data_ = this->create_publisher<mtms_interfaces::msg::EegDatapoint>("/eeg/raw_data", 10);
    publisher_streaming_ = this->create_publisher<std_msgs::msg::Bool>("/eeg/is_streaming", qos);
    publisher_trigger_ = this->create_publisher<mtms_interfaces::msg::Trigger>("/eeg/trigger_received", qos);

    auto descriptor = rcl_interfaces::msg::ParameterDescriptor{};

    descriptor.description = "Port";
    descriptor.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
    this->declare_parameter("port", NULL, descriptor);
    this->get_parameter("port", this->port_);

    /* HACK: Unfortunately, this is a terribly messy way of dividing the channels into EEG and EMG channels.
       Instead of trying to clean it up, we should ask for the manufacturer to provide more detailed
       information about individual channels in the measurement start packet. */

    descriptor.description = "EEG channel count for primary amplifier";
    this->declare_parameter("eeg_channels_primary_amplifier", NULL, descriptor);
    this->get_parameter("eeg_channels_primary_amplifier", this->eeg_channels_primary_amplifier_);

    descriptor.description = "EMG channel count for primary amplifier";
    this->declare_parameter("emg_channels_primary_amplifier", NULL, descriptor);
    this->get_parameter("emg_channels_primary_amplifier", this->emg_channels_primary_amplifier_);

    descriptor.description = "EEG channel count for secondary amplifier";
    this->declare_parameter("eeg_channels_secondary_amplifier", NULL, descriptor);
    this->get_parameter("eeg_channels_secondary_amplifier", this->eeg_channels_secondary_amplifier_);

    descriptor.description = "EMG channel count for secondary amplifier";
    this->declare_parameter("emg_channels_secondary_amplifier", NULL, descriptor);
    this->get_parameter("emg_channels_secondary_amplifier", this->emg_channels_secondary_amplifier_);

    this->set_channel_types();

    this->init_socket();

    this->first_sample_of_experiment_ = false;
  }

  void set_channel_types() {
    uint8_t channels_primary = this->eeg_channels_primary_amplifier_ + this->emg_channels_primary_amplifier_;
    uint8_t channels_secondary = this->eeg_channels_secondary_amplifier_ + this->emg_channels_secondary_amplifier_;

    uint8_t channels_total = channels_primary + channels_secondary;

    ChannelType type;
    for (uint8_t i = 1; i <= channels_total; i++) {
      if (i > channels_total - this->emg_channels_secondary_amplifier_) {
        type = this->ChannelType::EMG;
      } else if (i > this->eeg_channels_primary_amplifier_ && i <= channels_primary) {
        type = this->ChannelType::EMG;
      } else {
        type = this->ChannelType::EEG;
      }
      this->channel_types[i - 1] = type;
    }
  }

  void spin() {
    RCLCPP_INFO(this->get_logger(), "Waiting for measurement start packet.");

    while (rclcpp::ok()) {
      if (this->read_eeg_data_from_socket()) {
        this->handle_eeg_data_packet();
      }
      rclcpp::spin_some(this->get_node_base_interface());
    }
  }

  void init_socket() {

    // Init socket variable
    this->socket_length = sizeof(this->socket_other);

    // Init socket
    this->socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (this->socket_ == -1) {
      EegBridge::err("socket");
    }

    memset((char *) &(this->socket_own), 0, sizeof(this->socket_own));
    socket_own.sin_family = AF_INET;
    socket_own.sin_port = htons(this->port_);
    socket_own.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(this->socket_, (struct sockaddr *) &(this->socket_own), sizeof(this->socket_own)) == -1) {
      EegBridge::err("bind");
    }

    struct timeval read_timeout;
    read_timeout.tv_sec = 1;
    read_timeout.tv_usec = 0;
    setsockopt(this->socket_, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
  }

  void err(const char *message) {

    RCLCPP_INFO(this->get_logger(), "Error.");
    perror(message);
    exit(1);
  }

  bool read_eeg_data_from_socket() {
    auto success = recvfrom(this->socket_, this->buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &(this->socket_other),
                            &(this->socket_length));

    if (success == -1) {
      RCLCPP_WARN(this->get_logger(), "No data received, reason: %s", strerror(errno));

      auto stream_msg = std_msgs::msg::Bool();
      stream_msg.data = false;
      this->publisher_streaming_->publish(stream_msg);

      return false;
    }
    return true;
  }

  double_t read_time_from_buffer(uint8_t index) {
    uint64_t time_us = (uint64_t) buffer[index] << 56 |
                       (uint64_t) buffer[index + 1] << 48 |
                       (uint64_t) buffer[index + 2] << 40 |
                       (uint64_t) buffer[index + 3] << 32 |
                       (uint64_t) buffer[index + 4] << 24 |
                       (uint64_t) buffer[index + 5] << 16 |
                       (uint64_t) buffer[index + 6] << 8 |
                       (uint64_t) buffer[index + 7];
    return (double_t)time_us / 1000000.0;
  }

  void handle_trigger_packet() {
    RCLCPP_INFO(this->get_logger(), "Trigger packet received.");

    double_t new_trigger_timestamp = read_time_from_buffer(TRIGGER_PACKET_FIRST_TIME_INDEX);

    uint8_t trigger_index = buffer[TRIGGER_PORT_INDEX] >> 4;
    RCLCPP_INFO(this->get_logger(), "Trigger coming from port: %u\n", trigger_index);

    auto trigger_msg = mtms_interfaces::msg::Trigger();
    if (trigger_index == 1) {
      this->first_trigger_timestamp_ = new_trigger_timestamp;
      trigger_msg.time = 0;
    } else {
      trigger_msg.time = new_trigger_timestamp - this->first_trigger_timestamp_;
    }

    this->latest_trigger_timestamp_ = new_trigger_timestamp;

    trigger_msg.index = trigger_index;
    this->publisher_trigger_->publish(trigger_msg);

    RCLCPP_INFO(this->get_logger(), "New trigger timestamp: %.2f\n", this->latest_trigger_timestamp_);
    this->first_sample_of_experiment_ = true;
  }

  void handle_sample_packet() {
    uint16_t bundles = this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX] << 8 |
                       this->buffer[SAMPLE_PACKET_N_BUNDLES_INDEX + 1];

    if (bundles != 1) {
      RCLCPP_WARN(this->get_logger(), "Warning: Bundle size %u not supported. Expected 1.", bundles);
    }

    if (VERBOSE) {
      RCLCPP_INFO(this->get_logger(), "Sample packet received.");
      RCLCPP_INFO(this->get_logger(), "Number of bundles in this packet: %d", bundles);
    }

    double_t time = read_time_from_buffer(SAMPLE_PACKET_FIRST_TIME_INDEX);

    if (this->send_trigger_as_channel && get_trigger_package_from_buffer() != 0) {
      //RCLCPP_INFO(this->get_logger(), "Received trigger package: %d", get_trigger_package_from_buffer());
      this->latest_trigger_timestamp_ = time;
      this->publish_trigger_from_buffer(time);
      this->first_sample_of_experiment_ = true;
    }

    if (this->latest_trigger_timestamp_ > 0 && time >= this->latest_trigger_timestamp_) {
      double_t time_diff = time - this->latest_trigger_timestamp_;

      if (VERBOSE) {
        RCLCPP_INFO(this->get_logger(), "Last trigger timestamp:  %.4f", this->latest_trigger_timestamp_);
        RCLCPP_INFO(this->get_logger(), "Sample timestamp:        %.4f", time);
        RCLCPP_INFO(this->get_logger(), "Time since last trigger: %.4f", time_diff);
      }

      this->publish_eeg_datapoint(time_diff);
      this->first_sample_of_experiment_ = false;

    } else if (time < this->latest_trigger_timestamp_) {
      RCLCPP_INFO(this->get_logger(), "Last trigger timestamp: %.4f", this->latest_trigger_timestamp_);
      RCLCPP_INFO(this->get_logger(), "Sample timestamp:       %.4f", time);
      RCLCPP_WARN(this->get_logger(),
                  "Warning: Sample packet arrived %.4f seconds before the trigger. Skipping.",
                  this->latest_trigger_timestamp_ - time);
    } else {
      RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Latest trigger timestamp %.4f, time %.4f",
                  this->latest_trigger_timestamp_, time);
    }
  }

  void handle_measurement_start_packet() {
    RCLCPP_INFO(this->get_logger(), "Measurement start packet received.");
    this->measurement_start_packet_received_ = true;

    this->sampling_frequency_ = (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX] << 24 |
                                (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 1] << 16 |
                                (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 2] << 8 |
                                (uint32_t) buffer[MEASUREMENT_START_PACKET_SAMPLING_FREQUENCY_INDEX + 3];
    RCLCPP_INFO(this->get_logger(), "Sampling frequency set to %d Hz.", this->sampling_frequency_);

    this->n_channels_ = (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX] << 8 |
                        (uint16_t) buffer[MEASUREMENT_START_PACKET_N_CHANNELS_INDEX + 1];
    RCLCPP_INFO(this->get_logger(), "Number of channels set to %d.", this->n_channels_);

    this->send_trigger_as_channel = false;
    for (uint8_t i = 0; i < this->n_channels_; i++) {
      uint16_t source_channel = buffer[MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX + 2 * i] << 8 |
                                buffer[MEASUREMENT_START_PACKET_SOURCE_CHANNELS_INDEX + 2 * i + 1];
      if (source_channel == SOURCE_CHANNEL_FOR_TRIGGER) {
        this->send_trigger_as_channel = true;
      }
    }

    if (this->send_trigger_as_channel) {
      RCLCPP_INFO(this->get_logger(), "Trigger is sent as a channel.");

      this->n_channels_excluding_trigger_ = this->n_channels_ - 1;
    } else {
      RCLCPP_INFO(this->get_logger(), "Trigger is sent as a packet.");

      this->n_channels_excluding_trigger_ = this->n_channels_;
    }
  }

  void handle_eeg_data_packet() {
    uint8_t packet_type = this->buffer[0];

    switch (packet_type) {
      case MEASUREMENT_START_PACKET_ID:
        this->handle_measurement_start_packet();
        break;
      case SAMPLE_PACKET_ID:
        if (this->measurement_start_packet_received_) {
          this->handle_sample_packet();
        }
        break;
      case TRIGGER_PACKET_ID:
        if (this->measurement_start_packet_received_) {
          this->handle_trigger_packet();
        }
        break;
      default:
        RCLCPP_WARN(this->get_logger(), "Unknown packet type.");
    }
  }

  int get_trigger_package_from_buffer() {
    auto index = FIRST_CHANNEL_INDEX + this->n_channels_excluding_trigger_ * 3;

    return (uint8_t) buffer[index] << 16 |
           (uint8_t) buffer[index + 1] << 8 |
           (uint8_t) buffer[index + 2];
  }

  void publish_trigger_from_buffer(double_t time) {
    int trigger_channel_package = get_trigger_package_from_buffer();

    auto trigger_msg = mtms_interfaces::msg::Trigger();

    if (trigger_channel_package == TRIGGER_A_IN) {
      trigger_msg.index = 1;
      first_trigger_timestamp_ = time;
      trigger_msg.time = 0.0;
    } else if (trigger_channel_package == TRIGGER_B_IN) {
      trigger_msg.index = 2;
      trigger_msg.time = time - first_trigger_timestamp_;
    }
    this->publisher_trigger_->publish(trigger_msg);
  }

  void publish_eeg_datapoint(double_t time_since_trigger) {

    auto message = mtms_interfaces::msg::EegDatapoint();
    message.time = time_since_trigger;

    int i = FIRST_CHANNEL_INDEX;
    for (int channel = 1; channel <= this->n_channels_excluding_trigger_; channel++) {

      int result = (uint8_t) buffer[i] << 16 |
                   (uint8_t) buffer[i + 1] << 8 |
                   (uint8_t) buffer[i + 2];

      if (result > SIGNED_MAX) {
        result -= UNSIGNED_MAX;
      }

      double result_uv = result;
      result_uv *= DC_MODE_SCALE;
      result_uv /= NANO_TO_MICRO_CONVERSION;

      if (channel_types[channel - 1] == ChannelType::EEG) {
        message.eeg_channels.push_back(result_uv);
      } else {
        message.emg_channels.push_back(result_uv);
      }

      i += 3;
    }

    message.first_sample_of_experiment = this->first_sample_of_experiment_;

    this->publisher_data_->publish(message);

    auto stream_msg = std_msgs::msg::Bool();
    stream_msg.data = true;
    this->publisher_streaming_->publish(stream_msg);
  }


private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<mtms_interfaces::msg::EegDatapoint>::SharedPtr publisher_data_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_streaming_;
  rclcpp::Publisher<mtms_interfaces::msg::Trigger>::SharedPtr publisher_trigger_;

  double_t first_trigger_timestamp_;
  double_t latest_trigger_timestamp_;

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

  enum ChannelType {EEG, EMG};
  ChannelType channel_types[MAX_NUMBER_OF_CHANNELS];
  bool send_trigger_as_channel;

  uint8_t eeg_channels_primary_amplifier_;
  uint8_t emg_channels_primary_amplifier_;
  uint8_t eeg_channels_secondary_amplifier_;
  uint8_t emg_channels_secondary_amplifier_;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  node->spin();
  rclcpp::shutdown();
  return 0;
}
