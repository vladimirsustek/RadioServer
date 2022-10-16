/*
 * app_low.h
 *
 *  Created on: Oct 16, 2022
 *      Author: 42077
 */

#ifndef INC_APP_LOW_H_
#define INC_APP_LOW_H_

#include "eeprom_25aa1024.h"
#include "esp8266_http_server.h"
#include "bmp280.h"
#include "gpio.h"

void APP_ShortcutUSB(void);
uint8_t APP_EEPROM_CheckIfOk(void);

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
void APPP_NCoderStart(void);

uint8_t APP_CheckEEPROM(void);
uint8_t APP_CheckRadio(void);
void APP_CheckRadioOperation(char* message);
void APP_CheckTemperature(char* message);
void APP_CheckTime(char *message, RTC_TimeTypeDef* pRtc, uint8_t blinkDot);
uint8_t APP_CheckWIFI(void);

void APP_DisplayEspEnableSettings(char *message, uint8_t wifi, uint8_t blinkDot);
void APP_DisplayFrequencySetings(char *message, uint16_t freqeuncy, uint8_t blinkDot);
void APP_DisplayRDAEnableSettings(char *message, uint8_t mute);
void APP_DisplayVolumeSetings(char *message, uint16_t volume, uint8_t blinkDot);


#endif /* INC_APP_LOW_H_ */
