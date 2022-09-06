/*
 * esp8266_functions.h
 *
 *  Created on: Aug 16, 2022
 *      Author: 42077
 */

#ifndef ESP8266_FUNCTIONS_H_
#define ESP8266_FUNCTIONS_H_

#include "esp8266_port.h"
#include "esp8266_utils.h"

uint16_t CmdESPConsoleATCmd(const uint8_t* const cmd, const uint16_t lng);
uint16_t CmdESPConsoleWrStr(const uint8_t* const str, const uint16_t lng);

uint32_t ESP_CheckRX(uint32_t timeOut,
				     uint32_t blockingTimeOut,
					 U32_pFn_pC_pC_U32_pC_pU32 processFn,
					 char * keyWord,
					 char **retStr,
					 uint32_t * retU32);

uint8_t* ESP_CheckResponse(char *pCmd, uint32_t cmdLng, uint32_t timeOut);


#endif /* ESP8266_FUNCTIONS_H_ */
