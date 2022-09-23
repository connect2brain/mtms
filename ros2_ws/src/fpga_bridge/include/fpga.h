#ifndef FPGA_H
#define FPGA_H

#include "NiFpga.h"

#ifdef __unix__
#define ON_UNIX
#endif

bool init_fpga(void);
bool close_fpga(void);
bool run_fpga(void);

extern NiFpga_Session session;
extern NiFpga_Status status;
extern bool fpga_opened;

#endif
