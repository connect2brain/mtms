#ifndef MTMS_SIMULATOR__NEURONE_TRIGGER_CLIENT_HPP_
#define MTMS_SIMULATOR__NEURONE_TRIGGER_CLIENT_HPP_

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"

// Keeps a TCP connection to NeurOne's trigger server alive, retrying until the port is available.
// MTMS trigger_out ports are mapped to NeurOne trigger tokens:
//   port 1 -> "trigger_a"
//   port 2 -> "trigger_b"
class NeurOneTriggerClient {
public:
  explicit NeurOneTriggerClient(
    const rclcpp::Logger & logger,
    const std::string & host = "127.0.0.1",
    uint16_t port = 60000);
  ~NeurOneTriggerClient();

  // Best-effort: enqueues the token and the background worker will send it once connected.
  void EnqueueTriggerForPort(uint8_t port);

  NeurOneTriggerClient(const NeurOneTriggerClient &) = delete;
  NeurOneTriggerClient & operator=(const NeurOneTriggerClient &) = delete;

private:
  void WorkerLoop();
  int TryConnect();
  void CloseSocket();
  bool IsSocketClosed() const;
  bool SendTokenAll(int fd, const std::string & token);

  static std::string TokenForPort(uint8_t port);

  rclcpp::Logger logger_;
  std::string host_;
  uint16_t port_;

  std::atomic<bool> stop_{false};
  std::thread worker_;

  mutable std::mutex mu_;
  std::condition_variable cv_;
  std::deque<std::string> pending_tokens_;

  // Protected by mu_. Only accessed by the worker thread for simplicity.
  int sock_fd_{-1};
};

#endif  // MTMS_SIMULATOR__NEURONE_TRIGGER_CLIENT_HPP_

