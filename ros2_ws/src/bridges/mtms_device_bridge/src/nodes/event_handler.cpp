#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/msg/charge.hpp"
#include "event_interfaces/msg/discharge.hpp"
#include "event_interfaces/msg/pulse.hpp"
#include "event_interfaces/msg/trigger_out.hpp"
#include "event_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 channel_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetPulseFIFO;
const NiFpga_mTMS_HostToTargetFifoU8 charge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO;
const NiFpga_mTMS_HostToTargetFifoU8 discharge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO;
const NiFpga_mTMS_HostToTargetFifoU8 trigger_out_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetTriggerOutFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class EventHandler : public rclcpp::Node {
public:
  EventHandler();

private:
  void pulse_callback(const std::shared_ptr<event_interfaces::msg::Pulse> pulse);
  void charge_callback(const std::shared_ptr<event_interfaces::msg::Charge> charge);
  void discharge_callback(const std::shared_ptr<event_interfaces::msg::Discharge> discharge);
  void trigger_out_callback(const std::shared_ptr<event_interfaces::msg::TriggerOut> trigger_out);

  rclcpp::Subscription<event_interfaces::msg::Pulse>::SharedPtr send_pulse_subscriber_;
  rclcpp::Subscription<event_interfaces::msg::Charge>::SharedPtr send_charge_subscriber_;
  rclcpp::Subscription<event_interfaces::msg::Discharge>::SharedPtr send_discharge_subscriber_;
  rclcpp::Subscription<event_interfaces::msg::TriggerOut>::SharedPtr send_trigger_out_subscriber_;

  SerializedMessage serialized_message;
  bool safe_mode;
};

EventHandler::EventHandler() : Node("event_handler") {
  this->declare_parameter<bool>("safe-mode", false);
  this->get_parameter("safe-mode", safe_mode);

  serialized_message = SerializedMessage();

  send_pulse_subscriber_ = this->create_subscription<event_interfaces::msg::Pulse>(
      "/event/send/pulse", 10, std::bind(&EventHandler::pulse_callback, this, std::placeholders::_1));

  send_charge_subscriber_ = this->create_subscription<event_interfaces::msg::Charge>(
      "/event/send/charge", 10, std::bind(&EventHandler::charge_callback, this, std::placeholders::_1));

  send_discharge_subscriber_ = this->create_subscription<event_interfaces::msg::Discharge>(
      "/event/send/discharge", 10, std::bind(&EventHandler::discharge_callback, this, std::placeholders::_1));

  send_trigger_out_subscriber_ = this->create_subscription<event_interfaces::msg::TriggerOut>(
      "/event/send/trigger_out", 10, std::bind(&EventHandler::trigger_out_callback, this, std::placeholders::_1));
}

void EventHandler::pulse_callback(const std::shared_ptr<event_interfaces::msg::Pulse> pulse) {
  /* Unpack pulse message. */
  uint8_t channel = pulse->channel;

  event_interfaces::msg::EventInfo event_info = pulse->event_info;
  uint16_t id = event_info.id;
  uint8_t execution_condition = event_info.execution_condition.value;
  double_t execution_time = event_info.execution_time;

  /* Log pulse message. */
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Executing pulse on channel %d (id: %d, execution_condition: %d, execution_time: %.4f s)",
              pulse->channel,
              pulse->event_info.id,
              pulse->event_info.execution_condition.value,
              pulse->event_info.execution_time);

  /* Check if FPGA is OK. */
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Tried to execute pulse (id: %d) but FPGA is not in OK state", id);
    return;
  }

  /* Check that execution time is non-negative. */

  /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting pulse (id: %d)", id);
    return;
  }

  /* Serialize event info. */

  uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);

  /* XXX: Note that LabVIEW starts indexing from 1. Hence, do the conversion from 0-based
       indexing here. It would rather be the responsibility of FPGA to do the conversion;
       move the logic there eventually. */
  serialized_message.init(channel + 1);
  serialized_message.add_uint16(id);
  serialized_message.add_byte(execution_condition);
  serialized_message.add_uint64(execution_time_ticks);

  /* Serialize pulse parameters. */
  uint8_t num_of_waveform_pieces = (uint8_t) pulse->waveform.pieces.size();
  serialized_message.add_byte(num_of_waveform_pieces);

  for (uint8_t i = 0; i < num_of_waveform_pieces; i++) {
    event_interfaces::msg::WaveformPiece piece = pulse->waveform.pieces[i];

    serialized_message.add_byte(piece.waveform_phase.value);
    serialized_message.add_uint16(piece.duration_in_ticks);
  }

  serialized_message.finalize();

  if (this->safe_mode) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Safe mode is enabled, aborting pulse (id: %d)", id);
    return;
  }

  NiFpga_MergeStatus(&status,
                      NiFpga_StartFifo(session,
                                      channel_pulse_fifo));
  NiFpga_MergeStatus(&status,
                      NiFpga_WriteFifoU8(session,
                                        channel_pulse_fifo,
                                        serialized_message.serialized_message.data(),
                                        serialized_message.get_length(),
                                        NiFpga_InfiniteTimeout,
                                        NULL));
}

void EventHandler::charge_callback(const std::shared_ptr<event_interfaces::msg::Charge> charge) {
  /* Unpack charge message. */
  uint8_t channel = charge->channel;

  event_interfaces::msg::EventInfo event_info = charge->event_info;
  uint16_t id = event_info.id;
  uint8_t execution_condition = event_info.execution_condition.value;
  double_t execution_time = event_info.execution_time;

  /* Log charge message. */
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Charging channel %d to %d V (id: %d, execution_condition: %d, execution_time: %.4f s)",
              charge->channel,
              charge->target_voltage,
              charge->event_info.id,
              charge->event_info.execution_condition.value,
              charge->event_info.execution_time);

  /* Check if FPGA is OK. */
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "FPGA is not in OK state, aborting charge (id: %d)", id);
    return;
  }

  /* Check that execution time is non-negative. */

  /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting charge (id: %d)", id);
    return;
  }

  /* Serialize event info. */

  uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);

  serialized_message.init();
  serialized_message.add_uint16(id);
  serialized_message.add_byte(execution_condition);
  serialized_message.add_uint64(execution_time_ticks);

  /* Serialize charge parameters. */

  /* XXX: Charge requires the channel here instead of the beginning of the message. */

  /* XXX: Note that LabVIEW starts indexing from 1. Hence, do the conversion from 0-based
       indexing here. It would rather be the responsibility of FPGA to do the conversion;
       move the logic there eventually. */
  serialized_message.add_byte(channel + 1);

  uint16_t target_voltage = charge->target_voltage;
  serialized_message.add_uint16(target_voltage);

  serialized_message.finalize();

  if (this->safe_mode) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Safe mode is enabled, aborting charge (id: %d)", id);
    return;
  }
  NiFpga_MergeStatus(&status,
                      NiFpga_StartFifo(session,
                                      charge_fifo));

  NiFpga_MergeStatus(&status,
                      NiFpga_WriteFifoU8(session,
                                        charge_fifo,
                                        serialized_message.serialized_message.data(),
                                        serialized_message.get_length(),
                                        NiFpga_InfiniteTimeout,
                                        NULL));
}

void EventHandler::discharge_callback(const std::shared_ptr<event_interfaces::msg::Discharge> discharge) {
  /* Unpack discharge message. */
  uint8_t channel = discharge->channel;
  event_interfaces::msg::EventInfo event_info = discharge->event_info;

  uint16_t id = event_info.id;
  uint8_t execution_condition = event_info.execution_condition.value;
  double_t execution_time = event_info.execution_time;

  /* Log discharge message. */
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Discharging channel %d to %d V (id: %d, execution_condition: %d, execution_time: %.4f s)",
              discharge->channel,
              discharge->target_voltage,
              discharge->event_info.id,
              discharge->event_info.execution_condition.value,
              discharge->event_info.execution_time);

  /* Check if FPGA is OK. */
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Tried to discharge (id: %d) but FPGA is not in OK state", id);
    return;
  }

  /* Check that execution time is non-negative. */

  /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting discharge (id: %d)", id);
    return;
  }

  /* Serialize event info. */

  uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);

  /* XXX: Note that LabVIEW starts indexing from 1. Hence, do the conversion from 0-based
       indexing here. It would rather be the responsibility of FPGA to do the conversion;
       move the logic there eventually. */
  serialized_message.init(channel + 1);
  serialized_message.add_uint16(id);
  serialized_message.add_byte(execution_condition);
  serialized_message.add_uint64(execution_time_ticks);

  /* Serialize discharge parameter. */

  uint16_t target_voltage = discharge->target_voltage;
  serialized_message.add_uint16(target_voltage);

  serialized_message.finalize();

  if (this->safe_mode) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Safe mode is enabled, aborting discharge (id: %d)", id);
    return;
  }
  NiFpga_MergeStatus(&status,
                      NiFpga_StartFifo(session,
                                       discharge_fifo));

  NiFpga_MergeStatus(&status,
                      NiFpga_WriteFifoU8(session,
                                        discharge_fifo,
                                        serialized_message.serialized_message.data(),
                                        serialized_message.get_length(),
                                        NiFpga_InfiniteTimeout,
                                        NULL));
}

void EventHandler::trigger_out_callback(const std::shared_ptr<event_interfaces::msg::TriggerOut> trigger_out) {
  /* Unpack trigger out message. */
  uint8_t port = trigger_out->port;

  event_interfaces::msg::EventInfo event_info = trigger_out->event_info;
  uint16_t id = event_info.id;
  uint8_t execution_condition = event_info.execution_condition.value;
  double_t execution_time = event_info.execution_time;

  /* Log trigger out message. */
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Sending trigger out to port %d (id: %d, execution_condition: %d, execution_time: %.4f s)",
              port,
              trigger_out->event_info.id,
              trigger_out->event_info.execution_condition.value,
              trigger_out->event_info.execution_time);

  /* Check if FPGA is OK. */
  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "FPGA is not in OK state, aborting sending trigger out event (id: %d)", id);
    return;
  }

  /* Check that execution time is non-negative. */

  /* TODO: To properly propagate the error, sending pulses, charges, discharges, and trigger outs should be ROS services instead of messages. */
  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting trigger out (id: %d)", id);
    return;
  }

  /* Serialize event info. */

  uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);

  serialized_message.init(port);
  serialized_message.add_uint16(id);
  serialized_message.add_byte(execution_condition);
  serialized_message.add_uint64(execution_time_ticks);

  /* Serialize trigger out parameters. */

  uint32_t duration_us = trigger_out->duration_us;
  uint32_t duration_ticks = duration_us * (CLOCK_FREQUENCY_HZ / 1e6);

  serialized_message.add_uint32(duration_ticks);

  serialized_message.finalize();

  /* For consistency with channel indexing, start trigger out indexing from 1. */
  NiFpga_MergeStatus(&status,
                      NiFpga_StartFifo(session,
                                       trigger_out_fifo));

  NiFpga_MergeStatus(&status,
                      NiFpga_WriteFifoU8(session,
                                        trigger_out_fifo,
                                        serialized_message.serialized_message.data(),
                                        serialized_message.get_length(),
                                        NiFpga_InfiniteTimeout,
                                        NULL));
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<EventHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Event handler ready.");

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
