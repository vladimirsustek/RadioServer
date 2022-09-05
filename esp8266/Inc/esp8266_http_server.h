/*
 * esp8266_http_server.h
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */

#ifndef ESP8266_HTTP_SERVER_H_
#define ESP8266_HTTP_SERVER_H_


#include "main.h"

#include "esp8266_port.h"
#include "esp8266_http_data.h"
#include "esp8266_functions.h"

#include "cmd_rda5807m.h"

#include "eeprom_25aa1024.h"


#define MAX_HTTP_REQ_SIZE			(uint32_t)(128)

uint32_t ESP_HTTPinit(void);
uint32_t ESP_CheckReceiveHTTP(char **ppHTTPreq, uint32_t *pHTTPreqLng);
char* ESP_ProcessHTTP(char *pHTTPReq, uint32_t hhhtReqLng);

#endif /* ESP8266_HTTP_SERVER_H_ */
