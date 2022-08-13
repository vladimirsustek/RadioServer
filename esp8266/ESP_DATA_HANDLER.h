/*
 * ESP_DATA_HANDLER.h
 *
 *  Created on: Aug 13, 2022
 *      Author: 42077
 */

#ifndef ESP_DATA_HANDLER_H_
#define ESP_DATA_HANDLER_H_

void ESP_Init (char *SSID, char *PASSWD, char *STAIP);
void Server_Start (void);

typedef struct
{
	char firstname[15];
	char lastname[15];
	char age[3];
}userDetails;

#endif /* ESP_DATA_HANDLER_H_ */
