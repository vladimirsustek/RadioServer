#ifndef CMD_RDA5807M_H_INCLUDED
#define CMD_RDA5807M_H_INCLUDED

#include "../rda5807m/rda5807m.h"
#include "cmd_defs.h"

uint16_t CmdRDA5807mDoInit(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mDoReset(const uint8_t* const pStrCmd, const uint8_t lng);

uint16_t CmdRDA5807mSetFreq(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mSetVolm(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mSetMute(const uint8_t* const pStrCmd, const uint8_t lng);

uint16_t CmdRDA5807mGetRSSI(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetRDSR(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetRDSS(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetSTRO(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetCHST(const uint8_t* const pStrCmd, const uint8_t lng);

uint16_t CmdRDA5807mGetBLKA(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetBLKB(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetBLKC(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetBLKD(const uint8_t* const pStrCmd, const uint8_t lng);

uint16_t CmdRDA5807mGetERRA(const uint8_t* const pStrCmd, const uint8_t lng);
uint16_t CmdRDA5807mGetERRB(const uint8_t* const pStrCmd, const uint8_t lng);

#endif // CMD_RDA5807M_H_INCLUDED
