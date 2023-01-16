#include "rclcpp/rclcpp.hpp"

#include "event_interfaces/srv/send_discharge.hpp"
#include "event_interfaces/msg/discharge.hpp"
#include "event_interfaces/msg/event.hpp"

#include "NiFpga_mTMS.h"
#include "fpga.h"
#include "serdes.h"
#include "memory_utils.h"
#include "scheduling_utils.h"

const NiFpga_mTMS_HostToTargetFifoU8 discharge_fifo = NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO;

const uint32_t CLOCK_FREQUENCY_HZ = 4e7;

class DischargeHandler : public rclcpp::Node {
public:
  DischargeHandler()
      : Node("discharge_handler") {

    auto service_callback = [this](const std::shared_ptr<event_interfaces::srv::SendDischarge::Request> request,
                                   std::shared_ptr<event_interfaces::srv::SendDischarge::Response> response) -> void {
      event_interfaces::msg::Discharge discharge = request->discharge;

      uint8_t channel = discharge.channel;

      /* Serialize event info. */

      event_interfaces::msg::Event event = discharge.event;

      uint16_t id = event.id;
      uint8_t execution_condition = event.execution_condition.value;
      double_t time = event.time;
      uint64_t time_ticks = (uint64_t)(time * CLOCK_FREQUENCY_HZ);

      serialized_message.init(channel);
      serialized_message.add_uint16(id);
      serialized_message.add_byte(execution_condition);
      serialized_message.add_uint64(time_ticks);

      /* Serialize discharge. */

      uint16_t target_voltage = discharge.target_voltage;
      serialized_message.add_uint16(target_voltage);

      serialized_message.finalize();

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

      RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Sent discharge to channel %d", channel);

      response->success = true;
    };

    serialized_message = SerializedMessage();
    send_discharge_service_ = this->create_service<event_interfaces::srv::SendDischarge>(
        "/event/send_discharge", service_callback);
  }

private:
  rclcpp::Service<event_interfaces::srv::SendDischarge>::SharedPtr send_discharge_service_;
  SerializedMessage serialized_message;
};

int main(int argc, char **argv) {
  if (!init_fpga()) {
    return 1;
  }

  rclcpp::init(argc, argv);

#if defined(ON_UNIX) && defined(SCHEDULING_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Setting thread scheduling");
  set_thread_scheduling(pthread_self(), DEFAULT_SCHEDULING_POLICY, DEFAULT_REALTIME_SCHEDULING_PRIORITY);
#endif

  auto node = std::make_shared<DischargeHandler>();

#if defined(ON_UNIX) && defined(MEMORY_OPTIMIZATION)
  RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Locking memory");
  lock_memory();
  preallocate_memory(1024 * 1024 * 10); //10 MB
#endif

  RCLCPP_INFO(rclcpp::get_logger("discharge_handler"), "Discharge handler ready.");

  rclcpp::spin(node);
  rclcpp::shutdown();

  close_fpga();
}
