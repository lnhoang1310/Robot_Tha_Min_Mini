#ifndef ROBOT_H
#define ROBOT_H

#ifdef c_plus_plus
extern "C" {
#endif

#include "tmc2209.h"

typedef enum{
	ROBOT_STOP = 0,
	ROBOT_RUN
}Robot_Status_t;

typedef struct{
	TMC2209_Handle_t* motorLeft;
	TMC2209_Handle_t* motorRight;
	Robot_Status_t status;
}Robot_Typedef;

HAL_StatusTypeDef Robot_Init(Robot_Typedef* robot, UART_HandleTypeDef* huart_left, TIM_HandleTypeDef* htim_left, uint16_t channel_left, 
												UART_HandleTypeDef* huart_right, TIM_HandleTypeDef* htim_right, uint16_t channel_right);
HAL_StatusTypeDef Robot_Control(Robot_Typedef* robot, float speed_left, float speed_right);
void Calculate_Velocity(Robot_Typedef* robot);

#ifdef c_plus_plus
}
#endif

#endif
