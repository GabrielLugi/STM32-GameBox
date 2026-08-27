/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define _2PLAVA_A1_Pin GPIO_PIN_1
#define _2PLAVA_A1_GPIO_Port GPIOA
#define _2PLAVA_A1_EXTI_IRQn EXTI1_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define _1PLAVA_A2_Pin GPIO_PIN_4
#define _1PLAVA_A2_GPIO_Port GPIOA
#define _1PLAVA_A2_EXTI_IRQn EXTI4_IRQn
#define WS2812_D12_Pin GPIO_PIN_6
#define WS2812_D12_GPIO_Port GPIOA
#define ZELENA_A3_Pin GPIO_PIN_0
#define ZELENA_A3_GPIO_Port GPIOB
#define ZELENA_A3_EXTI_IRQn EXTI0_IRQn
#define BUZZER_D6_Pin GPIO_PIN_10
#define BUZZER_D6_GPIO_Port GPIOB
#define DC___D9_Pin GPIO_PIN_7
#define DC___D9_GPIO_Port GPIOC
#define RST___D8_Pin GPIO_PIN_9
#define RST___D8_GPIO_Port GPIOA
#define CRVENA_D2_Pin GPIO_PIN_10
#define CRVENA_D2_GPIO_Port GPIOA
#define CRVENA_D2_EXTI_IRQn EXTI15_10_IRQn
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define BL___D5_Pin GPIO_PIN_4
#define BL___D5_GPIO_Port GPIOB
#define ZUTA_D4_Pin GPIO_PIN_5
#define ZUTA_D4_GPIO_Port GPIOB
#define ZUTA_D4_EXTI_IRQn EXTI9_5_IRQn
#define CS_TFT__D10_Pin GPIO_PIN_6
#define CS_TFT__D10_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
