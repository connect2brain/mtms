/*
 * Generated with the FPGA Interface C API Generator 21.3
 * for NI-RIO 21.3 or later.
 */
#ifndef __NiFpga_mTMS_h__
#define __NiFpga_mTMS_h__

#ifndef NiFpga_Version
   #define NiFpga_Version 213
#endif

#include "NiFpga.h"

/**
 * The filename of the FPGA bitfile.
 *
 * This is a #define to allow for string literal concatenation. For example:
 *
 *    static const char* const Bitfile = "C:\\" NiFpga_mTMS_Bitfile;
 */
#define NiFpga_mTMS_Bitfile "NiFpga_mTMS.lvbitx"

/**
 * The signature of the FPGA bitfile.
 */
static const char* const NiFpga_mTMS_Signature = "C62CE2CD9787B6C15307A80377B33430";

#if NiFpga_Cpp
extern "C"
{
#endif

typedef enum
{
   NiFpga_mTMS_IndicatorBool_Devicestarted = 0x1804A,
   NiFpga_mTMS_IndicatorBool_Experimentstarted = 0x1804E
} NiFpga_mTMS_IndicatorBool;

typedef enum
{
   NiFpga_mTMS_IndicatorU8_Systemstate = 0x18022
} NiFpga_mTMS_IndicatorU8;

typedef enum
{
   NiFpga_mTMS_IndicatorU16_Channel1Capacitorvoltage = 0x1800E,
   NiFpga_mTMS_IndicatorU16_Channel1Temperature = 0x18032,
   NiFpga_mTMS_IndicatorU16_Channel2Capacitorvoltage = 0x18012,
   NiFpga_mTMS_IndicatorU16_Channel2Temperature = 0x18036,
   NiFpga_mTMS_IndicatorU16_Channel3Capacitorvoltage = 0x18016,
   NiFpga_mTMS_IndicatorU16_Channel3Temperature = 0x1803A,
   NiFpga_mTMS_IndicatorU16_Channel4Capacitorvoltage = 0x1801A,
   NiFpga_mTMS_IndicatorU16_Channel4Temperature = 0x1803E,
   NiFpga_mTMS_IndicatorU16_Channel5Capacitorvoltage = 0x1801E,
   NiFpga_mTMS_IndicatorU16_Channel5Temperature = 0x18042,
   NiFpga_mTMS_IndicatorU16_Startupsequenceerror = 0x18046
} NiFpga_mTMS_IndicatorU16;

typedef enum
{
   NiFpga_mTMS_IndicatorU64_time = 0x18024
} NiFpga_mTMS_IndicatorU64;

typedef enum
{
   NiFpga_mTMS_ControlBool_Eventtrigger = 0x18006,
   NiFpga_mTMS_ControlBool_Startdevice = 0x1802A,
   NiFpga_mTMS_ControlBool_Startexperiment = 0x18002,
   NiFpga_mTMS_ControlBool_Stopdevice = 0x1800A,
   NiFpga_mTMS_ControlBool_Stopexperiment = 0x1802E
} NiFpga_mTMS_ControlBool;

typedef enum
{
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostChargefeedbackFIFO = 3,
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostDischargefeedbackFIFO = 2,
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostSignalOutfeedbackFIFO = 1,
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostStimulationpulsefeedbackFIFO = 0
} NiFpga_mTMS_TargetToHostFifoU8;

typedef enum
{
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO = 7,
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO = 6,
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetSignalOutFIFO = 5,
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetStimulationpulseFIFO = 4
} NiFpga_mTMS_HostToTargetFifoU8;


#if NiFpga_Cpp
}
#endif

#endif
