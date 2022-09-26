//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
//
// MatlabProcessor.cpp
//
// Code generation for function 'MatlabProcessor'
//

// Include files
#include "MatlabProcessor.h"
#include "Thresholding.h"
#include "create_command.h"
#include "rt_nonfinite.h"
#include "run_processor_internal_types.h"
#include "coder_array.h"
#include <stdio.h>

// Variable Definitions
static const matlab_fpga_event r{
    5U, // channel
    {
        0U,     // event_id
        2U,     // execution_condition
        69999UL // time_us
    },          // b_event_info
    {{
         0U,  // mode
         200U // duration_in_ticks
     },
     {
         2U,  // mode
         269U // duration_in_ticks
     },
     {
         1U, // mode
         1U  // duration_in_ticks
     }},     // pieces
    0U,      // event_type
    0U       // target_voltage
};

// Function Definitions
MatlabProcessor *MatlabProcessor::AbstractMatlabProcessor_init()
{
  MatlabProcessor *obj;
  int obj_idx_0;
  int obj_idx_1;
  obj = this;
  // ABSTRACTMATLABPROCESSOR Construct an instance of this
  // MatlabProcessor
  //    Initializes some variables and at the end calls
  //    MatlabProcessor.constructor
  obj->window_size = 50U;
  obj->channel_count = 62U;
  obj_idx_0 = obj->channel_count;
  obj_idx_1 = obj->window_size;
  obj->data.set_size(obj_idx_0, obj_idx_1);
  obj_idx_0 *= obj_idx_1;
  for (obj_idx_1 = 0; obj_idx_1 < obj_idx_0; obj_idx_1++) {
    obj->data[obj_idx_1] = 0.0;
  }
  obj->experiment_in_progress = false;
  obj->last_sample_received_at_us = 0UL;
  obj->auto_enqueue = false;
  //  Tells the compiler to make commands a variable sized list
  obj_idx_0 = 5;
  if (obj->window_size < 5) {
    obj_idx_0 = 20;
  }
  obj->commands.set_size(obj_idx_0);
  for (obj_idx_1 = 0; obj_idx_1 < obj_idx_0; obj_idx_1++) {
    obj->commands[obj_idx_1] = r;
  }
  obj->events_sent = 0U;
  obj->constructor(&obj->_pobj0);
  return obj;
}

void MatlabProcessor::constructor(Thresholding *iobj_0)
{
  coder::array<double, 2U> b_r;
  int i;
  int loop_ub;
  printf("%s\n", "in ConcreteMatlabProcessor constructor");
  fflush(stdout);
  set_channel_count();
  set_window_size();
  set_auto_enqueue();
  i = window_size;
  b_r.set_size(i, i);
  loop_ub = i * i;
  for (i = 0; i < loop_ub; i++) {
    b_r[i] = 0.0;
  }
  peak_detection = iobj_0->init(b_r, window_size);
}

void MatlabProcessor::enqueue(const double element_data[], int element_size)
{
  int q0;
  unsigned int qY;
  q0 = window_size;
  qY = q0 - 1U;
  if (q0 - 1U > static_cast<unsigned int>(q0)) {
    qY = 0U;
  }
  q0 = static_cast<int>(qY);
  for (int i{0}; i < q0; i++) {
    data[i] = data[i + 1];
  }
  q0 = data.size(1) - 1;
  for (int i{0}; i < element_size; i++) {
    data[i + data.size(0) * q0] = element_data[i];
  }
}

void MatlabProcessor::on_data_received()
{
  matlab_fpga_event rv[2];
  matlab_fpga_event b_r;
  matlab_fpga_event r1;
  unsigned int q0_tmp;
  unsigned int qY;
  peak_detection->thresholding_algo();
  q0_tmp = events_sent;
  qY = q0_tmp + 1U;
  if (q0_tmp + 1U < q0_tmp) {
    qY = MAX_uint32_T;
  }
  create_command(qY, &b_r.channel, &b_r.b_event_info.event_id,
                 &b_r.b_event_info.execution_condition,
                 &b_r.b_event_info.time_us, b_r.pieces, &b_r.event_type,
                 &b_r.target_voltage);
  qY = q0_tmp + 2U;
  if (q0_tmp + 2U < q0_tmp) {
    qY = MAX_uint32_T;
  }
  b_create_command(qY, &r1.channel, &r1.b_event_info.event_id,
                   &r1.b_event_info.execution_condition,
                   &r1.b_event_info.time_us, r1.pieces, &r1.event_type,
                   &r1.target_voltage);
  rv[0] = b_r;
  rv[1] = r1;
  set_commands(rv);
}

void MatlabProcessor::on_init_experiment()
{
  commands.set_size(0);
}

void MatlabProcessor::set_auto_enqueue()
{
  // set_channel_count Set auto enqueue
  //    Sets whether new data samples should be auto enqueued into
  //    data variable
  auto_enqueue = true;
}

void MatlabProcessor::set_channel_count()
{
  int obj_idx_0;
  int obj_idx_1;
  // set_channel_count Set channel count
  //    Resets data to zeros as its dimensions change
  channel_count = 63U;
  obj_idx_0 = channel_count;
  obj_idx_1 = window_size;
  data.set_size(obj_idx_0, obj_idx_1);
  obj_idx_0 *= obj_idx_1;
  for (obj_idx_1 = 0; obj_idx_1 < obj_idx_0; obj_idx_1++) {
    data[obj_idx_1] = 0.0;
  }
}

void MatlabProcessor::set_commands(const matlab_fpga_event b_commands[2])
{
  commands.set_size(2);
  commands[0] = r;
  commands[0] = b_commands[0];
  commands[1] = r;
  commands[1] = b_commands[1];
}

void MatlabProcessor::set_window_size()
{
  int obj_idx_0;
  int obj_idx_1;
  // set_window_size Set window size
  //    Resets data to zeros as its dimensions change
  window_size = 50U;
  obj_idx_0 = channel_count;
  obj_idx_1 = window_size;
  data.set_size(obj_idx_0, obj_idx_1);
  obj_idx_0 *= obj_idx_1;
  for (obj_idx_1 = 0; obj_idx_1 < obj_idx_0; obj_idx_1++) {
    data[obj_idx_1] = 0.0;
  }
}

MatlabProcessor *MatlabProcessor::AbstractMatlabProcessor_setData()
{
  MatlabProcessor *obj;
  obj = this;
  obj->data.set_size(1, 10);
  for (int i{0}; i < 10; i++) {
    obj->data[i] = static_cast<double>(i) + 1.0;
  }
  return obj;
}

void MatlabProcessor::data_received(const double channel_data_data[],
                                    int channel_data_size,
                                    unsigned long time_us,
                                    boolean_T first_sample_of_experiment,
                                    coder::array<matlab_fpga_event, 1U> &ret)
{
  int loop_ub;
  unsigned int q0;
  unsigned int qY;
  if (first_sample_of_experiment) {
    experiment_in_progress = true;
  }
  if (auto_enqueue) {
    enqueue(channel_data_data, channel_data_size);
  }
  on_data_received();
  q0 = events_sent;
  qY = q0 + 1U;
  if (q0 + 1U < q0) {
    qY = MAX_uint32_T;
  }
  events_sent = qY;
  last_sample_received_at_us = time_us;
  ret.set_size(commands.size(0));
  loop_ub = commands.size(0);
  for (int i{0}; i < loop_ub; i++) {
    ret[i] = commands[i];
  }
}

void MatlabProcessor::end_experiment(coder::array<matlab_fpga_event, 1U> &ret)
{
  int loop_ub;
  experiment_in_progress = false;
  on_init_experiment();
  ret.set_size(commands.size(0));
  loop_ub = commands.size(0);
  for (int i{0}; i < loop_ub; i++) {
    ret[i] = commands[i];
  }
}

void MatlabProcessor::getData(coder::array<double, 2U> &val) const
{
  int loop_ub;
  val.set_size(data.size(0), data.size(1));
  loop_ub = data.size(0) * data.size(1);
  for (int i{0}; i < loop_ub; i++) {
    val[i] = data[i];
  }
}

MatlabProcessor *MatlabProcessor::init()
{
  MatlabProcessor *obj;
  obj = this;
  return obj->AbstractMatlabProcessor_init();
}

void MatlabProcessor::init_experiment(coder::array<matlab_fpga_event, 1U> &ret)
{
  int loop_ub;
  on_init_experiment();
  ret.set_size(commands.size(0));
  loop_ub = commands.size(0);
  for (int i{0}; i < loop_ub; i++) {
    ret[i] = commands[i];
  }
}

MatlabProcessor *MatlabProcessor::setData(const coder::array<double, 2U> &val)
{
  MatlabProcessor *obj;
  int loop_ub;
  obj = this;
  obj->data.set_size(1, val.size(1));
  loop_ub = val.size(1);
  for (int i{0}; i < loop_ub; i++) {
    obj->data[i] = val[i];
  }
  return obj;
}

MatlabProcessor *MatlabProcessor::setData()
{
  MatlabProcessor *obj;
  obj = this;
  obj->data.set_size(0, 0);
  return obj;
}

void MatlabProcessor::set_channel_count(unsigned short new_channel_count)
{
  int obj_idx_0;
  int obj_idx_1;
  // set_channel_count Set channel count
  //    Resets data to zeros as its dimensions change
  channel_count = new_channel_count;
  obj_idx_0 = channel_count;
  obj_idx_1 = window_size;
  data.set_size(obj_idx_0, obj_idx_1);
  obj_idx_0 *= obj_idx_1;
  for (obj_idx_1 = 0; obj_idx_1 < obj_idx_0; obj_idx_1++) {
    data[obj_idx_1] = 0.0;
  }
}

void MatlabProcessor::set_window_size(unsigned int new_window_size)
{
  int obj_idx_0;
  int obj_idx_1;
  unsigned int u;
  // set_window_size Set window size
  //    Resets data to zeros as its dimensions change
  u = new_window_size;
  if (new_window_size > 65535U) {
    u = 65535U;
  }
  window_size = static_cast<unsigned short>(u);
  obj_idx_0 = channel_count;
  obj_idx_1 = window_size;
  data.set_size(obj_idx_0, obj_idx_1);
  obj_idx_0 *= obj_idx_1;
  for (obj_idx_1 = 0; obj_idx_1 < obj_idx_0; obj_idx_1++) {
    data[obj_idx_1] = 0.0;
  }
}

// End of code generation (MatlabProcessor.cpp)
