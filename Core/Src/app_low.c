/*
 * app_low.c
 *
 *  Created on: Oct 16, 2022
 *      Author: 42077
 */

#include "app_low.h"

static uint8_t btnSetFlag = 0;
static uint8_t btn3SecPressFlag = 0;
static uint8_t ncDirection = 0;
static uint8_t clkStateprev = 0;
static uint8_t dtStateprev = 0;

/* ----------------- UART Redirection for printf ---------------------------------- */
/* -------------------------------------------------------------------------------- */
int _write(int file, char *ptr, int len)
{
	CDC_Transmit_FS((uint8_t*)ptr, (uint16_t)len);
	return len;
}

/* --- USB auxiliary function to shortcut PC re-inser the USB device  -- */
/* -------------------------------------------------------------------------------- */

/*
 * @brief Simulates that the USB was disconnected and so allows new USB insert
 * The STM USB pins are pulled down and so PC looses both signal lines
 * */

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
/* ------- N-Coder device porting functions (to be called by timer handler) -------- */
/* -------------------------------------------------------------------------------- */

/* @brief To be called periodically in timer handler/interrupt.
 * Raises btn3SecPressFlag when flag btn is being pressed long enough
 * internal debouncLevel counter limit 600 signalizes that the
 * function should be called each 5th millisecond (200Hz)
 * to fulfill 3 seconds interval */
void DBNC_NCSwitch3SecPress(void)
{
	static uint16_t btnCnt = 0;
	/* Value must be tuned appropriately  */
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

/* @brief To be called periodically in timer handler/interrupt.
 * Raises btnSetFlag when flag btn is being pressed
 * internal debouncLevel counter limit debouncLevel works well
 * when is the function called with frequency 200Hz (5ms) */
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

/* @brief Necessary initialization of the N Coder, to initialize
 * settled states for internal logic */
void APPP_NCoderStart(void)
{
	btnSetFlag = 0;
	btn3SecPressFlag = 0;
	ncDirection = 0;
	clkStateprev = 0;
	dtStateprev = 0;

	clkStateprev = HAL_GPIO_ReadPin(NCODER_CLK_GPIO_Port, NCODER_CLK_Pin);
	dtStateprev = HAL_GPIO_ReadPin(NCODER_DT_GPIO_Port, NCODER_DT_Pin);
}

/* @brief To be called periodically in timer handler/interrupt.
 * Raises ncDirection when N-Coder rotates left or right
 * the function works well well called with frequency 200Hz (5ms)
 */
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

/* ------- N-Coder device interface functions (to be used by upper layer)  -------- */
/* -------------------------------------------------------------------------------- */
/*
 * @brief Interface function to read 3 seconds N-Coder press
 */
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

/*
 * @brief Interface function to read short N-Coder click
 */
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


/*
 * @brief Interface function to read N-Coder rotation step direction (-1 or 1)
 */
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
/* ------- Functions to check/detect external peripheral status (fault/ok) -------- */
/* -------------------------------------------------------------------------------- */

/*
 * @brief Initiates EEPROM read to check its functionality
 */
uint8_t APP_CheckEEPROM(void)
{
	uint8_t anyFault = 0;
	if(!APP_EEPROM_CheckIfOk())
	{
		anyFault = 1;
	}
	else
	{
		anyFault = 0;
	}
	return anyFault;
}

/*
 * @brief Reads RSSI when radio is enabled to detect its functionality
 */
uint8_t APP_CheckRadio(void)
{

	uint8_t anyFault = 0;

	if(systemGlobalState.states.rdaEnabled)
	{
		// Receiver signal strength is always > 0
		if (0 == RDA5807mGetRSSI())
		{
			anyFault = 1;
		}
		else
		{
			anyFault = 0;
		}
	}
	else
	{
		anyFault = 0;
	}
	return anyFault;
}

/*
 * @brief Checks whether is ESP connected to host router and has IP
 * and also if provides a server functionality (see AT commands below)
 * and also initiates display message (offline or IP adr 192.168.X.Y)
 */
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
	}

	ESP_SendCommand("AT+CIPMUX?\r\n", strlen("AT+CIPMUX?\r\n"));

	if(NULL == ESP_CheckResponse("+CIPMUX:1", strlen("+CIPMUX:1"), ESP_TIMEOUT_300ms))
	{
		systemGlobalState.states.espConnected = 0;
		  // ensure the string is displayed
	    //while(LEDC_GetRollingStatus()) { }
		LEDC_SetNewRollingString("offline", strlen("offline"));
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

/* ------- Functions to check/detect external peripheral operation ------- -------- */
/* -------------------------------------------------------------------------------- */

/*
 * @brief When radio operates Gets and displays operational volume and frequency
 */
void APP_CheckRadioOperation(char* message)
{
	if(systemGlobalState.states.rdaEnabled)
	{
		sprintf(message, "frequency %d.%d khz volume %d",
				systemGlobalState.radioFreq / 100,
				systemGlobalState.radioFreq % 100,
				systemGlobalState.radioVolm);
		  // ensure the string is displayed
		LEDC_SetNewRollingString(message, strlen(message));
	}
}

/*
 * @brief Gets and displays current temperature within the radio plexi-case
 */
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

/*
 * @brief Gets and displays current time from the STM32F103 RTC module (32.768kHz xtal)
 */
void APP_CheckTime(char *message, RTC_TimeTypeDef* pRtc, uint8_t blinkDot)
{
	//HAL_RTC_GetTime(&hrtc, pRtc, RTC_FORMAT_BIN);

	LEDC_SetStandingDot(0);
	sprintf(message, "%02d%02d", pRtc->Hours, pRtc->Minutes);
	if (blinkDot)
	{
		LEDC_SetStandingDot(2);
	}
	LEDC_SetNewStandingText(message);
}

/*
 * @brief 
 */
void APP_DisplayEspEnableSettings(char *message, uint8_t wifi, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "ESP%d", wifi);
	if (blinkDot)
	{
		LEDC_SetStandingDot(3);
	}
	LEDC_SetNewStandingText(message);
}

void APP_DisplayFrequencySetings(char *message, uint16_t freqeuncy, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "%03d%01d", freqeuncy/100, freqeuncy%100);
	if (blinkDot)
	{
		LEDC_SetStandingDot(3);
	}
	LEDC_SetNewStandingText(message);
}

void APP_DisplayVolumeSetings(char *message, uint16_t volume, uint8_t blinkDot)
{
	LEDC_SetStandingDot(0);
	sprintf(message, "VL%02d", volume);
	if (blinkDot)
	{
		LEDC_SetStandingDot(2);
	}
	LEDC_SetNewStandingText(message);
}
void APP_DisplayRDAEnableSettings(char *message, uint8_t mute)
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

uint8_t APP_EEPROM_CheckIfOk(void)
{
	/* not needed to store into eeprom that the eeprom works*/
	EEPROM_GetSystemState();
	if (0xA == systemGlobalState.states.dummy0xA)
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
