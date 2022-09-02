/*
 * esp8266_utils.h
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */

#ifndef ESP8266_UTILS_H_
#define ESP8266_UTILS_H_

#include "stdint.h"
#include "string.h"
#include "esp8266_port.h"

#define IS_NUM(x)   ((x) >= '0' && (x) <= '9')

uint32_t ESP_CheckForKeyWord(char * key, char * buff, uint32_t buff_lng, char **ppRetStr, uint32_t *retVal);
char* IsESP_httpRequest(char* pStr, uint32_t strLng, uint32_t *pReqLng);
char* ESP_ExtractString(char* pKeyWord, char* pBUff, uint32_t buffLng, uint32_t *strLng);
uint32_t ESP_ExtractValue(char* pKeyWord, char* pBUff, uint32_t buffLng, uint32_t* val);


#endif /* ESP8266_UTILS_H_ */
