/*
 * app.h
 *
 *  Created on: Sep 20, 2022
 *      Author: 42077
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "eeprom_25aa1024.h"
#include "esp8266_http_server.h"
#include "bmp280.h"

#define CHECK_EEPROM		(uint8_t)(0)
#define CHECK_RADIO			(uint8_t)(1)
#define CHECK_WIFI			(uint8_t)(2)

#define CHECK_TEMPERATURE 	(uint8_t)(3)
#define CHECK_RADIO_OP		(uint8_t)(4)

void APP_ShortcutUSB(void);
void APP_ModuleCheckStates(void);
uint8_t APP_EEPROM_CheckIfOk(void);
void APP_ESP_InitConnect(void);
void APP_RDA5807M_RADIO_Init(void);
void APP_RDA5807M_RadioInit(void);
void APP_BMP280_SensorInit(void);
void APP_LEDC_DisplayInit(void);
void APP_RTC_GetHHMM(uint8_t *hh, uint8_t *mm);
void APP_RTC_Init(void);
#endif /* INC_APP_H_ */
