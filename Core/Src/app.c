/*
 * app.c
 *
 *  Created on: Sep 20, 2022
 *      Author: 42077
 */


#include "app.h"

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

	  HAL_Delay(2000);

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
		LEDC_SetNewRollingString("EEPROM fault", strlen("EEPROM fault"));
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
	uint8_t anyFault = 0;
	// Receiver signal strength is always > 0
	if(0 == RDA5807mGetRSSI())
	{
		// inform, save the state and try to re-init
		systemGlobalState.states.rdaFunctional = 0;
		  // ensure the string is displayed
	    //while(LEDC_GetRollingStatus()) { }
		LEDC_SetNewRollingString("radio fault", strlen("radio fault"));
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
		APP_ESP_InitConnect();
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
				//while(LEDC_GetRollingStatus()) { }
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

	LEDC_SetNewRollingString(message, strlen(message));
}

void APP_CheckTime(char *message)
{
	RTC_TimeTypeDef rtc;

	HAL_RTC_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
	sprintf(message, "%02d%02d", rtc.Hours, rtc.Minutes);
	LEDC_SetStandingDot(2);
	LEDC_SetNewStandingText(message);
}
void APP_ModuleCheckStates(uint32_t timeout)
{
	static uint32_t prevTick;
	static uint32_t prevTime;
	static uint8_t informationFSM = 0;

	uint8_t anyFault = 0;
	char message[32] = {0};

	if(prevTick + timeout < HAL_GetTick() && !LEDC_GetRollingStatus())
	{

		LEDC_StopStandingText();

		switch(informationFSM)
		{
		case CHECK_EEPROM:
		{
			anyFault = APP_CheckEEPROM();
			informationFSM = CHECK_RADIO;
		}
		if (anyFault) break;
		case CHECK_RADIO:
		{
			anyFault = APP_CheckRadio();
			informationFSM = CHECK_WIFI;
		}
		if (anyFault) break;
		case CHECK_WIFI:
		{
			APP_CheckWIFI();
			informationFSM = CHECK_TEMPERATURE;
		}
		break;
		case CHECK_TEMPERATURE:
		{
			APP_CheckTemperature(message);
			informationFSM = CHECK_RADIO_OP;
		}
		break;
		case CHECK_RADIO_OP:
		{
			APP_CheckRadioOperation(message);
			informationFSM = CHECK_EEPROM;
		}
		break;
		default :
		{
			/* Show time as a standing string */
		}
		}

		prevTick = HAL_GetTick();
	}
	else
	{

			if (prevTime + 1000 < HAL_GetTick())
			{
				LEDC_StopStandingText();
				LEDC_SetStandingDot(0);
				prevTime = HAL_GetTick();

				if(systemGlobalState.states.displayTime)
				{
					APP_CheckTime(message);
				}
				else
				{
					APP_CheckTemperature(message);
				}
			}
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
}

void APP_RTC_GetHHMM(uint8_t *hh, uint8_t *mm)
{
	if(hh) *hh = 12;
	if(mm) *mm = 34;
}
/*printf <=> uart redirection */
int _write(int file, char *ptr, int len)
{
	CDC_Transmit_FS((uint8_t*)ptr, (uint16_t)len);
	return len;
}

/* Simulates that the USB was disconnected and so allows new USB insert*/
