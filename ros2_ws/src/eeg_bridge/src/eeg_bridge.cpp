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

#define BUFFER_LENGTH 250
#define PORT 50000

#define SIGNED_MAX pow(2,23)
#define UNSIGNED_MAX pow(2,24)
#define DEFAULT_FREQUENCY_VALUE 500.0 // Hz
#define DC_MODE_SCALE 100
#define AC_MODE_SCALE 20
#define NANO_TO_MICRO_CONVERSION 1000

#define FIRST_CHANNEL_INDEX 28
#define SAMPLE_PACKET_FIRST_TIME_INDEX 20
#define TRIGGER_PACKET_FIRST_TIME_INDEX 8
#define TRIGGER_PORT_INDEX 24

#define NUMBER_OF_CHANNELS 62

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
        RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT,
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

    this->init_socket();

    this->declare_parameter<float>("sampling_frequency", DEFAULT_FREQUENCY_VALUE);
    this->get_parameter("sampling_frequency", sampling_frequency_);

    auto sampling_interval_int = int(round(1000 / sampling_frequency_));
    auto sampling_interval_ms = std::chrono::milliseconds(sampling_interval_int);

    this->first_sample_of_experiment_ = false;

    timer_ = this->create_wall_timer(sampling_interval_ms, std::bind(&EegBridge::timer_callback, this));
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
    socket_own.sin_port = htons(PORT);
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

  uint64_t read_time_from_buffer(uint8_t index) {
    return (uint64_t) buffer[index] << 56 |
           (uint64_t) buffer[index + 1] << 48 |
           (uint64_t) buffer[index + 2] << 40 |
           (uint64_t) buffer[index + 3] << 32 |
           (uint64_t) buffer[index + 4] << 24 |
           (uint64_t) buffer[index + 5] << 16 |
           (uint64_t) buffer[index + 6] << 8 |
           (uint64_t) buffer[index + 7];
  }

  void trigger_packet_received() {
    RCLCPP_INFO(this->get_logger(), "Trigger packet received.");

    uint64_t new_trigger_timestamp = read_time_from_buffer(TRIGGER_PACKET_FIRST_TIME_INDEX);

    uint8_t trigger_index = buffer[TRIGGER_PORT_INDEX] >> 4;
    RCLCPP_INFO(this->get_logger(), "Trigger coming from port: %u\n", trigger_index);

    auto trigger_msg = mtms_interfaces::msg::Trigger();
    if (trigger_index == 1) {
      this->first_trigger_timestamp_ = new_trigger_timestamp;
      trigger_msg.time_us = 0;
    } else {
      trigger_msg.time_us = new_trigger_timestamp - this->first_trigger_timestamp_;
    }

    this->latest_trigger_timestamp_ = new_trigger_timestamp;

    trigger_msg.index = trigger_index;
    this->publisher_trigger_->publish(trigger_msg);

    RCLCPP_INFO(this->get_logger(), "New trigger timestamp: %lu\n", this->latest_trigger_timestamp_);
    this->first_sample_of_experiment_ = true;
  }

  void sample_packet_received() {
    uint16_t bundles = this->buffer[10] << 8 | buffer[11];
    uint16_t num_channels = this->buffer[8] << 8 | buffer[9];

    if (bundles != 1) {
      RCLCPP_WARN(this->get_logger(), "Warning: Bundle size %u not supported. Expected 1.", bundles);
    }

    if (VERBOSE) {
      RCLCPP_INFO(this->get_logger(), "Sample packet received.");
      RCLCPP_INFO(this->get_logger(), "Number of bundles in this packet: %d", bundles);
    }

    uint64_t time = read_time_from_buffer(SAMPLE_PACKET_FIRST_TIME_INDEX);

    /* If the actual number of sent channels is larger than NUMBER_OF_CHANNELS, it means that we are using a
     * protocol where the triggers are sent as a part of EEG data instead of as separate packet */
    auto trigger_channel = num_channels > NUMBER_OF_CHANNELS;

    if (trigger_channel && get_trigger_package_from_buffer() != 0) {
      RCLCPP_INFO(this->get_logger(), "Received trigger package: %d", get_trigger_package_from_buffer());
      this->latest_trigger_timestamp_ = time;
      this->publish_trigger_from_buffer(time);
      this->first_sample_of_experiment_ = true;
    }

    if (this->latest_trigger_timestamp_ > 0 && time >= this->latest_trigger_timestamp_) {
      double_t time_diff_ms = (time - this->latest_trigger_timestamp_) / 1000;

      if (VERBOSE) {
        RCLCPP_INFO(this->get_logger(), "Last trigger timestamp: %lu", this->latest_trigger_timestamp_);
        RCLCPP_INFO(this->get_logger(), "Sample timestamp:       %lu", time);
        RCLCPP_INFO(this->get_logger(), "Time since last trigger (ms): %f\n", time_diff_ms);
      }

      this->publish_eeg_datapoint(time_diff_ms);
      this->first_sample_of_experiment_ = false;

    } else if (time < this->latest_trigger_timestamp_) {
      RCLCPP_INFO(this->get_logger(), "Last trigger timestamp: %lu", this->latest_trigger_timestamp_);
      RCLCPP_INFO(this->get_logger(), "Sample timestamp:       %lu", time);
      RCLCPP_WARN(this->get_logger(),
                  "Warning: Sample packet arrived %ld milliseconds before the trigger. Skipping.\n",
                  (this->latest_trigger_timestamp_ - time) / 1000);
    } else {
      RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "latest trigger timestamp %lu, time, %lu",
                  this->latest_trigger_timestamp_, time);
    }
  }

  void timer_callback() {

    auto success = recvfrom(this->socket_, this->buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &(this->socket_other),
                            &(this->socket_length));

    if (success == -1) {
      RCLCPP_WARN(this->get_logger(), "No data received, reason: %s", strerror(errno));

      auto stream_msg = std_msgs::msg::Bool();
      stream_msg.data = false;
      this->publisher_streaming_->publish(stream_msg);
      return;
    }

    auto packet_type = this->buffer[0];

    switch (packet_type) {
      case MEASUREMENT_START_PACKET_ID:
        RCLCPP_WARN(rclcpp::get_logger("eeg_bridge"), "Received measurement start packet, not implemented");
        break;
      case SAMPLE_PACKET_ID:
        this->sample_packet_received();
        break;
      case TRIGGER_PACKET_ID:
        this->trigger_packet_received();
        break;
      default:
        close(this->socket_);
        RCLCPP_WARN(this->get_logger(), "Unknown packet type.");
    }
  }

  int get_trigger_package_from_buffer() {
    auto index = FIRST_CHANNEL_INDEX + NUMBER_OF_CHANNELS * 3;

    return (uint8_t) buffer[index] << 16 |
           (uint8_t) buffer[index + 1] << 8 |
           (uint8_t) buffer[index + 2];
  }

  void publish_trigger_from_buffer(uint64_t time) {
    int trigger_channel_package = get_trigger_package_from_buffer();

    auto trigger_msg = mtms_interfaces::msg::Trigger();

    if (trigger_channel_package == TRIGGER_A_IN) {
      trigger_msg.index = 1;
      first_trigger_timestamp_ = time;
      trigger_msg.time_us = 0;
    } else if (trigger_channel_package == TRIGGER_B_IN) {
      trigger_msg.index = 2;
      trigger_msg.time_us = time - first_trigger_timestamp_;
    }

    this->publisher_trigger_->publish(trigger_msg);

  }

  void publish_eeg_datapoint(double time_since_trigger) {

    auto message = mtms_interfaces::msg::EegDatapoint();
    message.time = time_since_trigger;

    int i = FIRST_CHANNEL_INDEX;
    for (int channel = 1; channel <= NUMBER_OF_CHANNELS; channel++) {

      int result = (uint8_t) buffer[i] << 16 |
                   (uint8_t) buffer[i + 1] << 8 |
                   (uint8_t) buffer[i + 2];

      if (result > SIGNED_MAX) {
        result -= UNSIGNED_MAX;
      }

      double result_uv = result;
      result_uv *= DC_MODE_SCALE;
      result_uv /= NANO_TO_MICRO_CONVERSION;

      message.channel_datapoint.push_back(result_uv);

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

  uint64_t first_trigger_timestamp_;
  uint64_t latest_trigger_timestamp_;

  float sampling_frequency_;
  int socket_;
  sockaddr_in socket_own;
  sockaddr_in socket_other;
  socklen_t socket_length;
  uint8_t buffer[BUFFER_LENGTH];
  bool first_sample_of_experiment_;
};


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);

  //RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "memory optimization: %d", MEMORY_OPTIMIZATION);

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EegBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("eeg_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
