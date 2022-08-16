/*
 * esp8266_http_data.h
 *
 *  Created on: Aug 16, 2022
 *      Author: 42077
 */

#ifndef ESP8266_HTTP_DATA_H_
#define ESP8266_HTTP_DATA_H_

#include "stdint.h"

char *pageIndex;

const char * atCmd ;
const char * atCmd_RST;
const char * atCmd_CWMODE;
const char * atCmd_CIPMUX;
#if STATIC_IP_AND_NEW_WIFI
const char * atCmd_CWSTAIP;
const char * atCmd_CWJAP;
#endif
const char * atCmd_CIPSERVER;
const char * atRsp_OK;
const char * atRsp_ready;
const char * atCmd_CIPSEND;
const char * atCmd_CIPCLOSE;

#endif /* ESP8266_HTTP_DATA_H_ */
