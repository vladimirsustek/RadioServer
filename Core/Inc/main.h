/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOC
#define ESP_RST_Pin GPIO_PIN_1
#define ESP_RST_GPIO_Port GPIOA
#define FLOATING_A_Pin GPIO_PIN_2
#define FLOATING_A_GPIO_Port GPIOA
#define FLOATING_B_Pin GPIO_PIN_3
#define FLOATING_B_GPIO_Port GPIOA
#define LEDC_E_Pin GPIO_PIN_12
#define LEDC_E_GPIO_Port GPIOB
#define LEDC_D_Pin GPIO_PIN_13
#define LEDC_D_GPIO_Port GPIOB
#define LEDC_DP_Pin GPIO_PIN_14
#define LEDC_DP_GPIO_Port GPIOB
#define LEDC_C_Pin GPIO_PIN_15
#define LEDC_C_GPIO_Port GPIOB
#define LEDC_G_Pin GPIO_PIN_8
#define LEDC_G_GPIO_Port GPIOA
#define LEDC_A_Pin GPIO_PIN_9
#define LEDC_A_GPIO_Port GPIOA
#define LEDC_F_Pin GPIO_PIN_10
#define LEDC_F_GPIO_Port GPIOA
#define LEDC_A2_Pin GPIO_PIN_15
#define LEDC_A2_GPIO_Port GPIOA
#define LEDC_A1_Pin GPIO_PIN_3
#define LEDC_A1_GPIO_Port GPIOB
#define LEDC_A4_Pin GPIO_PIN_4
#define LEDC_A4_GPIO_Port GPIOB
#define LEDC_B_Pin GPIO_PIN_6
#define LEDC_B_GPIO_Port GPIOB
#define LEDC_A3_Pin GPIO_PIN_7
#define LEDC_A3_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
