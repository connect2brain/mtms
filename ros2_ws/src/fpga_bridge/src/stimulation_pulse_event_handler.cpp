#include <chrono>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include "fpga_interfaces/srv/send_stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_piece.hpp"
#include "fpga_interfaces/msg/stimulation_pulse_event.hpp"
#include "fpga_interfaces/msg/event_info.hpp"

#include "NiFpga_board_control.h"

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)
#define GET_BYTE(var,n) (uint8_t)((var) >> (8 * (n)))
#define MAX_SERIALIZED_MESSAGE_LENGTH 100

using namespace std::chrono_literals;
using std::placeholders::_1;

NiFpga_Session session;
NiFpga_Status status;
bool fpga_opened = false;

bool init_fpga(void) {
  /* must be called before any other calls */
  status = NiFpga_Initialize();
  if (NiFpga_IsError(status)) {
    RCLCPP_INFO(rclcpp::get_logger("stimulation_pulse_event_handler"), "FPGA could not be initialized, exiting.");
    return false;
  }

  /* opens a session, downloads the bitstream, and runs the FPGA */

  /* TODO: Remove hardcoded bitfile. */
  NiFpga_MergeStatus(&status, NiFpga_Open("C:\\Users\\mTMS\\mtms\\bitfiles\\NiFpga_board_control_0_1_3.lvbitx",
          NiFpga_board_control_Signature,
          "PXI1Slot4",
          NiFpga_OpenAttribute_NoRun,
          &session));

  if (NiFpga_IsError(status)) {
      RCLCPP_INFO(rclcpp::get_logger("stimulation_pulse_event_handler"), "FPGA bitfile could not be loaded, exiting.");
      return false;
  }

  fpga_opened = true;
  return true;
}

bool close_fpga(void) {
  if (fpga_opened) {
    /* must close if we successfully opened */
    NiFpga_MergeStatus(&status, NiFpga_Close(session, 0));
  }

  /* must be called after all other calls */
  NiFpga_MergeStatus(&status, NiFpga_Finalize());

  return true;
}

uint8_t serialized_message[MAX_SERIALIZED_MESSAGE_LENGTH] = {0};
uint8_t length = 0;

#define ESCAPE_CHARACTER 0x1B
#define TRANSPARENCY_MODIFIER 0x20
#define START_OF_MESSAGE 0xFE
#define END_OF_MESSAGE 0xFF

void init_serialized_message() {
  length = 0;
  serialized_message[length++] = START_OF_MESSAGE;
}

void finalize_serialized_message() {
  serialized_message[length++] = END_OF_MESSAGE;
}

void add_byte_to_serialized_message(uint8_t byte) {
  if (byte == START_OF_MESSAGE || byte == END_OF_MESSAGE) {
    serialized_message[length++] = ESCAPE_CHARACTER;
    serialized_message[length++] = byte^TRANSPARENCY_MODIFIER;
  } else {
    serialized_message[length++] = byte;
  }
}

void add_uint16_to_serialized_message(uint16_t value) {
  for (uint8_t i = 0; i < 2; i++) {
    add_byte_to_serialized_message(GET_BYTE(value, 1 - i));
  }
}

void add_uint32_to_serialized_message(uint32_t value) {
  for (uint8_t i = 0; i < 4; i++) {
    add_byte_to_serialized_message(GET_BYTE(value, 3 - i));
  }
}

void add_uint64_to_serialized_message(uint64_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    add_byte_to_serialized_message(GET_BYTE(value, 7 - i));
  }
}

const uint8_t channel_pulse_fifos[3] = {
  NiFpga_board_control_HostToTargetFifoU8_Channel1PulseFIFO,
  0,
  0
//  NiFpga_board_control_HostToTargetFifoU8_Channel2PulseFIFO,
//  NiFpga_board_control_HostToTargetFifoU8_Channel3PulseFIFO
};

void send_stimulation_pulse_event(const std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Request> request,
          std::shared_ptr<fpga_interfaces::srv::SendStimulationPulseEvent::Response> response)
{
  init_serialized_message();

  fpga_interfaces::msg::StimulationPulseEvent stimulation_pulse_event = request->stimulation_pulse_event;

  uint8_t channel = stimulation_pulse_event.channel;

  /* Serialize event info. */

  fpga_interfaces::msg::EventInfo event_info = stimulation_pulse_event.event_info;

  uint16_t event_id = event_info.event_id;
  uint8_t wait_for_trigger = event_info.wait_for_trigger;
  uint64_t time_us = event_info.time_us;

  add_uint16_to_serialized_message(event_id);
  add_byte_to_serialized_message(wait_for_trigger);
  add_uint64_to_serialized_message(time_us);

  /* Serialize stimulation pulse. */

  uint8_t n_pieces = (uint8_t) stimulation_pulse_event.pieces.size();
  add_byte_to_serialized_message(n_pieces);

  for (uint8_t i = 0; i < n_pieces; i++) {
    fpga_interfaces::msg::StimulationPulsePiece piece = stimulation_pulse_event.pieces[i];

    add_byte_to_serialized_message(piece.mode);
    add_uint16_to_serialized_message(piece.duration_in_ticks);
  }

  finalize_serialized_message();

  NiFpga_MergeStatus(&status,
    NiFpga_StartFifo(session,
                     channel_pulse_fifos[channel - 1]));

  NiFpga_MergeStatus(&status,
    NiFpga_WriteFifoU8(session,
                       channel_pulse_fifos[channel - 1],
                       serialized_message,
                       length,
                       NiFpga_InfiniteTimeout,
                       NULL));

  for (uint8_t i = 0; i < length; i++) {
    RCLCPP_INFO(rclcpp::get_logger("fpga"), "%d,  %d", i - 1, serialized_message[i]);
  }

  response->success = true;
}

class StimulationPulseEventHandler : public rclcpp::Node
{
  public:
    StimulationPulseEventHandler()
    : Node("stimulation_pulse_event_handler")
    {
      send_stimulation_pulse_event_service_ = this->create_service<fpga_interfaces::srv::SendStimulationPulseEvent>("/fpga/send_stimulation_pulse_event", send_stimulation_pulse_event);
    }

  private:
    rclcpp::Service<fpga_interfaces::srv::SendStimulationPulseEvent>::SharedPtr send_stimulation_pulse_event_service_;
};

int main(int argc, char **argv)
{
  if (!init_fpga())
  {
    return 1;
  }

  rclcpp::init(argc, argv);

  RCLCPP_INFO(rclcpp::get_logger("stimulation_pulse_event_handler"), "Stimulation pulse event handler ready.");

  rclcpp::spin(std::make_shared<StimulationPulseEventHandler>());
  rclcpp::shutdown();

  close_fpga();
}
