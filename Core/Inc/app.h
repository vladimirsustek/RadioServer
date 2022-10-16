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


#define APP_MESSAGE_LNG		(uint32_t)(32)

typedef enum {
	NO_SETTINGS_ONGOING = 0u,
	SETTING_FREQUENCY,
	SETTING_VOLUME,
	SETTING_MUTE,
	SETTING_TIME_HOURS,
	SETTING_TIME_MINUTES,
	SETTING_WIFI_ENABLE_DISABLE,
	PLATFORM_SETTINGS_SAVE} UserInput_t;

void APP_ShortcutUSB(void);
void APP_ModuleCheckStates(char *message, RTC_TimeTypeDef* pRTC);
uint8_t APP_EEPROM_CheckIfOk(void);
void APP_ESP_InitConnect(void);
void APP_RDA5807M_RADIO_Init(void);
void APP_RDA5807M_RadioInit(void);
void APP_BMP280_SensorInit(void);
void APP_LEDC_DisplayInit(void);
void APP_RTC_GetHHMM(uint8_t *hh, uint8_t *mm);
void APP_RTC_Init(void);
void APP_ShowFrequency(char *message, uint16_t freqeuncy, uint8_t blinkDot);
void APP_ShowVolume(char *message, uint16_t volume, uint8_t blinkDot);

uint8_t APP_ReadNCSwitchPulse(void);
uint8_t APP_ReadNCDirection(void);
uint8_t APP_UserInput(char *message, RTC_TimeTypeDef *pRTC);
uint8_t APP_ReadNCSwitch3SecPress(void);

void DBNC_NCDirection(void);
void DBNC_NCSwitch(void);
void DBNC_NCStart(void);
void DBNC_NCSwitch3SecPress(void);
void APP_CleanUPNcoder(void);
#endif /* INC_APP_H_ */
