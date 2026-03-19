#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

struct FrameType {
  static constexpr uint8_t MEASUREMENT_START = 1;
  static constexpr uint8_t SAMPLES = 2;
  static constexpr uint8_t MEASUREMENT_END = 4;
};

struct StartPacketFieldIndex {
  static constexpr size_t FRAME_TYPE = 0;
  static constexpr size_t MAIN_UNIT_NUM = 1;
  static constexpr size_t RESERVED = 2;
  static constexpr size_t SAMPLING_RATE_HZ = 4;
  static constexpr size_t SAMPLE_FORMAT = 8;
  static constexpr size_t TRIGGER_DEFS = 12;
  static constexpr size_t NUM_CHANNELS = 16;
  static constexpr size_t SOURCE_CHANNEL = 18;
};

struct SamplesPacketFieldIndex {
  static constexpr size_t SAMPLE_PACKET_SEQ_NO = 4;
  static constexpr size_t SAMPLE_NUM_CHANNELS = 8;
  static constexpr size_t SAMPLE_NUM_SAMPLE_BUNDLES = 10;
  static constexpr size_t SAMPLE_FIRST_SAMPLE_INDEX = 12;
  static constexpr size_t SAMPLE_FIRST_SAMPLE_TIME = 20;
  static constexpr size_t SAMPLE_SAMPLES = 28;
};

constexpr int DEFAULT_PORT = 50000;
constexpr int DEFAULT_SAMPLING_RATE = 5000;
constexpr int DEFAULT_EEG_CHANNELS = 63;
constexpr int DEFAULT_EMG_CHANNELS = 10;
constexpr int DEFAULT_TRIGGER_PORT = 60000;
constexpr double DEFAULT_TRIGGER_A_DELAY = 0.0;

void WriteU16BE(std::vector<uint8_t>& buf, size_t off, uint16_t value) {
  buf[off] = static_cast<uint8_t>((value >> 8) & 0xFF);
  buf[off + 1] = static_cast<uint8_t>(value & 0xFF);
}

void WriteU32BE(std::vector<uint8_t>& buf, size_t off, uint32_t value) {
  buf[off] = static_cast<uint8_t>((value >> 24) & 0xFF);
  buf[off + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
  buf[off + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
  buf[off + 3] = static_cast<uint8_t>(value & 0xFF);
}

void WriteU64BE(std::vector<uint8_t>& buf, size_t off, uint64_t value) {
  buf[off] = static_cast<uint8_t>((value >> 56) & 0xFF);
  buf[off + 1] = static_cast<uint8_t>((value >> 48) & 0xFF);
  buf[off + 2] = static_cast<uint8_t>((value >> 40) & 0xFF);
  buf[off + 3] = static_cast<uint8_t>((value >> 32) & 0xFF);
  buf[off + 4] = static_cast<uint8_t>((value >> 24) & 0xFF);
  buf[off + 5] = static_cast<uint8_t>((value >> 16) & 0xFF);
  buf[off + 6] = static_cast<uint8_t>((value >> 8) & 0xFF);
  buf[off + 7] = static_cast<uint8_t>(value & 0xFF);
}

struct Config {
  int port = DEFAULT_PORT;
  int trigger_port = DEFAULT_TRIGGER_PORT;
  int sampling_rate = DEFAULT_SAMPLING_RATE;
  int eeg_channels = DEFAULT_EEG_CHANNELS;
  int emg_channels = DEFAULT_EMG_CHANNELS;
  double trigger_a_delay = DEFAULT_TRIGGER_A_DELAY;
  double duration_seconds = -1.0;
  bool enable_trigger_b = true;
  bool simulate_dropped_samples = false;
};

void PrintUsage(const char* prog_name) {
  std::cout
      << "NeurOne EEG Simulator (C++)\n\n"
      << "Usage:\n  " << prog_name << " [options]\n\n"
      << "Options:\n"
      << "  --port <int>                    UDP destination port (default: "
      << DEFAULT_PORT << ")\n"
      << "  --trigger-port <int>            TCP trigger listen port (default: "
      << DEFAULT_TRIGGER_PORT << ")\n"
      << "  --sampling-rate <int>           Sampling rate Hz (default: "
      << DEFAULT_SAMPLING_RATE << ")\n"
      << "  --eeg-channels <int>            EEG channel count (default: "
      << DEFAULT_EEG_CHANNELS << ")\n"
      << "  --emg-channels <int>            EMG channel count (default: "
      << DEFAULT_EMG_CHANNELS << ")\n"
      << "  --trigger-a-delay <float>       trigger_a delay seconds (default: "
      << DEFAULT_TRIGGER_A_DELAY << ")\n"
      << "  --duration <float>              Duration seconds (default: run until interrupted)\n"
      << "  --disable-trigger-b             Disable trigger_b in trigger channel\n"
      << "  --simulate-dropped-samples      Skip sample packets in increasing bursts\n"
      << "  --help                          Show this help\n";
}

enum class ParseResult { kOk, kHelp, kError };

ParseResult ParseArgs(int argc, char** argv, Config& cfg) {
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    auto read_next = [&](const char* name) -> const char* {
      if (i + 1 >= argc) {
        std::cerr << "Missing value for " << name << "\n";
        return nullptr;
      }
      return argv[++i];
    };

    if (arg == "--port") {
      const char* v = read_next("--port");
      if (!v) return ParseResult::kError;
      cfg.port = std::stoi(v);
    } else if (arg == "--trigger-port") {
      const char* v = read_next("--trigger-port");
      if (!v) return ParseResult::kError;
      cfg.trigger_port = std::stoi(v);
    } else if (arg == "--sampling-rate") {
      const char* v = read_next("--sampling-rate");
      if (!v) return ParseResult::kError;
      cfg.sampling_rate = std::stoi(v);
    } else if (arg == "--eeg-channels") {
      const char* v = read_next("--eeg-channels");
      if (!v) return ParseResult::kError;
      cfg.eeg_channels = std::stoi(v);
    } else if (arg == "--emg-channels") {
      const char* v = read_next("--emg-channels");
      if (!v) return ParseResult::kError;
      cfg.emg_channels = std::stoi(v);
    } else if (arg == "--trigger-a-delay") {
      const char* v = read_next("--trigger-a-delay");
      if (!v) return ParseResult::kError;
      cfg.trigger_a_delay = std::stod(v);
    } else if (arg == "--duration") {
      const char* v = read_next("--duration");
      if (!v) return ParseResult::kError;
      cfg.duration_seconds = std::stod(v);
    } else if (arg == "--disable-trigger-b") {
      cfg.enable_trigger_b = false;
    } else if (arg == "--simulate-dropped-samples") {
      cfg.simulate_dropped_samples = true;
    } else if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      return ParseResult::kHelp;
    } else {
      std::cerr << "Unknown option: " << arg << "\n";
      return ParseResult::kError;
    }
  }
  return ParseResult::kOk;
}

class NeurOneSimulator {
 public:
  explicit NeurOneSimulator(const Config& cfg)
      : cfg_(cfg),
        total_channels_(cfg_.eeg_channels + cfg_.emg_channels + 1),
        rng_(std::random_device{}()),
        eeg_noise_(0.0, 5.0),
        emg_noise_(0.0, 2.0) {
    BuildSourceChannels();
    SetupSockets();
  }

  ~NeurOneSimulator() { CleanupSockets(); }

  void Stop() {
    running_.store(false, std::memory_order_relaxed);
    if (trigger_sock_ >= 0) {
      ::shutdown(trigger_sock_, SHUT_RDWR);
      ::close(trigger_sock_);
      trigger_sock_ = -1;
    }
  }

  void Run(double duration_seconds) {
    running_.store(true, std::memory_order_relaxed);
    sample_index_ = 0;
    simulation_start_wall_ = std::chrono::steady_clock::now();
    next_drop_time_set_ = false;
    pending_dropped_samples_ = 0;
    total_simulated_dropped_samples_ = 0;

    std::cout << "Starting NeurOne simulator on port " << cfg_.port << "\n";
    std::cout << "Sampling rate: " << cfg_.sampling_rate << " Hz\n";
    std::cout << "EEG channels: " << cfg_.eeg_channels << ", EMG channels: " << cfg_.emg_channels << "\n";
    std::cout << "Trigger server listening on port " << cfg_.trigger_port << "\n";
    std::cout << "Trigger A delay: " << (cfg_.trigger_a_delay * 1000.0) << " ms\n";
    if (cfg_.simulate_dropped_samples) {
      std::cout << "Dropped sample simulation enabled (burst every 1s, +1 burst size every 5s)\n";
    }

    std::thread trigger_thread(&NeurOneSimulator::HandleTriggerConnections, this);

    SendMeasurementStartPacket();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto run_start_wall = std::chrono::steady_clock::now();
    const auto run_start_steady = std::chrono::steady_clock::now();
    const double sample_period = 1.0 / static_cast<double>(cfg_.sampling_rate);

    while (running_.load(std::memory_order_relaxed)) {
      const auto target =
          run_start_steady +
          std::chrono::nanoseconds(static_cast<long long>(sample_index_ * sample_period * 1e9));
      std::this_thread::sleep_until(target);

      const auto now = std::chrono::steady_clock::now();
      if (ShouldDropCurrentSample(now)) {
        ++sample_index_;
      } else {
        SendSamplePacket();
      }

      if (duration_seconds > 0.0) {
        const double elapsed = std::chrono::duration<double>(now - run_start_wall).count();
        if (elapsed >= duration_seconds) {
          break;
        }
      }
    }

    running_.store(false, std::memory_order_relaxed);
    Stop();
    if (trigger_thread.joinable()) {
      trigger_thread.join();
    }

    SendMeasurementEndPacket();
    std::cout << "Simulation completed. Final sample index " << sample_index_
              << ", simulated dropped samples " << total_simulated_dropped_samples_ << "\n";
  }

 private:
  void BuildSourceChannels() {
    const std::vector<std::pair<int, int>> eeg_ranges = {{1, 32}, {41, 72}, {81, 112}, {121, 152}};
    const std::vector<std::pair<int, int>> emg_ranges = {{33, 40}, {73, 80}, {113, 120}, {153, 160}};

    int max_eeg = 0;
    for (const auto& r : eeg_ranges) max_eeg += (r.second - r.first + 1);
    int max_emg = 0;
    for (const auto& r : emg_ranges) max_emg += (r.second - r.first + 1);

    if (cfg_.eeg_channels > max_eeg) {
      throw std::runtime_error("Requested EEG channels exceed NeurOne-compatible source range.");
    }
    if (cfg_.emg_channels > max_emg) {
      throw std::runtime_error("Requested EMG channels exceed NeurOne-compatible source range.");
    }

    int remaining_eeg = cfg_.eeg_channels;
    for (const auto& [start, end] : eeg_ranges) {
      if (remaining_eeg <= 0) break;
      const int count = std::min(end - start + 1, remaining_eeg);
      for (int ch = start; ch < start + count; ++ch) {
        source_channels_.push_back(static_cast<uint16_t>(ch));
      }
      remaining_eeg -= count;
    }

    int remaining_emg = cfg_.emg_channels;
    for (const auto& [start, end] : emg_ranges) {
      if (remaining_emg <= 0) break;
      const int count = std::min(end - start + 1, remaining_emg);
      for (int ch = start; ch < start + count; ++ch) {
        source_channels_.push_back(static_cast<uint16_t>(ch));
      }
      remaining_emg -= count;
    }

    source_channels_.push_back(65535);
  }

  void SetupSockets() {
    udp_sock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock_ < 0) throw std::runtime_error("Failed to create UDP socket.");

    int on = 1;
    ::setsockopt(udp_sock_, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    sockaddr_in udp_bind {};
    udp_bind.sin_family = AF_INET;
    udp_bind.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_bind.sin_port = htons(0);
    if (::bind(udp_sock_, reinterpret_cast<sockaddr*>(&udp_bind), sizeof(udp_bind)) < 0) {
      throw std::runtime_error("Failed to bind UDP socket.");
    }

    device_addr_.sin_family = AF_INET;
    device_addr_.sin_port = htons(static_cast<uint16_t>(cfg_.port));
    if (::inet_pton(AF_INET, "127.0.0.1", &device_addr_.sin_addr) != 1) {
      throw std::runtime_error("Failed to parse destination IP.");
    }

    trigger_sock_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (trigger_sock_ < 0) throw std::runtime_error("Failed to create TCP trigger socket.");

    ::setsockopt(trigger_sock_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    sockaddr_in trigger_bind {};
    trigger_bind.sin_family = AF_INET;
    trigger_bind.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    trigger_bind.sin_port = htons(static_cast<uint16_t>(cfg_.trigger_port));
    if (::bind(trigger_sock_, reinterpret_cast<sockaddr*>(&trigger_bind), sizeof(trigger_bind)) < 0) {
      throw std::runtime_error("Failed to bind TCP trigger socket.");
    }
    if (::listen(trigger_sock_, 1) < 0) {
      throw std::runtime_error("Failed to listen on TCP trigger socket.");
    }
  }

  void CleanupSockets() {
    if (udp_sock_ >= 0) {
      ::close(udp_sock_);
      udp_sock_ = -1;
    }
    if (trigger_sock_ >= 0) {
      ::close(trigger_sock_);
      trigger_sock_ = -1;
    }
  }

  int GetDropBurstSize(double elapsed_seconds) const {
    return static_cast<int>(elapsed_seconds / 5.0) + 1;
  }

  bool ShouldDropCurrentSample(std::chrono::steady_clock::time_point now) {
    if (!cfg_.simulate_dropped_samples) {
      return false;
    }

    if (!next_drop_time_set_) {
      next_drop_time_ = simulation_start_wall_ + std::chrono::seconds(1);
      next_drop_time_set_ = true;
    }

    if (pending_dropped_samples_ == 0 && now >= next_drop_time_) {
      const double elapsed = std::chrono::duration<double>(now - simulation_start_wall_).count();
      pending_dropped_samples_ = GetDropBurstSize(elapsed);
      const int burst_size = pending_dropped_samples_;

      while (next_drop_time_ <= now) {
        next_drop_time_ += std::chrono::seconds(1);
      }

      std::cout << "Simulating dropped sample burst: " << burst_size
                << " consecutive (elapsed " << elapsed << " s)\n";
    }

    if (pending_dropped_samples_ > 0) {
      --pending_dropped_samples_;
      ++total_simulated_dropped_samples_;
      return true;
    }
    return false;
  }

  void HandleTriggerConnections() {
    std::cout << "Listening for triggers on port " << cfg_.trigger_port << "\n";
    while (running_.load(std::memory_order_relaxed)) {
      if (trigger_sock_ < 0) break;

      fd_set readfds;
      FD_ZERO(&readfds);
      FD_SET(trigger_sock_, &readfds);
      timeval tv {};
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      const int sel = ::select(trigger_sock_ + 1, &readfds, nullptr, nullptr, &tv);
      if (sel < 0) {
        if (errno == EINTR) continue;
        break;
      }
      if (sel == 0) continue;
      if (!FD_ISSET(trigger_sock_, &readfds)) continue;

      sockaddr_in client_addr {};
      socklen_t client_len = sizeof(client_addr);
      int client = ::accept(trigger_sock_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
      if (client < 0) {
        if (errno == EINTR) continue;
        continue;
      }

      std::cout << "Accepted trigger connection\n";
      HandleClientConnection(client);
      ::close(client);
    }
  }

  void HandleClientConnection(int client_sock) {
    timeval recv_to {};
    recv_to.tv_sec = 1;
    recv_to.tv_usec = 0;
    ::setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &recv_to, sizeof(recv_to));

    char buf[1024];
    while (running_.load(std::memory_order_relaxed)) {
      const ssize_t n = ::recv(client_sock, buf, sizeof(buf) - 1, 0);
      if (n == 0) {
        break;
      }
      if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
          continue;
        }
        break;
      }
      buf[n] = '\0';

      std::istringstream iss(buf);
      std::string token;
      while (iss >> token) {
        ScheduleTrigger(token);
      }
    }
  }

  void ScheduleTrigger(const std::string& message) {
    if (message != "trigger_a" && message != "trigger_b") return;

    const auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(trigger_mu_);

    if (message == "trigger_a") {
      auto at = now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                          std::chrono::duration<double>(cfg_.trigger_a_delay));
      pending_triggers_.push_back({at, TriggerType::A});
      std::cout << "Requested trigger_a, scheduled to appear in "
                << (cfg_.trigger_a_delay * 1000.0) << " ms\n";
    } else {
      pending_triggers_.push_back({now, TriggerType::B});
      std::cout << "Requested trigger_b, appears immediately\n";
    }
  }

  static std::array<uint8_t, 3> Int32ToInt24Bytes(int32_t value) {
    value = std::max(-(1 << 23), std::min((1 << 23) - 1, value));
    const uint32_t u = static_cast<uint32_t>(value);
    return {static_cast<uint8_t>((u >> 16) & 0xFF), static_cast<uint8_t>((u >> 8) & 0xFF),
            static_cast<uint8_t>(u & 0xFF)};
  }

  int32_t GenerateSampleData(int channel_index, bool trigger_a, bool trigger_b) {
    const double t = static_cast<double>(sample_index_) / static_cast<double>(cfg_.sampling_rate);
    constexpr double pi = 3.14159265358979323846;

    if (channel_index < cfg_.eeg_channels) {
      const double alpha_freq = 10.0;
      const double base_signal = 50.0 * std::sin(2.0 * pi * alpha_freq * t);
      const double noise = eeg_noise_(rng_);
      return static_cast<int32_t>((base_signal + noise) * 1000.0);
    }
    if (channel_index < (cfg_.eeg_channels + cfg_.emg_channels)) {
      const double emg_freq = 50.0;
      const double base_signal = 20.0 * std::sin(2.0 * pi * emg_freq * t);
      const double noise = emg_noise_(rng_);
      return static_cast<int32_t>((base_signal + noise) * 1000.0);
    }

    int32_t trigger_value = 0;
    if (trigger_a) trigger_value |= (1 << 1);
    if (trigger_b) trigger_value |= (1 << 3);
    return trigger_value;
  }

  void SendMeasurementStartPacket() {
    const size_t packet_size = StartPacketFieldIndex::SOURCE_CHANNEL + 2 * total_channels_;
    std::vector<uint8_t> packet(packet_size, 0);

    packet[StartPacketFieldIndex::FRAME_TYPE] = FrameType::MEASUREMENT_START;
    packet[StartPacketFieldIndex::MAIN_UNIT_NUM] = 1;
    packet[StartPacketFieldIndex::RESERVED] = 0;
    WriteU32BE(packet, StartPacketFieldIndex::SAMPLING_RATE_HZ, static_cast<uint32_t>(cfg_.sampling_rate));
    WriteU32BE(packet, StartPacketFieldIndex::SAMPLE_FORMAT, 1);
    WriteU32BE(packet, StartPacketFieldIndex::TRIGGER_DEFS, 0);
    WriteU16BE(packet, StartPacketFieldIndex::NUM_CHANNELS, static_cast<uint16_t>(total_channels_));
    for (size_t i = 0; i < source_channels_.size(); ++i) {
      WriteU16BE(packet, StartPacketFieldIndex::SOURCE_CHANNEL + 2 * i, source_channels_[i]);
    }

    std::cout << "Sending measurement start packet: " << total_channels_ << " channels, "
              << cfg_.sampling_rate << " Hz\n";
    ::sendto(udp_sock_, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&device_addr_),
             sizeof(device_addr_));
  }

  void SendSamplePacket() {
    const size_t packet_size = SamplesPacketFieldIndex::SAMPLE_SAMPLES + 3 * total_channels_;
    std::vector<uint8_t> packet(packet_size, 0);
    packet[0] = FrameType::SAMPLES;
    WriteU32BE(packet, SamplesPacketFieldIndex::SAMPLE_PACKET_SEQ_NO, static_cast<uint32_t>(sample_index_ % 10000));
    WriteU16BE(packet, SamplesPacketFieldIndex::SAMPLE_NUM_CHANNELS, static_cast<uint16_t>(total_channels_));
    WriteU16BE(packet, SamplesPacketFieldIndex::SAMPLE_NUM_SAMPLE_BUNDLES, 1);
    WriteU64BE(packet, SamplesPacketFieldIndex::SAMPLE_FIRST_SAMPLE_INDEX, sample_index_);

    const auto now_sys = std::chrono::system_clock::now().time_since_epoch();
    const uint64_t current_time_us =
        static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(now_sys).count());
    WriteU64BE(packet, SamplesPacketFieldIndex::SAMPLE_FIRST_SAMPLE_TIME, current_time_us);

    bool trigger_a = false;
    bool trigger_b = false;
    const auto now = std::chrono::steady_clock::now();
    {
      std::lock_guard<std::mutex> lock(trigger_mu_);
      std::vector<std::pair<std::chrono::steady_clock::time_point, TriggerType>> remaining;
      remaining.reserve(pending_triggers_.size());
      for (const auto& [when, type] : pending_triggers_) {
        if (now >= when) {
          if (type == TriggerType::A) trigger_a = true;
          if (type == TriggerType::B) trigger_b = true;
        } else {
          remaining.push_back({when, type});
        }
      }
      pending_triggers_.swap(remaining);
    }

    if (!cfg_.enable_trigger_b) {
      trigger_b = false;
    }

    for (int i = 0; i < total_channels_; ++i) {
      const int32_t value = GenerateSampleData(i, trigger_a, trigger_b);
      const auto bytes = Int32ToInt24Bytes(value);
      const size_t off = SamplesPacketFieldIndex::SAMPLE_SAMPLES + 3 * i;
      packet[off] = bytes[0];
      packet[off + 1] = bytes[1];
      packet[off + 2] = bytes[2];
    }

    ::sendto(udp_sock_, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&device_addr_),
             sizeof(device_addr_));
    ++sample_index_;
  }

  void SendMeasurementEndPacket() {
    std::vector<uint8_t> packet(4, 0);
    packet[0] = FrameType::MEASUREMENT_END;
    std::cout << "Sending measurement end packet\n";
    ::sendto(udp_sock_, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&device_addr_),
             sizeof(device_addr_));
  }

  enum class TriggerType { A, B };

  Config cfg_;
  int total_channels_;
  int udp_sock_ = -1;
  int trigger_sock_ = -1;
  sockaddr_in device_addr_ {};

  std::vector<uint16_t> source_channels_;
  std::atomic<bool> running_ {false};
  uint64_t sample_index_ = 0;

  std::chrono::steady_clock::time_point simulation_start_wall_ {};
  std::chrono::steady_clock::time_point next_drop_time_ {};
  bool next_drop_time_set_ = false;
  int pending_dropped_samples_ = 0;
  uint64_t total_simulated_dropped_samples_ = 0;

  std::mutex trigger_mu_;
  std::vector<std::pair<std::chrono::steady_clock::time_point, TriggerType>> pending_triggers_;

  std::mt19937 rng_;
  std::normal_distribution<double> eeg_noise_;
  std::normal_distribution<double> emg_noise_;
};

NeurOneSimulator* g_simulator = nullptr;

void HandleSignal(int) {
  if (g_simulator) {
    std::cout << "\nStopping simulator...\n";
    g_simulator->Stop();
  }
}

}  // namespace

int main(int argc, char** argv) {
  Config cfg;
  try {
    const ParseResult parse_result = ParseArgs(argc, argv, cfg);
    if (parse_result == ParseResult::kHelp) {
      return 0;
    }
    if (parse_result == ParseResult::kError) {
      PrintUsage(argv[0]);
      return 1;
    }

    NeurOneSimulator simulator(cfg);
    g_simulator = &simulator;

    ::signal(SIGINT, HandleSignal);
    ::signal(SIGTERM, HandleSignal);

    simulator.Run(cfg.duration_seconds);
    g_simulator = nullptr;
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
