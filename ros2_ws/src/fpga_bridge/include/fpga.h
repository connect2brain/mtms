#ifndef FPGA_H
#define FPGA_H

#include "NiFpga.h"

#ifdef __unix__
#define ON_UNIX
#endif

bool init_fpga();
bool close_fpga();
bool run_fpga();

extern NiFpga_Session session;
extern NiFpga_Status status;
extern bool fpga_opened;

#endif
