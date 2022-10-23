#include <chrono>
#include <map>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/feedback.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

#define CHANNEL_COUNT 5

using namespace std::chrono_literals;

NiFpga_mTMS_TargetToHostFifoU8 pulse_feedback_fifo = NiFpga_mTMS_TargetToHostFifoU8_TargettoHostStimulationpulsefeedbackFIFO;
NiFpga_mTMS_TargetToHostFifoU8 charge_feedback_fifo = NiFpga_mTMS_TargetToHostFifoU8_TargettoHostChargefeedbackFIFO;
NiFpga_mTMS_TargetToHostFifoU8 discharge_feedback_fifo = NiFpga_mTMS_TargetToHostFifoU8_TargettoHostDischargefeedbackFIFO;
NiFpga_mTMS_TargetToHostFifoU8 signal_out_feedback_fifo = NiFpga_mTMS_TargetToHostFifoU8_TargettoHostSignalOutfeedbackFIFO;

class FeedbackMonitorBridge : public rclcpp::Node {
public:
  FeedbackMonitorBridge()
      : Node("feedback_monitor_bridge") {
    pulse_feedback_publisher_ = this->create_publisher<fpga_interfaces::msg::Feedback>(
        "/fpga/pulse_feedback", 10);
    charge_feedback_publisher_ = this->create_publisher<fpga_interfaces::msg::Feedback>(
        "/fpga/charge_feedback", 10);
    discharge_feedback_publisher_ = this->create_publisher<fpga_interfaces::msg::Feedback>(
        "/fpga/discharge_feedback", 10);
    signal_out_feedback_publisher_ = this->create_publisher<fpga_interfaces::msg::Feedback>(
        "/fpga/signal_out_feedback", 10);

    timer_ = this->create_wall_timer(20ms, std::bind(&FeedbackMonitorBridge::update_feedback_topics, this));
  }

private:
  /* HACK: There are essentially two ways to pass event feedback from FPGA, one is handled by read_fifo_and_publish
           function below (used by pulse, discharge, and signal out events), and the other is handled by
           read_non_multiplexed_fifo_and_publish.

           They should be unified on the FPGA. The ideal way to do that might be drop the channel information and
           send feedback for all event types in the way it is done with charging at the moment. However, to
           confirm that, more SW components needs to be written that use the event feedback data. (Note that
           channel is already disregarded so that we are able to use a single ROS message type for all event
           feedback.) */

  void read_fifo_and_publish(std::string event_type,
                             NiFpga_mTMS_TargetToHostFifoU8 fifo,
                             std::map<uint8_t, std::vector<uint8_t>> map,
                             rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr* publisher) {
    size_t elements_remaining = 0;
    NiFpga_Status read_status;
    std::vector<uint8_t> data(2);

    // Start by checking if there is enough data in the FIFO.
    read_status = NiFpga_ReadFifoU8(session, fifo, data.data(), 0, NiFpga_InfiniteTimeout,
                                    &elements_remaining);
    if (NiFpga_IsError(read_status)) {
      RCLCPP_ERROR(rclcpp::get_logger("feedback_monitor_bridge"), "Error reading data from fifo %d", read_status);
      return;
    }

    while (elements_remaining > 1) {
      read_status = NiFpga_ReadFifoU8(session, fifo, data.data(), 2, NiFpga_InfiniteTimeout,
                                      &elements_remaining);

      if (NiFpga_IsError(read_status)) {
        RCLCPP_ERROR(rclcpp::get_logger("feedback_monitor_bridge"), "Error reading data from fifo %d",
                     read_status);
        return;
      }

      uint8_t channel = data[0];

      // If a key does not exist in map yet, add the key.
      if (map.find(channel) == map.end()) {
        std::vector<uint8_t> message_data;
        map.insert(std::make_pair(channel, message_data));
      }

      map[channel].push_back(data[1]);

      // A whole message has been read.
      if (map[channel].size() > 2) {
        uint16_t id = map[channel][0] * 256 + map[channel][1];
        uint8_t status_code = map[channel][2];
        publish_feedback(event_type, id, status_code, publisher);
      }
    }
  }

  void read_non_multiplexed_fifo_and_publish(std::string event_type,
                                             NiFpga_mTMS_TargetToHostFifoU8 fifo,
                                             rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr* publisher) {
    size_t elements_remaining = 0;
    NiFpga_Status read_status;
    std::vector<uint8_t> data(3);

    // Start by checking if there is enough data in the FIFO.
    read_status = NiFpga_ReadFifoU8(session, fifo, data.data(), 0, NiFpga_InfiniteTimeout,
                                    &elements_remaining);
    if (NiFpga_IsError(read_status)) {
      RCLCPP_ERROR(rclcpp::get_logger("feedback_monitor_bridge"), "Error reading data from fifo %d", read_status);
      return;
    }

    while (elements_remaining > 2) {
      read_status = NiFpga_ReadFifoU8(session, fifo, data.data(), 3, NiFpga_InfiniteTimeout,
                                      &elements_remaining);

      if (NiFpga_IsError(read_status)) {
        RCLCPP_ERROR(rclcpp::get_logger("feedback_monitor_bridge"), "Error reading data from fifo %d",
                     read_status);
        return;
      }

      uint16_t event_id = data[0] * 256 + data[1];
      uint8_t status_code = data[2];
      publish_feedback(event_type, event_id, status_code, publisher);
    }
  }

  void publish_feedback(std::string event_type,
                        uint16_t id,
                        uint8_t status_code,
                        rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr* publisher) {

    fpga_interfaces::msg::Feedback feedback;
    feedback.id = id;
    feedback.status_code = status_code;

    RCLCPP_INFO(rclcpp::get_logger("feedback_monitor_bridge"),
                "Publishing data to %s feedback: {event_id: %d, status: %d}",
                event_type.data(),
                id,
                status_code);

    (*publisher)->publish(feedback);
  }

  void update_feedback_topics() {
    read_fifo_and_publish("Pulse", pulse_feedback_fifo, pulse_data, &pulse_feedback_publisher_);
    read_fifo_and_publish("Discharge", discharge_feedback_fifo, discharge_data, &discharge_feedback_publisher_);
    read_fifo_and_publish("Signal out", signal_out_feedback_fifo, signal_out_data, &signal_out_feedback_publisher_);
    read_non_multiplexed_fifo_and_publish("Charge", charge_feedback_fifo, &charge_feedback_publisher_);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr pulse_feedback_publisher_;
  rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr signal_out_feedback_publisher_;
  rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr charge_feedback_publisher_;
  rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr discharge_feedback_publisher_;

  //channel index, data
  std::map<uint8_t, std::vector<uint8_t>> pulse_data = {};
  std::map<uint8_t, std::vector<uint8_t>> discharge_data = {};
  std::map<uint8_t, std::vector<uint8_t>> signal_out_data = {};
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("feedback_monitor_bridge"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_NORMAL_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<FeedbackMonitorBridge>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("feedback_monitor_bridge"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("feedback_monitor_bridge"), "Feedback monitor bridge ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
