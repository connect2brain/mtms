#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/srv/send_signal_out.hpp"
#include "event_interfaces/msg/signal_out.hpp"
#include "event_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 signal_out_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetSignalOutFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class SignalOutHandler : public rclcpp::Node {
public:
  SignalOutHandler()
      : Node("signal_out_handler") {

    auto service_callback = [this](const std::shared_ptr<event_interfaces::srv::SendSignalOut::Request> request,
                                   std::shared_ptr<event_interfaces::srv::SendSignalOut::Response> response) -> void {
      event_interfaces::msg::SignalOut signal_out = request->signal_out;

      uint8_t port = signal_out.port;

      event_interfaces::msg::EventInfo event_info = signal_out.event_info;

      uint16_t id = event_info.id;
      uint8_t execution_condition = event_info.execution_condition.value;
      double_t execution_time = event_info.execution_time;
      uint64_t execution_time_ticks = (uint64_t)(execution_time * CLOCK_FREQUENCY_HZ);

      serialized_message.init(port);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(execution_time_ticks);

      /* Serialize signal out. */

      uint32_t duration_us = signal_out.duration_us;
      uint32_t duration_ticks = duration_us * CLOCK_FREQUENCY_HZ / 1e6;

      serialized_message.add_uint32(duration_ticks);

      serialized_message.finalize();

      /* For consistency with channel indexing, start signal out indexing from 1. */
      NiFpga_MergeStatus(&status,
                         NiFpga_StartFifo(session,
                                          signal_out_fifo));

      NiFpga_MergeStatus(&status,
                         NiFpga_WriteFifoU8(session,
                                            signal_out_fifo,
                                            serialized_message.serialized_message.data(),
                                            serialized_message.get_length(),
                                            NiFpga_InfiniteTimeout,
                                            NULL));

      RCLCPP_INFO(rclcpp::get_logger("signal_out_handler"), "Sent signal out to port %d", port);

      response->success = true;
    };


    serialized_message = SerializedMessage();
    send_signal_out_service_ = this->create_service<event_interfaces::srv::SendSignalOut>(
        "/event/send_signal_out", service_callback);
  }

private:
  rclcpp::Service<event_interfaces::srv::SendSignalOut>::SharedPtr send_signal_out_service_;
  SerializedMessage serialized_message;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);


#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("signal_out_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<SignalOutHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("signal_out_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif
  RCLCPP_INFO(rclcpp::get_logger("signal_out_handler"), "Signal out handler ready.");

  rclcpp::spin(node);

  rclcpp::shutdown();

  close_fpga();
}
