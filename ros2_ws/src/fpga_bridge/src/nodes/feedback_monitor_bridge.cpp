#include <chrono>
#include <map>

#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/msg/feedback.hpp"

#include "fpga.h"
#include "NiFpga_mTMS.h"

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

    void read_fifo_and_publish(NiFpga_mTMS_TargetToHostFifoU8 fifo,
                               std::map<uint8_t, std::vector<uint8_t>> map,
                               std::string event_type) {
        size_t elements_remaining = 0;
        NiFpga_Status read_status;
        std::vector<uint8_t> data(2);

        //start by checking if there is enough data in the fifo
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

            //if key does not exist in map yet, add the key and
            if (map.find(channel) == map.end()) {
                std::vector<uint8_t> message_data;
                map.insert(std::make_pair(channel, message_data));
            }

            map[channel].push_back(data[1]);

            //a whole message has been read
            if (map[channel].size() > 2) {
                auto event_id = map[channel][0] * 256 + map[channel][1];
                uint8_t status_code = map[channel][2];
                publish_feedback(event_type, channel, event_id, status_code);
            }

        }

    }

    void publish_feedback(std::string event_type, uint8_t channel, int event_id, uint8_t status_code) {
        fpga_interfaces::msg::Feedback feedback;
        feedback.channel = channel;
        feedback.event_id = event_id;
        feedback.status = status_code;

        RCLCPP_DEBUG(rclcpp::get_logger("feedback_monitor_bridge"),
                    "Publishing data to %s feedback: {channel: %d, event_id: %d, status: %d}", event_type.data(),
                    channel, event_id, status_code);

        if (event_type == "Pulse") {
            pulse_feedback_publisher_->publish(feedback);
        } else if (event_type == "Charge") {
            charge_feedback_publisher_->publish(feedback);
        } else if (event_type == "Discharge") {
            discharge_feedback_publisher_->publish(feedback);
        } else {
            signal_out_feedback_publisher_->publish(feedback);
        }

    }

    void update_feedback_topics() {
        read_fifo_and_publish(pulse_feedback_fifo, pulse_data, "Pulse");
        read_fifo_and_publish(charge_feedback_fifo, charge_data, "Charge");
        read_fifo_and_publish(discharge_feedback_fifo, discharge_data, "Discharge");
        read_fifo_and_publish(signal_out_feedback_fifo, signal_out_data, "Signal out");
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr pulse_feedback_publisher_;
    rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr signal_out_feedback_publisher_;
    rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr charge_feedback_publisher_;
    rclcpp::Publisher<fpga_interfaces::msg::Feedback>::SharedPtr discharge_feedback_publisher_;

    //channel index, data
    std::map<uint8_t, std::vector<uint8_t>> pulse_data = {};
    std::map<uint8_t, std::vector<uint8_t>> charge_data = {};
    std::map<uint8_t, std::vector<uint8_t>> discharge_data = {};
    std::map<uint8_t, std::vector<uint8_t>> signal_out_data = {};
};

int main(int argc, char **argv) {
    if (!init_fpga()) {
        return 1;
    }

    rclcpp::init(argc, argv);

    RCLCPP_INFO(rclcpp::get_logger("feedback_monitor_bridge"), "Feedback monitor bridge ready.");

    rclcpp::spin(std::make_shared<FeedbackMonitorBridge>());
    rclcpp::shutdown();

    close_fpga();
}
