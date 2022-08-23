//
// Created by alqio on 23.8.2022.
//

#ifndef FPGA_BRIDGE_SCHEDULING_UTILS_H
#define FPGA_BRIDGE_SCHEDULING_UTILS_H

#include <pthread.h>
#include <sys/types.h>
#include <sched.h>
#include <string>
#include <memory>
#include <thread>
#include <cstring>

#define DEFAULT_SCHEDULING_POLICY SCHED_RR
#define DEFAULT_SCHEDULING_PRIORITY 98

void set_thread_scheduling(std::thread::native_handle_type thread, int policy, int sched_priority);

#endif //FPGA_BRIDGE_SCHEDULING_UTILS_H
