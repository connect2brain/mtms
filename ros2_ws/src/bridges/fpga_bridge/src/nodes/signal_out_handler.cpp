#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_signal_out.hpp"
#include "fpga_interfaces/msg/signal_out.hpp"
#include "fpga_interfaces/msg/event.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 signal_out_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetSignalOutFIFO;

class SignalOutHandler : public rclcpp::Node {
public:
  SignalOutHandler()
      : Node("signal_out_handler") {

    auto service_callback = [this](const std::shared_ptr<fpga_interfaces::srv::SendSignalOut::Request> request,
                                   std::shared_ptr<fpga_interfaces::srv::SendSignalOut::Response> response) -> void {
      fpga_interfaces::msg::SignalOut signal_out = request->signal_out;

      uint8_t port = signal_out.port;

      fpga_interfaces::msg::Event event = signal_out.event;

      uint16_t id = event.id;
      uint8_t execution_condition = event.execution_condition.value;
      uint64_t time_us = event.time_us;

      serialized_message.init(port);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_us);

      /* Serialize signal out. */

      /* TODO: Why is the type conversion to uint8_t here? */
      uint32_t duration_us = (uint8_t) signal_out.duration_us;
      serialized_message.add_uint32(duration_us);

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
    send_signal_out_service_ = this->create_service<fpga_interfaces::srv::SendSignalOut>(
        "/fpga/send_signal_out", service_callback);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendSignalOut>::SharedPtr send_signal_out_service_;
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
