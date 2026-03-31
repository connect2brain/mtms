#include <chrono>
#include <limits>

#include "rclcpp/rclcpp.hpp"

#include "mtms_device_interfaces/srv/request_events.hpp"

#include "mtms_event_interfaces/msg/pulse.hpp"
#include "mtms_event_interfaces/msg/charge.hpp"
#include "mtms_event_interfaces/msg/discharge.hpp"
#include "mtms_event_interfaces/msg/trigger_out.hpp"
#include "mtms_event_interfaces/msg/execution_condition.hpp"

#include "realtime_utils/utils.h"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"

const NiFpga_mTMS_HostToTargetFifoU8 channel_pulse_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetPulseFIFO;
const NiFpga_mTMS_HostToTargetFifoU8 charge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO;
const NiFpga_mTMS_HostToTargetFifoU8 discharge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO;
const NiFpga_mTMS_HostToTargetFifoU8 trigger_out_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetTriggerOutFIFO;
const NiFpga_mTMS_ControlBool event_aggregation_lock = NiFpga_mTMS_ControlBool_Eventaggregationlock;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

const NiFpga_mTMS_IndicatorU64 time_indicator = NiFpga_mTMS_IndicatorU64_Time;

/* Set minimum margin to 1 ms. Sending a paired pulse with two triggers to the mTMS device takes 0.12-0.15 ms on the average,
   so there is plenty of extra time to process the events.*/
const double MINIMUM_MARGIN_S = 0.001;

class EventHandler : public rclcpp::Node {
public:
  EventHandler();

private:
  void handle_request_events(const std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Request> request,
                             std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Response> response);

  void process_pulse(const mtms_event_interfaces::msg::Pulse &pulse);
  void process_charge(const mtms_event_interfaces::msg::Charge &charge);
  void process_discharge(const mtms_event_interfaces::msg::Discharge &discharge);
  void process_trigger_out(const mtms_event_interfaces::msg::TriggerOut &trigger_out);

  rclcpp::Service<mtms_device_interfaces::srv::RequestEvents>::SharedPtr request_events_service;

  /* Pre-allocated serialized messages - one per event type to avoid any allocation on the request path. */
  SerializedMessage pulse_msg_;
  SerializedMessage charge_msg_;
  SerializedMessage discharge_msg_;
  SerializedMessage trigger_out_msg_;

  void tic();
  void toc(const std::string & prefix);
  std::chrono::time_point<std::chrono::system_clock> start_time;

  bool safe_mode;
};

EventHandler::EventHandler() : Node("event_handler") {
  this->declare_parameter<bool>("safe-mode", false);
  this->get_parameter("safe-mode", safe_mode);

  request_events_service = this->create_service<mtms_device_interfaces::srv::RequestEvents>(
      "/mtms/device/events/request", std::bind(&EventHandler::handle_request_events, this, std::placeholders::_1, std::placeholders::_2));
}

void EventHandler::handle_request_events(const std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Request> request,
                                         std::shared_ptr<mtms_device_interfaces::srv::RequestEvents::Response> response) {

  tic();

  /* Find the earliest event time. */
  double earliest_event_time = std::numeric_limits<double>::max();

  for (const auto &pulse : request->pulses) {
    if (pulse.event_info.execution_condition.value == mtms_event_interfaces::msg::ExecutionCondition::TIMED) {
      earliest_event_time = std::min(earliest_event_time, pulse.event_info.execution_time);
    }
  }

  for (const auto &charge : request->charges) {
    if (charge.event_info.execution_condition.value == mtms_event_interfaces::msg::ExecutionCondition::TIMED) {
      earliest_event_time = std::min(earliest_event_time, charge.event_info.execution_time);
    }
  }

  for (const auto &discharge : request->discharges) {
    if (discharge.event_info.execution_condition.value == mtms_event_interfaces::msg::ExecutionCondition::TIMED) {
      earliest_event_time = std::min(earliest_event_time, discharge.event_info.execution_time);
    }
  }

  for (const auto &trigger_out : request->trigger_outs) {
    if (trigger_out.event_info.execution_condition.value == mtms_event_interfaces::msg::ExecutionCondition::TIMED) {
      earliest_event_time = std::min(earliest_event_time, trigger_out.event_info.execution_time);
    }
  }

  /* Read current time from FPGA. */
  auto processing_start = std::chrono::steady_clock::now();
  uint64_t current_time_ticks;
  NiFpga_MergeStatus(&status, NiFpga_ReadU64(session, time_indicator, &current_time_ticks));
  double current_mtms_time = (double)current_time_ticks / CLOCK_FREQUENCY_HZ;

  /* Check if there is enough time to process the events. */
  double margin = earliest_event_time - current_mtms_time;

  if (margin < MINIMUM_MARGIN_S) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"),
                "Insufficient time margin (%.1f ms), rejecting batch (earliest event time: %.1f ms, current time: %.1f ms)",
                margin * 1000, earliest_event_time * 1000, current_mtms_time * 1000);
    response->success = false;
    return;
  }

  /* Set event aggregation lock before processing events. */
  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, event_aggregation_lock, NiFpga_True));

  for (const auto &pulse : request->pulses) {
    process_pulse(pulse);
  }
  for (const auto &charge : request->charges) {
    process_charge(charge);
  }
  for (const auto &discharge : request->discharges) {
    process_discharge(discharge);
  }
  for (const auto &trigger_out : request->trigger_outs) {
    process_trigger_out(trigger_out);
  }

  /* Reset event aggregation lock after processing events. */
  NiFpga_MergeStatus(&status, NiFpga_WriteBool(session, event_aggregation_lock, NiFpga_False));

  double processing_duration_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - processing_start).count();
  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Event batch processing took %.2f ms", processing_duration_ms);

  /* Log each pulse, charge, discharge and trigger out event. */
  for (const auto &pulse : request->pulses) {
    RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Executing pulse on channel %d (id: %d, execution_condition: %d, execution_time: %.4f s)",
                pulse.channel, pulse.event_info.id, pulse.event_info.execution_condition.value, pulse.event_info.execution_time);
  }
  for (const auto &charge : request->charges) {
    RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Charging channel %d to %d V (id: %d, execution_condition: %d, execution_time: %.4f s)",
                charge.channel, charge.target_voltage, charge.event_info.id, charge.event_info.execution_condition.value, charge.event_info.execution_time);
  }
  for (const auto &discharge : request->discharges) {
    RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Discharging channel %d to %d V (id: %d, execution_condition: %d, execution_time: %.4f s)",
                discharge.channel, discharge.target_voltage, discharge.event_info.id, discharge.event_info.execution_condition.value, discharge.event_info.execution_time);
  }
  for (const auto &trigger_out : request->trigger_outs) {
    RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Sent trigger out to port %d (id: %d, execution_condition: %d, execution_time: %.4f s)",
                trigger_out.port, trigger_out.event_info.id, trigger_out.event_info.execution_condition.value, trigger_out.event_info.execution_time);
  }

  toc("handle_request_events");

  response->success = true;
}

void EventHandler::process_pulse(const mtms_event_interfaces::msg::Pulse &pulse) {
  const auto &event_info = pulse.event_info;
  const uint16_t id = event_info.id;
  const uint8_t execution_condition = event_info.execution_condition.value;
  const double_t execution_time = event_info.execution_time;

  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Tried to execute pulse (id: %d) but FPGA is not in OK state", id);
    return;
  }

  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting pulse (id: %d)", id);
    return;
  }

  /* Serialize into pre-allocated buffer — init() resets cursor, no alloc. */
  const uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);
  pulse_msg_.init(pulse.channel + 1);
  pulse_msg_.add_uint16(id);
  pulse_msg_.add_byte(execution_condition);
  pulse_msg_.add_uint64(execution_time_ticks);

  const uint8_t num_of_waveform_pieces = (uint8_t) pulse.waveform.pieces.size();
  pulse_msg_.add_byte(num_of_waveform_pieces);

  for (uint8_t i = 0; i < num_of_waveform_pieces; i++) {
    const auto &piece = pulse.waveform.pieces[i];
    pulse_msg_.add_byte(piece.waveform_phase.value);
    pulse_msg_.add_uint16(piece.duration_in_ticks);
  }

  pulse_msg_.finalize();

  if (this->safe_mode) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Safe mode is enabled, aborting pulse (id: %d)", id);
    return;
  }

  NiFpga_MergeStatus(&status, NiFpga_StartFifo(session, channel_pulse_fifo));
  NiFpga_MergeStatus(&status, NiFpga_WriteFifoU8(session, channel_pulse_fifo, pulse_msg_.serialized_message.data(),
                                                 pulse_msg_.get_length(), NiFpga_InfiniteTimeout, NULL));
}

void EventHandler::process_charge(const mtms_event_interfaces::msg::Charge &charge) {
  const auto &event_info = charge.event_info;
  const uint16_t id = event_info.id;
  const uint8_t execution_condition = event_info.execution_condition.value;
  const double_t execution_time = event_info.execution_time;

  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "FPGA is not in OK state, aborting charge (id: %d)", id);
    return;
  }

  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting charge (id: %d)", id);
    return;
  }

  const uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);
  charge_msg_.init();
  charge_msg_.add_uint16(id);
  charge_msg_.add_byte(execution_condition);
  charge_msg_.add_uint64(execution_time_ticks);

  charge_msg_.add_byte(charge.channel + 1);
  charge_msg_.add_uint16(charge.target_voltage);
  charge_msg_.finalize();

  if (this->safe_mode) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Safe mode is enabled, aborting charge (id: %d)", id);
    return;
  }

  NiFpga_MergeStatus(&status, NiFpga_StartFifo(session, charge_fifo));
  NiFpga_MergeStatus(&status, NiFpga_WriteFifoU8(session, charge_fifo, charge_msg_.serialized_message.data(),
                                                 charge_msg_.get_length(), NiFpga_InfiniteTimeout, NULL));
}

void EventHandler::process_discharge(const mtms_event_interfaces::msg::Discharge &discharge) {
  const auto &event_info = discharge.event_info;
  const uint16_t id = event_info.id;
  const uint8_t execution_condition = event_info.execution_condition.value;
  const double_t execution_time = event_info.execution_time;

  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Tried to discharge (id: %d) but FPGA is not in OK state", id);
    return;
  }

  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting discharge (id: %d)", id);
    return;
  }

  const uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);
  discharge_msg_.init(discharge.channel + 1);
  discharge_msg_.add_uint16(id);
  discharge_msg_.add_byte(execution_condition);
  discharge_msg_.add_uint64(execution_time_ticks);

  discharge_msg_.add_uint16(discharge.target_voltage);
  discharge_msg_.finalize();

  if (this->safe_mode) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "Safe mode is enabled, aborting discharge (id: %d)", id);
    return;
  }

  NiFpga_MergeStatus(&status, NiFpga_StartFifo(session, discharge_fifo));
  NiFpga_MergeStatus(&status, NiFpga_WriteFifoU8(session, discharge_fifo, discharge_msg_.serialized_message.data(),
                                                 discharge_msg_.get_length(), NiFpga_InfiniteTimeout, NULL));
}

void EventHandler::process_trigger_out(const mtms_event_interfaces::msg::TriggerOut &trigger_out) {
  const auto &event_info = trigger_out.event_info;
  const uint16_t id = event_info.id;
  const uint8_t execution_condition = event_info.execution_condition.value;
  const double_t execution_time = event_info.execution_time;

  if (!is_fpga_ok()) {
    RCLCPP_WARN(rclcpp::get_logger("event_handler"), "FPGA is not in OK state, aborting sending trigger out event (id: %d)", id);
    return;
  }

  if (execution_time < 0.0) {
    RCLCPP_ERROR(rclcpp::get_logger("event_handler"), "Execution time cannot be negative, aborting trigger out (id: %d)", id);
    return;
  }

  const uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);
  trigger_out_msg_.init(trigger_out.port);
  trigger_out_msg_.add_uint16(id);
  trigger_out_msg_.add_byte(execution_condition);
  trigger_out_msg_.add_uint64(execution_time_ticks);

  const uint32_t duration_ticks = trigger_out.duration_us * (CLOCK_FREQUENCY_HZ / 1e6);
  trigger_out_msg_.add_uint32(duration_ticks);
  trigger_out_msg_.finalize();

  NiFpga_MergeStatus(&status, NiFpga_StartFifo(session, trigger_out_fifo));
  NiFpga_MergeStatus(&status, NiFpga_WriteFifoU8(session, trigger_out_fifo, trigger_out_msg_.serialized_message.data(),
                                                 trigger_out_msg_.get_length(), NiFpga_InfiniteTimeout, NULL));
}

void EventHandler::tic() {
  start_time = std::chrono::system_clock::now();
}

void EventHandler::toc(const std::string & prefix) {
  const auto end_time = std::chrono::system_clock::now();
  const double start_s = std::chrono::duration<double>(start_time.time_since_epoch()).count();
  const double end_s = std::chrono::duration<double>(end_time.time_since_epoch()).count();
  const double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
  RCLCPP_INFO(this->get_logger(), "%s: start=%.3f s, end=%.3f s, duration=%.1f ms", prefix.c_str(), start_s, end_s, duration_ms);
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto logger = rclcpp::get_logger("event_handler");

  realtime_utils::MemoryConfig mem_config;
  mem_config.enable_memory_optimization = true;
  mem_config.preallocate_size = 10 * 1024 * 1024; // 10 MB

  realtime_utils::SchedulingConfig sched_config;
  sched_config.enable_scheduling_optimization = true;
  sched_config.scheduling_policy = SCHED_RR;
  sched_config.priority_level = realtime_utils::PriorityLevel::HIGHEST_REALTIME;

  try {
    realtime_utils::initialize_scheduling(sched_config, logger);
    realtime_utils::initialize_memory(mem_config, logger);
  } catch (const std::exception& e) {
    RCLCPP_FATAL(logger, "Initialization failed: %s", e.what());
    return -1;
  }

  auto node = std::make_shared<EventHandler>();

  RCLCPP_INFO(rclcpp::get_logger("event_handler"), "Event handler ready.");

  auto timer = node->create_wall_timer(
      std::chrono::milliseconds(FPGA_OK_CHECK_INTERVAL_MS),
      [&]() {
          if (!is_fpga_ok()) {
              init_fpga();
          }
      }
  );
  rclcpp::spin(node);

  close_fpga();
  rclcpp::shutdown();
}