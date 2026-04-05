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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum{
	LAND_MINE,
	TANK_MINE
}Mine_Type;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define STEP_RIGHT_DIR_Pin GPIO_PIN_13
#define STEP_RIGHT_DIR_GPIO_Port GPIOC
#define AS5600_RIGHT_SCL_Pin GPIO_PIN_14
#define AS5600_RIGHT_SCL_GPIO_Port GPIOC
#define AS5600_RIGHT_SDA_Pin GPIO_PIN_15
#define AS5600_RIGHT_SDA_GPIO_Port GPIOC
#define STEP_LEFT_DIR_Pin GPIO_PIN_0
#define STEP_LEFT_DIR_GPIO_Port GPIOA
#define STEP_LEFT_PWM_Pin GPIO_PIN_1
#define STEP_LEFT_PWM_GPIO_Port GPIOA
#define STEP_LEFT_EN_Pin GPIO_PIN_5
#define STEP_LEFT_EN_GPIO_Port GPIOA
#define AS5600_LEFT_SCL_Pin GPIO_PIN_13
#define AS5600_LEFT_SCL_GPIO_Port GPIOB
#define AS5600_LEFT_SDA_Pin GPIO_PIN_14
#define AS5600_LEFT_SDA_GPIO_Port GPIOB
#define VL53L0X_SCL_Pin GPIO_PIN_15
#define VL53L0X_SCL_GPIO_Port GPIOB
#define VL53L0X_SDA_Pin GPIO_PIN_8
#define VL53L0X_SDA_GPIO_Port GPIOA
#define VL53L0X_XHSUT_Pin GPIO_PIN_12
#define VL53L0X_XHSUT_GPIO_Port GPIOA
#define STEP_RIGHT_EN_Pin GPIO_PIN_9
#define STEP_RIGHT_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
