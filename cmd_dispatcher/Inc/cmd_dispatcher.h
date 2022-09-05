#ifndef CMD_DISPATCHER_H_INCLUDED
#define CMD_DISPATCHER_H_INCLUDED

#include "cmd_commands.h"
#include "cmd_defs.h"
#include "cmd_rda5807m.h"

#include "ledc_if.h"
#include "esp8266_functions.h"
#include "eeprom_25aa1024.h"

uint16_t CmdDispatch(const uint8_t* const pStrCmd, const uint16_t lng);

#endif // CMD_DISPATCHER_H_INCLUDED
