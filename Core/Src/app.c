/*
 * app.c
 *
 *  Created on: Sep 20, 2022
 *      Author: 42077
 */


#include "app.h"

static uint8_t displayFSMstate = 0;

void APP_ESP_InitConnect(void)
{
	  // ensure the string is displayed
	  BluePill_SetBlinkState(PERMANENT_BLINK);
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
	  BluePill_SetBlinkState(NO_BLINK);

}

void APP_RDA5807M_RadioInit(void)
{
	if (systemGlobalState.states.rdaEnabled)
	{
		RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);
	}
	else
	{
		RDA5807mReset();
	}
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

void APP_ModuleCheckStates(char *message, RTC_TimeTypeDef* pRTC)
{
	static uint8_t prevSeconds = 0;
	static uint8_t timeElapsed = 0;
	const uint8_t pernamentDot = 1;
	uint8_t ee_err;
	uint8_t rda_err;

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

			if (systemGlobalState.states.rdaEnabled)
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
			if (systemGlobalState.states.espEnabled)
			{
				LEDC_StopStandingText();
				APP_CheckWIFI();
			}
		}
		break;
		case 11 :
		{
			LEDC_StopStandingText();
			ee_err = APP_CheckEEPROM();
			rda_err = APP_CheckRadio();
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

uint8_t APP_UserInput(char *message, RTC_TimeTypeDef *pRTC)
{

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
				APP_DisplayFrequencySetings(message, systemGlobalState.radioFreq, blinkDot);
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
			userInputFSM = SETTING_RDA_ENABLE;
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
				APP_DisplayVolumeSetings(message, systemGlobalState.radioVolm, blinkDot);
				blinkDot ^= 0x80;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case SETTING_RDA_ENABLE :
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
					systemGlobalState.states.rdaEnabled = 1;
					RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);
				}
				else
				{
					systemGlobalState.states.rdaEnabled = 0;
					RDA5807mReset();
				}
				// set volume and save changes to EEPROM
			}
			if (timeOut + blinkDotPeriod < PLATFORM_TICK_MS())
			{
				APP_DisplayRDAEnableSettings(message, systemGlobalState.states.rdaEnabled);
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
			/* To move forward to SETTING_MINUTES_HOURS
			 * because for comming here is needed 3s */
			static uint8_t secondClick = 1;

			if (2 == secondClick)
			{
				secondClick = 1;
				userInputFSM = SETTING_TIME_MINUTES;
				APP_ReadNCDirection();
			}
			else
			{
				secondClick++;
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
			userInputFSM = SETTING_ESP_ENABLE;
			wifi = systemGlobalState.states.espEnabled;
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
	case SETTING_ESP_ENABLE:
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
				APP_DisplayEspEnableSettings(message, wifi, blinkDot);
				blinkDot ^= 0x01;
				timeOut = PLATFORM_TICK_MS();
			}
		}
	}
	break;
	case PLATFORM_SETTINGS_SAVE :
	{
		uint8_t rebootNeeded = ((systemGlobalState.states.espEnabled != wifi) && wifi) ? 1 : 0;

		systemGlobalState.states.espEnabled = wifi;
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
