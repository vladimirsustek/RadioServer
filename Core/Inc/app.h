/*
 * app.h
 *
 *  Created on: Sep 20, 2022
 *      Author: 42077
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "app_low.h"

#define APP_MESSAGE_LNG		(uint32_t)(32)

typedef enum {
	NO_SETTINGS_ONGOING = 0u,
	SETTING_FREQUENCY,
	SETTING_VOLUME,
	SETTING_RDA_ENABLE,
	SETTING_TIME_HOURS,
	SETTING_TIME_MINUTES,
	SETTING_ESP_ENABLE,
	PLATFORM_SETTINGS_SAVE} UserInput_t;


void APP_RDA5807M_RadioInit(void);
void APP_BMP280_SensorInit(void);
void APP_LEDC_DisplayInit(void);
void APP_ESP_InitConnect(void);

void APP_ModuleCheckStates(char *message, RTC_TimeTypeDef* pRTC);
#endif /* INC_APP_H_ */
