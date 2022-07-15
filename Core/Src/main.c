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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmd_dispatcher.h"
//#include "usbd_cdc_if.h"
#include  "ledc_port.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint8_t ASCII27SEG(uint8_t ascii);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
uint8_t alphabet[62] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.-";
uint8_t string[] = "   HELLO WORLD 192.168.0.1 123   ";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void MAIN_ShortcutUSB(void);
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
    //uint8_t rxStrlng;
    //uint8_t* rxStrBuff = NULL;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  //MAIN_ShortcutUSB();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  uint8_t segment = 0, anodes= 1, digit = 0;

  uint32_t tick = HAL_GetTick();
  uint32_t tick2 = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#if 0
      rxStrBuff = VCOMFetchReceivedLine(&rxStrlng);
      if (NULL != rxStrBuff) {
          /* If so and is terminated by <LF>,
             process it as command*/
          CmdDispatch(rxStrBuff, rxStrlng);
      }
#endif

      if (tick + 350 < HAL_GetTick())
      {
          tick = HAL_GetTick();

          segment = (segment + 1) % (strlen((char*)string)-3);
      }

      if (tick2 + 3 < HAL_GetTick())
      {
          tick2 = HAL_GetTick();

          if (anodes == 0x08)
        	  anodes = 0x01;
          else
        	  anodes = anodes << 1;

          digit = (digit + 1) % 4;

          uint8_t dig = ASCII27SEG(string[segment + digit]);

          LEDC_WRITE_7SEG(dig);
    	  LEDC_WRITE_ANODES(anodes);

      }

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
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 50000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ESP_RST_Pin|LEDC_G_Pin|LEDC_A_Pin|LEDC_F_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LEDC_E_Pin|LEDC_D_Pin|LEDC_DP_Pin|LEDC_C_Pin
                          |LEDC_B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDC_A2_GPIO_Port, LEDC_A2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LEDC_A1_Pin|LEDC_A4_Pin|LEDC_A3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ESP_RST_Pin LEDC_G_Pin LEDC_A_Pin LEDC_F_Pin
                           LEDC_A2_Pin */
  GPIO_InitStruct.Pin = ESP_RST_Pin|LEDC_G_Pin|LEDC_A_Pin|LEDC_F_Pin
                          |LEDC_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LEDC_E_Pin LEDC_D_Pin LEDC_DP_Pin LEDC_C_Pin
                           LEDC_A1_Pin LEDC_A4_Pin LEDC_B_Pin LEDC_A3_Pin */
  GPIO_InitStruct.Pin = LEDC_E_Pin|LEDC_D_Pin|LEDC_DP_Pin|LEDC_C_Pin
                          |LEDC_A1_Pin|LEDC_A4_Pin|LEDC_B_Pin|LEDC_A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/*printf <=> uart redirection */
int _write(int file, char *ptr, int len)
{
	//CDC_Transmit_FS((uint8_t*)ptr, (uint16_t)len);
	return len;
}

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


uint8_t ASCII27SEG(uint8_t ascii)
{
	uint8_t encoded;

	switch(ascii)
	{
	case '0': encoded = 0b00111111; break;
	case '1': encoded = 0b00000110; break;
	case '2': encoded = 0b01011011; break;
	case '3': encoded = 0b01001111; break;
	case '4': encoded = 0b01100110; break;
	case '5': encoded = 0b01101101; break;
	case '6': encoded = 0b01111101; break;
	case '7': encoded = 0b00000111; break;
	case '8': encoded = 0b01111111; break;
	case '9': encoded = 0b01101111; break;
	case 'A': encoded = 0b01110111; break;
	case 'B': encoded = 0b01111100; break;
	case 'C': encoded = 0b00111001; break;
	case 'D': encoded = 0b01011110; break;
	case 'E': encoded = 0b01111001; break;
	case 'F': encoded = 0b01110001; break;
	case 'G': encoded = 0b00111101; break;
	case 'H': encoded = 0b01110100; break;
	case 'I': encoded = 0b00110000; break;
	case 'J': encoded = 0b00011110; break;
	case 'K': encoded = 0b01110101; break;
	case 'L': encoded = 0b00111000; break;
	case 'M': encoded = 0b01010101; break;
	case 'N': encoded = 0b01010100; break;
	case 'O': encoded = 0b01011100; break;
	case 'P': encoded = 0b01110011; break;
	case 'Q': encoded = 0b01100111; break;
	case 'R': encoded = 0b01010000; break;
	case 'S': encoded = 0b01101101; break;
	case 'T': encoded = 0b01111000; break;
	case 'U': encoded = 0b00111110; break;
	case 'V': encoded = 0b00011100; break;
	case 'W': encoded = 0b01101010; break;
	case 'X': encoded = 0b01110110; break;
	case 'Y': encoded = 0b01101110; break;
	case 'Z': encoded = 0b01011011; break;
	case '.': encoded = 0b10000000; break;
	case '-': encoded = 0b01000000; break;
	default: encoded = 0x00; break;
	}
	return encoded;
}


void LEDC_Show4Digits(const uint8_t * digits)
{
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

