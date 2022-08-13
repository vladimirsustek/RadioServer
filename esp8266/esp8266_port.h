/*
 * esp8266_port.h
 *
 *  Created on: Jul 19, 2022
 *      Author: 42077
 */

#ifndef ESP8266_PORT_H_
#define ESP8266_PORT_H_

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f1xx_hal.h"

#include "ledc_if.h"
#include "usbd_cdc_if.h"

#define ESP_COM_BUFF_LNG			(uint32_t)(1024u)

#define ESP_NEVER_VALUE		(uint32_t)(0xA5A5A5A5)

#define ESP_OK						(uint32_t)(0)
#define ESP_COMMAND_BUSY			(uint32_t)(-1)
#define ESP_RX_SILENT				(uint32_t)(-2)
#define ESP_INCORRECT_CRLF			(uint32_t)(-3)
#define ESP_RX_PENDING				(uint32_t)(-4)
#define ESP_TX_TIMEOUT				(uint32_t)(-5)
#define ESP_HARD_ERR				(uint32_t)(-6)
#define ESP_RSP_ERR					(uint32_t)(-7)

uint32_t ESP_ComInit();
uint32_t ESP_SendCommand(uint8_t* pStrCmd, uint32_t lng);
void ESP_CheckReceived();
#endif /* ESP8266_PORT_H_ */
