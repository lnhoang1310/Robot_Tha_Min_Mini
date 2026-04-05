#ifndef INC_SERVO_H
#define INC_SERVO_H

#include "stm32f1xx.h"
#define MIN_PULSE_WIDTH 490
#define MAX_PULSE_WIDTH 2550

#define IDLE_POSITION_ANGLE_TYPE_LAND_MINE 180
#define PUSH_POSITION_ANGLE_TYPE_LAND_MINE 0
#define IDLE_POSITION_ANGLE_TYPE_TANK_MINE 0
#define PUSH_POSITION_ANGLE_TYPE_TANK_MINE 180
#define CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE 0
#define OPEN_DOOR_POSITION_ANGLE_TYPE_TANK_MINE 180

typedef struct{
	TIM_HandleTypeDef *htim;
	uint32_t Channel;
	uint8_t Angle;
}Servo_TypeDef;

void Servo_Set(Servo_TypeDef* servo, uint8_t angle);
void Servo_Init(Servo_TypeDef *servo, TIM_HandleTypeDef* htim, uint32_t channel, uint8_t idle_angle);
#endif
