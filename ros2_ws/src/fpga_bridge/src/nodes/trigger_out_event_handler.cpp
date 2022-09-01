#include "rclcpp/rclcpp.hpp"

#include "fpga_interfaces/srv/send_trigger_out_event.hpp"
#include "fpga_interfaces/msg/trigger_out_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 trigger_out_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetSignalOutFIFO;

class TriggerOutEventHandler : public rclcpp::Node {
public:
  TriggerOutEventHandler()
      : Node("trigger_out_event_handler") {

    auto service_callback = [this](const std::shared_ptr<fpga_interfaces::srv::SendTriggerOutEvent::Request> request,
                                   std::shared_ptr<fpga_interfaces::srv::SendTriggerOutEvent::Response> response) -> void {
      fpga_interfaces::msg::TriggerOutEvent trigger_out_event = request->trigger_out_event;

      uint8_t index = trigger_out_event.index;

      fpga_interfaces::msg::EventInfo event_info = trigger_out_event.event_info;

      uint16_t event_id = event_info.event_id;
      uint8_t execution_condition = event_info.execution_condition;
      uint64_t time_us = event_info.time_us;

      serialized_message.init(index);
      serialized_message.add_uint16(event_id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_us);

      /* Serialize trigger out . */
      uint32_t duration_us = (uint8_t) trigger_out_event.duration_us;
      serialized_message.add_uint32(duration_us);

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
      RCLCPP_INFO(rclcpp::get_logger("trigger_out_event_handler"), "Sent trigger out request for index %d", index);

      response->success = true;
    };

    serialized_message = SerializedMessage();
    send_trigger_out_event_service_ = this->create_service<fpga_interfaces::srv::SendTriggerOutEvent>(
        "/fpga/send_trigger_out_event", service_callback);
  }

private:
  rclcpp::Service<fpga_interfaces::srv::SendTriggerOutEvent>::SharedPtr send_trigger_out_event_service_;
  SerializedMessage serialized_message;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

  auto node = std::make_shared<TriggerOutEventHandler>();

  RCLCPP_INFO(rclcpp::get_logger("trigger_out_event_handler"), "Trigger out event handler ready.");

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("trigger_out_event_handler"), "setting memory opt");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  rclcpp::spin(node);

  rclcpp::shutdown();

  close_fpga();
}
