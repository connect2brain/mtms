#include "mtms_simulator/neurone_trigger_client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>

NeurOneTriggerClient::NeurOneTriggerClient(
  const rclcpp::Logger & logger, const std::string & host, uint16_t port)
  : logger_(logger),
    host_(host),
    port_(port)
{
  worker_ = std::thread(&NeurOneTriggerClient::WorkerLoop, this);
}

NeurOneTriggerClient::~NeurOneTriggerClient()
{
  stop_.store(true, std::memory_order_relaxed);
  cv_.notify_all();
  if (worker_.joinable()) {
    worker_.join();
  }
  CloseSocket();
}

std::string NeurOneTriggerClient::TokenForPort(uint8_t port)
{
  switch (port) {
    case 1:
      return "trigger_a";
    case 2:
      return "trigger_b";
    default:
      return {};
  }
}

void NeurOneTriggerClient::EnqueueTriggerForPort(uint8_t port)
{
  const std::string token = TokenForPort(port);
  if (token.empty()) {
    RCLCPP_WARN(logger_, "mTMS simulator: unsupported trigger_out port %u (expected 1 or 2)", port);
    return;
  }

  {
    std::lock_guard<std::mutex> lock(mu_);
    pending_tokens_.push_back(token);
  }
  cv_.notify_one();
}

void NeurOneTriggerClient::CloseSocket()
{
  std::lock_guard<std::mutex> lock(mu_);
  if (sock_fd_ >= 0) {
    ::shutdown(sock_fd_, SHUT_RDWR);
    ::close(sock_fd_);
    sock_fd_ = -1;
  }
}

bool NeurOneTriggerClient::IsSocketClosed() const
{
  std::lock_guard<std::mutex> lock(mu_);
  return sock_fd_ < 0;
}

bool NeurOneTriggerClient::SendTokenAll(int fd, const std::string & token)
{
  size_t sent_total = 0;
  while (sent_total < token.size()) {
    const ssize_t rc = ::send(
      fd, token.data() + sent_total, token.size() - sent_total, 0);
    if (rc < 0) {
      if (errno == EINTR) continue;
      return false;
    }
    if (rc == 0) return false;
    sent_total += static_cast<size_t>(rc);
  }
  return true;
}

int NeurOneTriggerClient::TryConnect()
{
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  const int current_flags = ::fcntl(fd, F_GETFL, 0);
  if (current_flags >= 0) {
    ::fcntl(fd, F_SETFL, current_flags | O_NONBLOCK);
  }

  sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port_);
  if (::inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1) {
    ::close(fd);
    return -1;
  }

  int rc = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  if (rc == 0) {
    return fd;
  }

  if (rc < 0 && errno != EINPROGRESS) {
    ::close(fd);
    return -1;
  }

  fd_set write_fds;
  FD_ZERO(&write_fds);
  FD_SET(fd, &write_fds);

  timeval tv {};
  tv.tv_sec = 0;
  tv.tv_usec = 200000;  // 200ms connect timeout

  const int sel = ::select(fd + 1, nullptr, &write_fds, nullptr, &tv);
  if (sel <= 0) {
    ::close(fd);
    return -1;
  }

  int so_error = 0;
  socklen_t so_error_len = sizeof(so_error);
  if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len) != 0 || so_error != 0) {
    ::close(fd);
    return -1;
  }

  return fd;
}

void NeurOneTriggerClient::WorkerLoop()
{
  using namespace std::chrono_literals;

  while (!stop_.load(std::memory_order_relaxed)) {
    if (IsSocketClosed()) {
      const int fd = TryConnect();
      if (fd < 0) {
        std::this_thread::sleep_for(1s);
        continue;
      }

      {
        std::lock_guard<std::mutex> lock(mu_);
        sock_fd_ = fd;
      }

      RCLCPP_INFO(
        logger_, "mTMS simulator: connected to NeurOne trigger server at %s:%u", host_.c_str(), port_);
    }

    std::string token;
    int fd_to_send = -1;
    {
      std::unique_lock<std::mutex> lock(mu_);
      cv_.wait_for(lock, 1s, [&]() {
        return stop_.load(std::memory_order_relaxed) || !pending_tokens_.empty();
      });

      if (stop_.load(std::memory_order_relaxed)) {
        return;
      }

      if (sock_fd_ >= 0 && !pending_tokens_.empty()) {
        token = pending_tokens_.front();
        pending_tokens_.pop_front();
        fd_to_send = sock_fd_;
      }
    }

    if (!token.empty() && fd_to_send >= 0) {
      if (!SendTokenAll(fd_to_send, token)) {
        RCLCPP_WARN(logger_, "mTMS simulator: NeurOne trigger connection lost; will retry");
        CloseSocket();
      }
      continue;
    }

    // No token to send: probe socket for closure periodically.
    int fd_probe = -1;
    {
      std::lock_guard<std::mutex> lock(mu_);
      fd_probe = sock_fd_;
    }
    if (fd_probe >= 0) {
      char buf {};
      const ssize_t n = ::recv(fd_probe, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
      if (n == 0) {
        RCLCPP_WARN(logger_, "mTMS simulator: NeurOne trigger connection closed by peer; will retry");
        CloseSocket();
      } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        RCLCPP_WARN(logger_, "mTMS simulator: NeurOne trigger connection error; will retry");
        CloseSocket();
      }
    }
  }
}

