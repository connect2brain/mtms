/*
 * Generated with the FPGA Interface C API Generator 21.3
 * for NI-RIO 21.3 or later.
 */
#ifndef __NiFpga_board_control_h__
#define __NiFpga_board_control_h__

#ifndef NiFpga_Version
   #define NiFpga_Version 213
#endif

#include "NiFpga.h"

/**
 * The filename of the FPGA bitfile.
 *
 * This is a #define to allow for string literal concatenation. For example:
 *
 *    static const char* const Bitfile = "C:\\" NiFpga_board_control_Bitfile;
 */
#define NiFpga_board_control_Bitfile "NiFpga_board_control.lvbitx"

/**
 * The signature of the FPGA bitfile.
 */
static const char* const NiFpga_board_control_Signature = "CBC3968E6B4320CBE79B184C7B9DEF92";

#if NiFpga_Cpp
extern "C"
{
#endif

typedef enum
{
   NiFpga_board_control_IndicatorBool_Operationsuccessful = 0x1800A
} NiFpga_board_control_IndicatorBool;

typedef enum
{
   NiFpga_board_control_IndicatorU8_charger_interface__channel = 0x1801A,
   NiFpga_board_control_IndicatorU8_charger_interface__end_of_charge_status = 0x18026,
   NiFpga_board_control_IndicatorU8_charger_interface__version_major = 0x18076,
   NiFpga_board_control_IndicatorU8_charger_interface__version_minor = 0x1807A,
   NiFpga_board_control_IndicatorU8_charger_interface__version_patch = 0x1807E,
   NiFpga_board_control_IndicatorU8_safety_monitor__version_major = 0x1804E,
   NiFpga_board_control_IndicatorU8_safety_monitor__version_minor = 0x18052,
   NiFpga_board_control_IndicatorU8_safety_monitor__version_patch = 0x18056
} NiFpga_board_control_IndicatorU8;

typedef enum
{
   NiFpga_board_control_IndicatorU16_charger_interface__error = 0x18072,
   NiFpga_board_control_IndicatorU16_charger_interface__voltage = 0x18022,
   NiFpga_board_control_IndicatorU16_charger_interface__voltage_setpoint = 0x1801E,
   NiFpga_board_control_IndicatorU16_safety_monitor__cumulative_errors = 0x18066,
   NiFpga_board_control_IndicatorU16_safety_monitor__current_errors = 0x1806A,
   NiFpga_board_control_IndicatorU16_safety_monitor__emergency_errors = 0x1806E
} NiFpga_board_control_IndicatorU16;

typedef enum
{
   NiFpga_board_control_IndicatorU32_safety_monitor__n_startup_messages = 0x1803C,
   NiFpga_board_control_IndicatorU32_safety_monitor__n_status_messages = 0x18038
} NiFpga_board_control_IndicatorU32;

typedef enum
{
   NiFpga_board_control_ControlBool_signal__15_v_igbts = 0x18062,
   NiFpga_board_control_ControlBool_signal__15_v_others = 0x1805A,
   NiFpga_board_control_ControlBool_signal__24_v = 0x1805E,
   NiFpga_board_control_ControlBool_stop = 0x18002
} NiFpga_board_control_ControlBool;

typedef enum
{
   NiFpga_board_control_ControlU8_charger_interface__command = 0x1800E,
   NiFpga_board_control_ControlU8_charger_interface__set_channel = 0x18012
} NiFpga_board_control_ControlU8;

typedef enum
{
   NiFpga_board_control_ControlU16_charger_interface__set_voltage_setpoint = 0x18016
} NiFpga_board_control_ControlU16;

typedef enum
{
   NiFpga_board_control_ControlI32_Timeoutticks = 0x18004
} NiFpga_board_control_ControlI32;

typedef enum
{
   NiFpga_board_control_IndicatorArrayU8_discharge_controller__version_major = 0x18030,
   NiFpga_board_control_IndicatorArrayU8_discharge_controller__version_minor = 0x1802C,
   NiFpga_board_control_IndicatorArrayU8_discharge_controller__version_patch = 0x18028
} NiFpga_board_control_IndicatorArrayU8;

typedef enum
{
   NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_major = 5,
   NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_minor = 5,
   NiFpga_board_control_IndicatorArrayU8Size_discharge_controller__version_patch = 5
} NiFpga_board_control_IndicatorArrayU8Size;

typedef enum
{
   NiFpga_board_control_IndicatorArrayU16_discharge_controller__capacitor_voltage = 0x18034,
   NiFpga_board_control_IndicatorArrayU16_discharge_controller__errors = 0x18048
} NiFpga_board_control_IndicatorArrayU16;

typedef enum
{
   NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__capacitor_voltage = 5,
   NiFpga_board_control_IndicatorArrayU16Size_discharge_controller__errors = 5
} NiFpga_board_control_IndicatorArrayU16Size;

typedef enum
{
   NiFpga_board_control_IndicatorArrayU32_discharge_controller__n_startup_messages = 0x18040,
   NiFpga_board_control_IndicatorArrayU32_discharge_controller__n_status_messages = 0x18044
} NiFpga_board_control_IndicatorArrayU32;

typedef enum
{
   NiFpga_board_control_IndicatorArrayU32Size_discharge_controller__n_startup_messages = 5,
   NiFpga_board_control_IndicatorArrayU32Size_discharge_controller__n_status_messages = 5
} NiFpga_board_control_IndicatorArrayU32Size;


#if NiFpga_Cpp
}
#endif

#endif
