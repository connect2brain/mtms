#ifndef TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_
#define TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_

#include <mutex>
#include <optional>

#include "rclcpp/rclcpp.hpp"

#include "mtms_eeg_interfaces/msg/sample.hpp"
#include "mtms_system_interfaces/msg/session.hpp"
#include "mtms_system_interfaces/msg/timebase_mapping.hpp"

#include "ring_buffer.h"

/* A matched pair of one EEG sample and the session message that triggered pairing. */
struct SamplePair {
  mtms_eeg_interfaces::msg::Sample eeg_sample;
  mtms_system_interfaces::msg::Session session;
};

class TimebaseCalibrator : public rclcpp::Node {
public:
  TimebaseCalibrator();

  /* Returns a copy of all currently stored pairs (oldest first). */
  std::vector<SamplePair> get_pairs() const;

private:
  static constexpr size_t PAIR_BUFFER_SIZE = 10;

  void eeg_callback(const mtms_eeg_interfaces::msg::Sample::SharedPtr msg);
  void session_callback(const mtms_system_interfaces::msg::Session::SharedPtr msg);

  /* Fits session_time = scale * eeg_time + offset over all buffered pairs.
     Returns false if the fit cannot be computed (fewer than 2 pairs, or
     degenerate timestamps). */
  bool compute_lms(double & scale, double & offset) const;

  /* Protects latest_eeg_sample and pairs. */
  mutable std::mutex mutex;

  /* Most-recently received EEG sample; empty until the first sample arrives. */
  std::optional<mtms_eeg_interfaces::msg::Sample> latest_eeg_sample;

  /* Ring buffer holding the last PAIR_BUFFER_SIZE matched pairs. */
  RingBuffer<SamplePair> pairs;

  rclcpp::Subscription<mtms_eeg_interfaces::msg::Sample>::SharedPtr eeg_subscription;
  rclcpp::Subscription<mtms_system_interfaces::msg::Session>::SharedPtr session_subscription;

  rclcpp::Publisher<mtms_system_interfaces::msg::TimebaseMapping>::SharedPtr eeg_to_mtms_publisher;
  rclcpp::Publisher<mtms_system_interfaces::msg::TimebaseMapping>::SharedPtr mtms_to_eeg_publisher;
};

#endif  // TIMEBASE_CALIBRATOR__TIMEBASE_CALIBRATOR_H_
