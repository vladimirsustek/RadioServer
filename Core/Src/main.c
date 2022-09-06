/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>

#include "stm32f1xx_hal_conf.h"
#include "usbd_cdc_if.h"

#include "esp8266_http_server.h"
#include "rda5807m.h"
#include "cmd_dispatcher.h"
#include "eeprom_25aa1024.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CHECK_EEPROM	(uint8_t)(0)
#define CHECK_RADIO		(uint8_t)(1)
#define CHECK_WIFI		(uint8_t)(2)
#define CHECK_FREQUENCY	(uint8_t)(3)
#define CHECK_VOLUME	(uint8_t)(4)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void MAIN_ShortcutUSB(void);
void MAIN_ModuleCheckStates(void);
uint8_t MAIN_EEPROM_CheckIfOk(void);
void MAIN_ESP_InitConnect(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t* rxStrBuff = NULL;
	char *pHTTPReq = NULL;

   uint32_t rxStrlng;

   uint32_t httpReqLng = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  MAIN_ShortcutUSB();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_DMA_Init();
  MX_USB_DEVICE_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  LEDC_InitHW();

  // Checks EEPROM functionality and also reads the systemGlobalState
  // stored in the EEPROM containing last frequency, volume
  MAIN_EEPROM_CheckIfOk();

  if(0 == systemGlobalState.states.eepromFunctional)
  {
	  LEDC_SetNewRollingString("EEPROM fault", strlen("EEPROM fault"));
  }

  RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);

  MAIN_ESP_InitConnect();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  // VCOM RX routine
      rxStrBuff = VCOMFetchReceivedLine(&rxStrlng);

      if (NULL != rxStrBuff)
      {
          /* If so and is terminated by <LF>,
             process it as command*/
          CmdDispatch(rxStrBuff, rxStrlng);
      }

      // Function returns ESP_RET_OK when HTTP request received
      if(ESP_RET_OK == ESP_CheckReceiveHTTP(&pHTTPReq, &httpReqLng))
      {
    	  // Process the request
          ESP_ProcessHTTP(pHTTPReq, httpReqLng);
      }

      // 1x per 30 second check whether:
      // 				ESP is connected to WIFI
      //				RDA is functional (RSSI > 0)
      // If RDA is functional inform about freq and volm
      MAIN_ModuleCheckStates();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*printf <=> uart redirection */
int _write(int file, char *ptr, int len)
{
	CDC_Transmit_FS((uint8_t*)ptr, (uint16_t)len);
	return len;
}

/* Simulates that the USB was disconnected and so allows new USB insert*/
void MAIN_ShortcutUSB(void)
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

void MAIN_ModuleCheckStates(void)
{
	static uint32_t prevTick;
	static uint8_t informationFSM = 0;
	uint8_t anyFault = 0;

	if(prevTick + 30*1000 < HAL_GetTick() && !LEDC_GetRollingStatus())
	{
		char message[32] = {0};


		switch(informationFSM)
		{
		case CHECK_EEPROM:
		{
			if(!MAIN_EEPROM_CheckIfOk())
			{
				LEDC_SetNewRollingString("EEPROM fault", strlen("EEPROM fault"));
				anyFault = 1;
			}
			else
			{
				anyFault = 0;
			}

			informationFSM = CHECK_RADIO;
		}
		if (anyFault) break;
		case CHECK_RADIO:
		{
			// Receiver signal strength is always > 0
			if(0 == RDA5807mGetRSSI())
			{
				// inform, save the state and try to re-init
				systemGlobalState.states.rdaFunctional = 0;
				LEDC_SetNewRollingString("radio fault", strlen("radio fault"));
				anyFault = 1;
				RDA5807mInit(systemGlobalState.radioFreq, systemGlobalState.radioVolm);
			}
			else
			{
				anyFault = 0;
				systemGlobalState.states.rdaFunctional = 1;
			}

			informationFSM = CHECK_WIFI;

		}
		if (anyFault) break;
		case CHECK_WIFI:
		{
			// 1) Are we connected ? (AT+CWJAP?)
			// 2) Are we providing a server ? (AT+CIPMUX?)


			ESP_SendCommand("AT+CWJAP?\r\n", strlen("AT+CWJAP?\r\n"));
			// either +CWJAP:"SSID","MAC address" ..  OK
			// or No AP OK
			if(ESP_CheckResponse("+CWJAP", strlen("+CWJAP"), ESP_TIMEOUT_300ms))
			{
				systemGlobalState.states.espConnected = 0;
				LEDC_SetNewRollingString("offline", strlen("offline"));

				anyFault = 1;
				// Try to re-init
				MAIN_ESP_InitConnect();
			}
			else
			{
				anyFault = 0;
			}

			ESP_SendCommand("AT+CIPMUX?\r\n", strlen("AT+CIPMUX?\r\n"));

			if(ESP_CheckResponse("+CIPMUX:1", strlen("+CIPMUX:1"), ESP_TIMEOUT_300ms))
			{
				systemGlobalState.states.espConnected = 0;
				LEDC_SetNewRollingString("offline", strlen("offline"));

				anyFault = 1;
				// Try to re-init
				MAIN_ESP_InitConnect();
			}
			else
			{
				anyFault = 0;
			}
			informationFSM = CHECK_FREQUENCY;
		}
		if (anyFault) break;
		case CHECK_FREQUENCY:
		{
			if(systemGlobalState.states.rdaFunctional)
			{
				sprintf(message, "frequency %d khz", systemGlobalState.radioFreq*10);
				LEDC_SetNewRollingString(message, strlen(message));
			}

			informationFSM = CHECK_VOLUME;
		}
		break;
		case CHECK_VOLUME:
		{
			if(systemGlobalState.states.rdaFunctional)
			{
				if(systemGlobalState.states.rdaIsMute)
				{
					sprintf(message, "muted");
				}
				else
				{
					sprintf(message, "volume %d", systemGlobalState.radioVolm);
				}
				LEDC_SetNewRollingString(message, strlen(message));
			}

			informationFSM = CHECK_EEPROM;
		}
		default :
		{
			/* Show time as a standing string */
		}
		}

		prevTick = HAL_GetTick();
	}
}


uint8_t MAIN_EEPROM_CheckIfOk(void)
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
	}

	return (uint8_t)systemGlobalState.states.eepromFunctional;

}

void MAIN_ESP_InitConnect(void)
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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

