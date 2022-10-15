/*
 * app.c
 *
 *  Created on: Sep 20, 2022
 *      Author: 42077
 */


#include "app.h"
#include "gpio.h"

static uint8_t displayFSMstate = 0;

static uint8_t btnSetFlag = 0;
static uint8_t btn3SecPressFlag = 0;
static uint8_t ncDirection = 0;
static uint8_t clkStateprev = 0;
static uint8_t dtStateprev = 0;

void APP_ShortcutUSB(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOA_CLK_ENABLE();

	  /*Configure GPIO pin Output Level Low */
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

	  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_11;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  PLATFORM_DELAY_MS(2000);

	  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12|GPIO_PIN_11);

	  __HAL_RCC_GPIOA_CLK_DISABLE();
}

uint8_t APP_CheckEEPROM(void)
{
	uint8_t anyFault = 0;
	if(!APP_EEPROM_CheckIfOk())
	{
		  // ensure the string is displayed
	    //while(LEDC_GetRollingStatus()) { }
		anyFault = 1;
	}
	else
	{
		anyFault = 0;
	}
	return anyFault;
}

uint8_t APP_CheckRadio(void)
{
	return;
	/* TODO Add mechanism to check operation*/
	uint8_t anyFault = 0;
	// Receiver signal strength is always > 0
	if(0 == RDA5807mGetRSSI() && 0 == systemGlobalState.states.rdaIsMute)
	{
		// inform, save the state and try to re-init
		systemGlobalState.states.rdaFunctional = 0;
		  // ensure the string is displayed
	    //while(LEDC_GetRollingStatus()) { }
		anyFault = 1;
		RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);
	}
	else
	{
		anyFault = 0;
		systemGlobalState.states.rdaFunctional = 1;
	}
	return anyFault;
}

uint8_t APP_CheckWIFI(void)
{
	uint8_t anyFault = 0;
	// 1) Are we connected ? (AT+CWJAP?)
	// 2) Are we providing a server ? (AT+CIPMUX?)


	ESP_SendCommand("AT+CWJAP?\r\n", strlen("AT+CWJAP?\r\n"));
	// either +CWJAP:"SSID","MAC address" ..  OK
	// or No AP OK
	if(NULL == ESP_CheckResponse("+CWJAP", strlen("+CWJAP"), ESP_TIMEOUT_300ms))
	{
		systemGlobalState.states.espConnected = 0;
		  // ensure the string is displayed
	    //while(LEDC_GetRollingStatus()) { }
		LEDC_SetNewRollingString("offline", strlen("offline"));
		// Try to re-init
		APP_ESP_InitConnect();
	}

	ESP_SendCommand("AT+CIPMUX?\r\n", strlen("AT+CIPMUX?\r\n"));

	if(NULL == ESP_CheckResponse("+CIPMUX:1", strlen("+CIPMUX:1"), ESP_TIMEOUT_300ms))
	{
		systemGlobalState.states.espConnected = 0;
		  // ensure the string is displayed
	    //while(LEDC_GetRollingStatus()) { }
		LEDC_SetNewRollingString("offline", strlen("offline"));

		// Try to re-init
		BluePill_SetBlinkState(PERMANENT_BLINK);
		APP_ESP_InitConnect();
		BluePill_SetBlinkState(NO_BLINK);
	}
	else
	{
		// Shows server's IP address where can client connect
		// +CIFSR:STAIP,"192.yyy.0.108"
		// +CIFSR:STAMAC,"xx:3a:e8:0b:76:53"
		ESP_SendCommand("AT+CIFSR\r\n", strlen("AT+CIFSR\r\n"));
		uint8_t* ipStr = ESP_CheckResponse("+CIFSR:STAIP,", strlen("+CIFSR:STAIP,"), ESP_TIMEOUT_300ms) + strlen("+CIFSR:STAIP,");

		if(ipStr)
		{
			uint8_t idx = 0;
			uint8_t quotationMarkCnt = 0;
			for(idx = 0; idx < 17; idx++)
			{

				if('"' == ipStr[idx])
				{
					quotationMarkCnt++;
				}
				if(quotationMarkCnt == 2)
				{
					break;
				}
			}
			if(2 == quotationMarkCnt)
			{

				char ipAdr[15] = {0};
				memcpy(ipAdr, ipStr + 1, idx -1);
				// ensure the string is displayed
				LEDC_SetNewRollingString(ipAdr, idx -1);
			}
		}
	}
	return anyFault;
}

void APP_CheckRadioOperation(char* message)
{
	if(systemGlobalState.states.rdaFunctional && !systemGlobalState.states.rdaIsMute)
	{
		sprintf(message, "frequency %d.%d khz volume %d",
				systemGlobalState.radioFreq / 100,
				systemGlobalState.radioFreq % 100,
				systemGlobalState.radioVolm);
		  // ensure the string is displayed
		LEDC_SetNewRollingString(message, strlen(message));
	}
}

void APP_CheckTemperature(char* message)
{
    int32_t temp, press;
	bmp280_get_temp_press(&temp, &press);

	if (temp > 0)
	{
		sprintf(message, "%02ld*C", temp/100);
	}
	else
	{
		temp = (temp < -9) ? -9 : temp;
		sprintf(message, "%ld*C", temp/100);
	}
	LEDC_SetStandingDot(0);
	LEDC_SetNewStandingText(message);
}

void APP_CheckTime(char *message, RTC_TimeTypeDef* pRtc, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "%02d%02d", pRtc->Hours, pRtc->Minutes);
	if (blinkDot)
	{
		LEDC_SetStandingDot(2);
	}
	LEDC_SetNewStandingText(message);
}

void APP_CheckWifiEnable(char *message, uint8_t wifi, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "ESP%d", wifi);
	if (blinkDot)
	{
		LEDC_SetStandingDot(3);
	}
	LEDC_SetNewStandingText(message);
}

void APP_ShowFrequency(char *message, uint16_t freqeuncy, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "%03d%01d", freqeuncy/100, freqeuncy%100);
	if (blinkDot)
	{
		LEDC_SetStandingDot(3);
	}
	LEDC_SetNewStandingText(message);
}

void APP_ShowVolume(char *message, uint16_t volume, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "VL%02d", volume);
	if (blinkDot)
	{
		LEDC_SetStandingDot(2);
	}
	LEDC_SetNewStandingText(message);
}
void APP_ShowMute(char *message, uint8_t mute)
{
	LEDC_SetStandingDot(0);

	if (mute)
	{
		sprintf(message, "RDA1");
	}
	else
	{
		sprintf(message, "RDA0");
	}
	LEDC_SetNewStandingText(message);
}
void APP_ModuleCheckStates(char *message, RTC_TimeTypeDef* pRTC)
{
	static uint8_t prevSeconds = 0;
	static uint8_t timeElapsed = 0;
	const uint8_t pernamentDot = 1;

	HAL_RTC_GetTime(&hrtc, pRTC, RTC_FORMAT_BIN);

	// FSM is triggered each 9th second (9, 19, 29, 39, 49, 59th second)
	// and also no rolling string may be in progress
	// FSM has 12th periodic states, each lasts 10 seconds
	// Note that case 6, 10 and especially 11 due to rolling text may
	// last a bit longer

	// 00 APP_CheckTime (Display time HH.MM)
	// 01 APP_CheckTemperature (Display temperature XX°C)
	// 02 APP_CheckTime (Display time HH.MM)
	// 03 APP_CheckTemperature (Display temperature XX°C)
	// 04 APP_CheckTime (Display time HH.MM)
	// 05 APP_CheckTemperature (Display temperature XX°C)
	// 06 APP_CheckRadioOperation (Display "Frequency XXX.YY volume ZZ")
	//    when radio operates otherwise APP_CheckTime (Display time HH.MM)
	// 07 APP_CheckTemperature (Display temperature XX°C)
	// 08 APP_CheckTime (Display time HH.MM)
	// 09 APP_CheckTemperature (Display temperature XX°C)
	// 10 APP_CheckWIFI (Displays IP e.g. 123.123.123.123 or "offline")
	// 11 APP_CheckEEPROM and APP_CheckRadio (RDA, ROM ERR)
	//    otherwise APP_CheckTime (Display time HH.MM)

	// check whether 10 seconds has elapsed and the "has elapsed" flag was not used
	timeElapsed = (!timeElapsed && (pRTC->Seconds != prevSeconds) && (9 == (pRTC->Seconds % 10))) ? 1 : 0;

	// check whether flag is set and also LEDC is available (could happen time elapsed but LEDC was busy)
	if(timeElapsed && !LEDC_GetRollingStatus())
	{
		timeElapsed = 0;
		prevSeconds = pRTC->Seconds;
		memset(message, '\0', APP_MESSAGE_LNG);

		switch(displayFSMstate)
		{
		case 0:
		case 2:
		case 4:
		case 8:
		{
			APP_CheckTime(message, pRTC, pernamentDot);
		}
		break;
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
		{
			APP_CheckTemperature(message);
		}
		break;
		case 6 :
		{

			if (0 == systemGlobalState.states.rdaIsMute)
			{
				LEDC_StopStandingText();
				APP_CheckRadioOperation(message);
			}
			else
			{

				APP_CheckTime(message, pRTC, pernamentDot);
			}
		}
		break;
		case 10 :
		{
			if (systemGlobalState.states.wifiEnabled)
			{
				LEDC_StopStandingText();
				APP_CheckWIFI();
			}
		}
		break;
		case 11 :
		{
			uint8_t ee_err = APP_CheckEEPROM();
			uint8_t rda_err = APP_CheckRadio();
			LEDC_StopStandingText();
			if (ee_err && !rda_err) sprintf(message, "ROM ERR");
			if (!ee_err && rda_err) sprintf(message, "RDA ERR");
			if (ee_err && rda_err) sprintf(message, "ROM RDA ERR");
			if (!ee_err && !rda_err) APP_CheckTime(message, pRTC, pernamentDot);
		}
		break;
		default :
		{
			displayFSMstate = 0;
		}

		}

		displayFSMstate = (displayFSMstate + 1) % 12;
	}
}


uint8_t APP_EEPROM_CheckIfOk(void)
{
	/* not needed to store into eeprom that the eeprom works*/
	EEPROM_GetSystemState();
	if (0x5 == systemGlobalState.states.dummy0x5)
	{
		systemGlobalState.states.eepromFunctional = 1;
	}
	else
	{
		systemGlobalState.states.eepromFunctional = 0;
		LEDC_SetNewRollingString("EEPROM fault", strlen("EEPROM fault"));
	}
	return systemGlobalState.states.eepromFunctional;
}

void APP_ESP_InitConnect(void)
{
	  // ensure the string is displayed
      while(LEDC_GetRollingStatus()) { }
	  LEDC_SetNewInfiniteRollingString("Connecting");

	  if(ESP_RET_OK != ESP_HTTPinit())
	  {
		  // Stop string Connecting and ensure the second string is displayed
		  LEDC_StopInfiniteRollingString();
		  while(LEDC_GetRollingStatus());
		  LEDC_SetNewRollingString("ESP Init fault", strlen("ESP Init fault"));
	  }
	  else
	  {
		  // Stop string Connecting and ensure the second string is displayed
		  LEDC_StopInfiniteRollingString();
		  while(LEDC_GetRollingStatus());
		  LEDC_SetNewRollingString("ESP Connected", strlen("ESP Connected"));
	  }

}

void APP_RDA5807M_RadioInit(void)
{
	RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);
}

void APP_BMP280_SensorInit(void)
{
	BMP280_SensorInit();
}

void APP_LEDC_DisplayInit(void)
{
	LEDC_InitHW();
}

void APP_RTC_Init(void)
{
	// By the STM32F103 nature RTC does not need to be started
}

uint8_t APP_ReadNCSwitchPulse(void)
{
	uint8_t result = 0;
	if (1 == btnSetFlag && HAL_GPIO_ReadPin(NCODER_BTN_GPIO_Port, NCODER_BTN_Pin))
	{
		result = btnSetFlag;
		btnSetFlag = 0;
	}
	return result;
}

// Must be called periodically cca. 10-n*10x per second
void DBNC_NCSwitch(void)
{
	static uint8_t btnCnt = 0;
	const uint8_t debouncLevel = 10;

	if(!HAL_GPIO_ReadPin(NCODER_BTN_GPIO_Port, NCODER_BTN_Pin))
	{
		// Must be calculated
		if (btnCnt < debouncLevel)
		{
			btnCnt++;
		}
		else
		{
			btnSetFlag = 1;
		}
	}
	else
	{
		if(btnCnt)
		{
			btnCnt--;
		}
		else
		{
			btnSetFlag = 0;
		}
	}
}

// Necessary initialization, otherwise DBNC_NCSwitch triggers a direction change
void DBNC_NCStart(void)
{
	clkStateprev = HAL_GPIO_ReadPin(NCODER_CLK_GPIO_Port, NCODER_CLK_Pin);
	dtStateprev = HAL_GPIO_ReadPin(NCODER_DT_GPIO_Port, NCODER_DT_Pin);
}

void DBNC_NCDirection(void)
{
	static uint8_t clkChangedAtTime = 0;
	static uint8_t dtChangedAtTime = 0;
	static uint8_t internTick = 1; // can't be initialized as 0 !

	uint8_t clkStatenow, dtStatenow;

	if((clkStatenow = (uint8_t)HAL_GPIO_ReadPin(NCODER_CLK_GPIO_Port, NCODER_CLK_Pin)) != clkStateprev)
	{
		clkStateprev = clkStatenow;
		clkChangedAtTime = internTick;
	}

	internTick = (0 == ((internTick + 1) & (256 - 1))) ? 1 : ((internTick + 1) & (256 - 1));

	if((dtStatenow = (uint8_t)HAL_GPIO_ReadPin(NCODER_DT_GPIO_Port, NCODER_DT_Pin)) != dtStateprev)
	{
		dtStateprev = dtStatenow;
		dtChangedAtTime = internTick;
	}

	if (clkChangedAtTime && dtChangedAtTime)
	{
		ncDirection = (clkChangedAtTime > dtChangedAtTime) ? (uint8_t)-1 : 1;
		clkChangedAtTime = dtChangedAtTime = 0;
	}
}

uint8_t APP_ReadNCDirection(void)
{
	uint8_t result = 0;
	if (0 != ncDirection)
	{
		result = ncDirection;
		ncDirection = 0;
	}
	return result;
}

uint8_t APP_UserInput(char *message, RTC_TimeTypeDef *pRTC)
{
#warning bypassed here
	return 0;

	uint8_t direction = 0;
	const uint8_t blinkDotPeriod = 100;
	static uint32_t timeOut = 0;
	static uint8_t blinkDot = 0;
	static uint8_t wifi;
	static UserInput_t userInputFSM = NO_SETTINGS_ONGOING;

	switch(userInputFSM)
	{
	case NO_SETTINGS_ONGOING:
	{
		if (APP_ReadNCSwitch3SecPress())
		{
			userInputFSM = SETTING_TIME_HOURS;
			APP_ReadNCDirection();
		}
		else
		{
			if (APP_ReadNCSwitchPulse())
			{
				userInputFSM = SETTING_FREQUENCY;
				APP_ReadNCDirection();
			}
		}

	}
	break;
	case SETTING_FREQUENCY:
	{
		if(APP_ReadNCSwitchPulse())
		{
			userInputFSM = SETTING_VOLUME;
			APP_ReadNCDirection();
		}
		else
		{
			if (0 != (direction = APP_ReadNCDirection()))
			{
				if (1 == direction)
				{
					if (systemGlobalState.radioFreq < RDA5807mWW_FREQ_MAX)
					{
						systemGlobalState.radioFreq += 10;
					}
				}
				else
				{
					if (systemGlobalState.radioFreq > RDA5807mWW_FREQ_MIN)
					{
						systemGlobalState.radioFreq -= 10;
					}
				}
				// set volume and save changes to EEPROM
				RDA5807mSetVolm(systemGlobalState.radioFreq);
				EEPROM_SetSystemState();
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_ShowFrequency(message, systemGlobalState.radioFreq, blinkDot);
				blinkDot ^= 0x01;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case SETTING_VOLUME:
	{
		if(APP_ReadNCSwitchPulse())
		{
			userInputFSM = SETTING_MUTE;
			APP_ReadNCDirection();
		}
		else
		{
			if (0 != (direction = APP_ReadNCDirection()))
			{
				if (1 == direction)
				{
					if (systemGlobalState.radioVolm < RDA5807mVOLUME_MAX)
					{
						systemGlobalState.radioVolm++;
					}
				}
				else
				{
					if (systemGlobalState.radioVolm > RDA5807mVOLUME_MIN)
					{
						systemGlobalState.radioVolm--;
					}
				}
				// set volume and save changes to EEPROM
				RDA5807mSetVolm(systemGlobalState.radioVolm);
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_ShowVolume(message, systemGlobalState.radioVolm, blinkDot);
				blinkDot ^= 0x80;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case SETTING_MUTE :
	{
		if(APP_ReadNCSwitchPulse())
		{
			userInputFSM = PLATFORM_SETTINGS_SAVE;
			APP_ReadNCDirection();
		}
		else
		{
			if (0 != (direction = APP_ReadNCDirection()))
			{
				if (1 == direction)
				{
					systemGlobalState.states.rdaIsMute = 1;
					RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);
				}
				else
				{
					systemGlobalState.states.rdaIsMute = 0;
					RDA5807mReset();
				}
				// set volume and save changes to EEPROM
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_ShowMute(message, systemGlobalState.states.rdaIsMute);
				blinkDot ^= 0x80;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case SETTING_TIME_HOURS:
	{
		if(APP_ReadNCSwitchPulse())
		{
			static uint8_t doubleClick = 1;

			if (2 == doubleClick)
			{
				doubleClick = 1;
				userInputFSM = SETTING_TIME_MINUTES;
				APP_ReadNCDirection();
			}
			else
			{
				doubleClick++;
			}

		}
		else
		{
			if (0 != (direction = APP_ReadNCDirection()))
			{
				if (1 == direction)
				{
					if (pRTC->Hours < 23)
					{
						pRTC->Hours++;
					}
				}
				else
				{
					if (pRTC->Hours > 0)
					{
						pRTC->Hours--;
					}
				}
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_CheckTime(message, pRTC, blinkDot);
				blinkDot ^= 0x01;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case SETTING_TIME_MINUTES:
	{
		if(APP_ReadNCSwitchPulse())
		{
			userInputFSM = SETTING_WIFI_ENABLE_DISABLE;
			wifi = systemGlobalState.states.wifiEnabled;
			APP_ReadNCDirection();
		}
		else
		{
			if (0 != (direction = APP_ReadNCDirection()))
			{
				if (1 == direction)
				{
					if (pRTC->Minutes < 59)
					{
						pRTC->Minutes++;
					}
				}
				else
				{
					if (pRTC->Minutes > 0)
					{
						pRTC->Minutes--;
					}
				}
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_CheckTime(message, pRTC, blinkDot);
				blinkDot ^= 0x01;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case SETTING_WIFI_ENABLE_DISABLE:
	{
		if(APP_ReadNCSwitchPulse())
		{
			userInputFSM = PLATFORM_SETTINGS_SAVE;
			APP_ReadNCDirection();
		}
		else
		{
			if (0 != (direction = APP_ReadNCDirection()))
			{
				if (1 == direction)
				{
					wifi = 1;
				}
				else
				{
					wifi = 0;
				}
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_CheckWifiEnable(message, wifi, blinkDot);
				blinkDot ^= 0x01;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case PLATFORM_SETTINGS_SAVE :
	{
		uint8_t rebootNeeded = ((systemGlobalState.states.wifiEnabled != wifi) && wifi) ? 1 : 0;

		systemGlobalState.states.wifiEnabled = wifi;
		EEPROM_SetSystemState();

		if (rebootNeeded)
		{
			NVIC_SystemReset();
		}
		else
		{
			HAL_RTC_SetTime(&hrtc, pRTC, RTC_FORMAT_BIN);
			userInputFSM = NO_SETTINGS_ONGOING;
			APP_CheckTime(message, pRTC, 1);
		}
	}
	break;
	default :
	{
		userInputFSM = NO_SETTINGS_ONGOING;
	}
	}

	return (uint8_t)userInputFSM;
}

void DBNC_NCSwitch3SecPress(void)
{
	static uint16_t btnCnt = 0;
	const uint16_t debouncLevel = 600;

	if(!HAL_GPIO_ReadPin(NCODER_BTN_GPIO_Port, NCODER_BTN_Pin))
	{
		// Must be calculated
		if (btnCnt < debouncLevel)
		{
			btnCnt++;
		}
		else
		{
			btn3SecPressFlag = 1;
		}
	}
	else
	{
		if(btnCnt)
		{
			btnCnt--;
		}
		else
		{
			btn3SecPressFlag = 0;
		}
	}
}

uint8_t APP_ReadNCSwitch3SecPress(void)
{
	uint8_t result = 0;
	if (1 == btn3SecPressFlag)
	{
		result = btn3SecPressFlag;
		btnSetFlag = 0;
	}
	return result;
}

/*printf <=> uart redirection */
int _write(int file, char *ptr, int len)
{
	CDC_Transmit_FS((uint8_t*)ptr, (uint16_t)len);
	return len;
}

/* Simulates that the USB was disconnected and so allows new USB insert*/
