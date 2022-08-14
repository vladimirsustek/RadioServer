/*
 * esp8266_http_server.h
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */

#ifndef ESP8266_HTTP_SERVER_H_
#define ESP8266_HTTP_SERVER_H_

#include "esp8266_utils.h"

uint32_t ESP_httpInit (void);
uint32_t Server_Send (char *str, int Link_ID);

typedef struct
{
	char firstname[15];
	char lastname[15];
	char age[3];
}userDetails;

#endif /* ESP8266_HTTP_SERVER_H_ */
