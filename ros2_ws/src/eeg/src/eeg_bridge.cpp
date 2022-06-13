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

#define BUFFER_LENGTH 250
#define PORT 50000
#define SIGNED_MAX pow(2,23)
#define UNSIGNED_MAX pow(2,24)
#define DEFAULT_FREQUENCY_VALUE 500.0 // Hz
#define DC_MODE_SCALE 100
#define AC_MODE_SCALE 20
#define NANO_TO_MICRO_CONVERSION 1000
#define FIRST_CHANNEL_INDEX 28
#define NUMBER_OF_CHANNELS 62

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "mtms_interfaces/msg/eeg_datapoint.hpp"

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
      EegBridge::init_socket();

      this->declare_parameter<float>("sampling_frequency", DEFAULT_FREQUENCY_VALUE);
      this->get_parameter("sampling_frequency", sampling_frequency_);

      auto sampling_interval_int = int(round(1000 / sampling_frequency_));
      auto sampling_interval_ms = std::chrono::milliseconds(sampling_interval_int);

      timer_ = this->create_wall_timer(sampling_interval_ms, std::bind(&EegBridge::timer_callback, this));
    }

    void init_socket() {

      // Init socket variables
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

      if (bind(this->socket_, (struct sockaddr*)&(this->socket_own), sizeof(this->socket_own)) == -1) {
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

    void timer_callback() {

      auto val = recvfrom(this->socket_, this->buffer, BUFFER_LENGTH, 0,(struct sockaddr*) &(this->socket_other), &(this->socket_length));

      if (val == -1) {

        RCLCPP_INFO(this->get_logger(), "No data received");

        auto stream_msg = std_msgs::msg::Bool();
        stream_msg.data = false;
        this->publisher_streaming_->publish(stream_msg);
      }

      else if (this->buffer[0] < 4) {

        auto message = mtms_interfaces::msg::EegDatapoint();

        int i = FIRST_CHANNEL_INDEX;
        for (int channel = 1; channel <= NUMBER_OF_CHANNELS; channel++) {

          int result = buffer[i] << 16 | buffer[i+1] << 8 | buffer[i+2];

          if (result > SIGNED_MAX) {
            result -= UNSIGNED_MAX;
          }

          double result_uv = result;
          result_uv *= DC_MODE_SCALE;
          result_uv /= NANO_TO_MICRO_CONVERSION;

          message.channel_datapoint.push_back(result_uv);
          RCLCPP_INFO(this->get_logger(), "Channel: %d, Result: %f", channel, result_uv);

          i+=3;
        }

        this->publisher_data_->publish(message);
        
        auto stream_msg = std_msgs::msg::Bool();
        stream_msg.data = true;
        this->publisher_streaming_->publish(stream_msg);
      }

      else {
        close(this->socket_);
        RCLCPP_INFO(this->get_logger(), "Closing.");
      }
    }

  private:
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<mtms_interfaces::msg::EegDatapoint>::SharedPtr publisher_data_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_streaming_;
    float sampling_frequency_;
    int socket_;
    sockaddr_in socket_own;
    sockaddr_in socket_other;
    socklen_t socket_length;
    char buffer[BUFFER_LENGTH];
};

int main(int argc, char * argv[]) {

  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<EegBridge>());
  rclcpp::shutdown();
  return 0;
}
