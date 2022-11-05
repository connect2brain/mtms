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
static const char* const NiFpga_mTMS_Signature = "A2F5F35B30791F558C18A3EE3DD9F2F1";

#if NiFpga_Cpp
extern "C"
{
#endif

typedef enum
{
   NiFpga_mTMS_IndicatorU8_Channel1VersionmajorDC = 0x18056,
   NiFpga_mTMS_IndicatorU8_Channel1VersionminorDC = 0x1806A,
   NiFpga_mTMS_IndicatorU8_Channel1VersionpatchDC = 0x1807E,
   NiFpga_mTMS_IndicatorU8_Channel2VersionmajorDC = 0x1805A,
   NiFpga_mTMS_IndicatorU8_Channel2VersionminorDC = 0x1806E,
   NiFpga_mTMS_IndicatorU8_Channel2VersionpatchDC = 0x18082,
   NiFpga_mTMS_IndicatorU8_Channel3VersionmajorDC = 0x1805E,
   NiFpga_mTMS_IndicatorU8_Channel3VersionminorDC = 0x18072,
   NiFpga_mTMS_IndicatorU8_Channel3VersionpatchDC = 0x18086,
   NiFpga_mTMS_IndicatorU8_Channel4VersionmajorDC = 0x18062,
   NiFpga_mTMS_IndicatorU8_Channel4VersionminorDC = 0x18076,
   NiFpga_mTMS_IndicatorU8_Channel4VersionpatchDC = 0x1808A,
   NiFpga_mTMS_IndicatorU8_Channel5VersionmajorDC = 0x18066,
   NiFpga_mTMS_IndicatorU8_Channel5VersionminorDC = 0x1807A,
   NiFpga_mTMS_IndicatorU8_Channel5VersionpatchDC = 0x1808E,
   NiFpga_mTMS_IndicatorU8_Devicestate = 0x1801E,
   NiFpga_mTMS_IndicatorU8_Experimentstate = 0x180F2,
   NiFpga_mTMS_IndicatorU8_Startuperror = 0x18026,
   NiFpga_mTMS_IndicatorU8_Startupstate = 0x18106,
   NiFpga_mTMS_IndicatorU8_VersionmajorCI = 0x180CA,
   NiFpga_mTMS_IndicatorU8_VersionmajorFPGA = 0x180FA,
   NiFpga_mTMS_IndicatorU8_VersionmajorSB = 0x18092,
   NiFpga_mTMS_IndicatorU8_VersionmajorSM = 0x180BE,
   NiFpga_mTMS_IndicatorU8_VersionminorCI = 0x180CE,
   NiFpga_mTMS_IndicatorU8_VersionminorFPGA = 0x180FE,
   NiFpga_mTMS_IndicatorU8_VersionminorSB = 0x18096,
   NiFpga_mTMS_IndicatorU8_VersionminorSM = 0x180C2,
   NiFpga_mTMS_IndicatorU8_VersionpatchCI = 0x180D2,
   NiFpga_mTMS_IndicatorU8_VersionpatchFPGA = 0x18102,
   NiFpga_mTMS_IndicatorU8_VersionpatchSB = 0x1809A,
   NiFpga_mTMS_IndicatorU8_VersionpatchSM = 0x180C6
} NiFpga_mTMS_IndicatorU8;

typedef enum
{
   NiFpga_mTMS_IndicatorU16_Channel1Capacitorvoltage = 0x1800A,
   NiFpga_mTMS_IndicatorU16_Channel1Errors = 0x180AA,
   NiFpga_mTMS_IndicatorU16_Channel2Capacitorvoltage = 0x1800E,
   NiFpga_mTMS_IndicatorU16_Channel2Errors = 0x180AE,
   NiFpga_mTMS_IndicatorU16_Channel3Capacitorvoltage = 0x18012,
   NiFpga_mTMS_IndicatorU16_Channel3Errors = 0x180B2,
   NiFpga_mTMS_IndicatorU16_Channel4Capacitorvoltage = 0x18016,
   NiFpga_mTMS_IndicatorU16_Channel4Errors = 0x180B6,
   NiFpga_mTMS_IndicatorU16_Channel5Capacitorvoltage = 0x1801A,
   NiFpga_mTMS_IndicatorU16_Channel5Errors = 0x180BA,
   NiFpga_mTMS_IndicatorU16_Channel6Capacitorvoltage = 0x1810A,
   NiFpga_mTMS_IndicatorU16_Coil1Temperature = 0x18042,
   NiFpga_mTMS_IndicatorU16_Coil2Temperature = 0x18046,
   NiFpga_mTMS_IndicatorU16_Coil3Temperature = 0x1804A,
   NiFpga_mTMS_IndicatorU16_Coil4Temperature = 0x1804E,
   NiFpga_mTMS_IndicatorU16_Coil5Temperature = 0x18052,
   NiFpga_mTMS_IndicatorU16_Coil6Temperature = 0x18112,
   NiFpga_mTMS_IndicatorU16_Cumulativeerrors = 0x1809E,
   NiFpga_mTMS_IndicatorU16_Currenterrors = 0x180A2,
   NiFpga_mTMS_IndicatorU16_Emergencyerrors = 0x180A6
} NiFpga_mTMS_IndicatorU16;

typedef enum
{
   NiFpga_mTMS_IndicatorU32_Coil1Pulsecount = 0x1802C,
   NiFpga_mTMS_IndicatorU32_Coil2Pulsecount = 0x18030,
   NiFpga_mTMS_IndicatorU32_Coil3Pulsecount = 0x18034,
   NiFpga_mTMS_IndicatorU32_Coil4Pulsecount = 0x18038,
   NiFpga_mTMS_IndicatorU32_Coil5Pulsecount = 0x1803C,
   NiFpga_mTMS_IndicatorU32_Coil6Pulsecount = 0x1810C
} NiFpga_mTMS_IndicatorU32;

typedef enum
{
   NiFpga_mTMS_IndicatorU64_Time = 0x18020
} NiFpga_mTMS_IndicatorU64;

typedef enum
{
   NiFpga_mTMS_ControlBool_Disablechecks = 0x1802A,
   NiFpga_mTMS_ControlBool_Eventtrigger = 0x18006,
   NiFpga_mTMS_ControlBool_Startdevice = 0x180EA,
   NiFpga_mTMS_ControlBool_Startexperiment = 0x18002,
   NiFpga_mTMS_ControlBool_Stopdevice = 0x180EE,
   NiFpga_mTMS_ControlBool_Stopexperiment = 0x180E6
} NiFpga_mTMS_ControlBool;

typedef enum
{
   NiFpga_mTMS_ControlU8_Maximumpulsespertimepulses = 0x180DE
} NiFpga_mTMS_ControlU8;

typedef enum
{
   NiFpga_mTMS_ControlU16_Maximumpulsedurationticks = 0x180D6,
   NiFpga_mTMS_ControlU16_Maximumrisingfallingdifferenceticks = 0x180DA
} NiFpga_mTMS_ControlU16;

typedef enum
{
   NiFpga_mTMS_ControlU32_Experimentstarttriggerdurationticks = 0x180F4,
   NiFpga_mTMS_ControlU32_Maximumpulsespertimetimeticks = 0x180E0
} NiFpga_mTMS_ControlU32;

typedef enum
{
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostChargefeedbackFIFO = 3,
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostDischargefeedbackFIFO = 2,
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostPulsefeedbackFIFO = 1,
   NiFpga_mTMS_TargetToHostFifoU8_TargettoHostSignalOutfeedbackFIFO = 0
} NiFpga_mTMS_TargetToHostFifoU8;

typedef enum
{
   NiFpga_mTMS_TargetToHostFifoU64_DebugFIFO = 8
} NiFpga_mTMS_TargetToHostFifoU64;

typedef enum
{
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetChargeFIFO = 7,
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetDischargeFIFO = 6,
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetPulseFIFO = 5,
   NiFpga_mTMS_HostToTargetFifoU8_HosttoTargetSignalOutFIFO = 4
} NiFpga_mTMS_HostToTargetFifoU8;


#if NiFpga_Cpp
}
#endif

#endif
