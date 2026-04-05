#include "Servo.h"

uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max){
	return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

void Servo_Control(Servo_TypeDef* servo){
	uint32_t ccr = map(servo->Angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
	switch(servo->Channel){
	case TIM_CHANNEL_1:
		servo->htim->Instance->CCR1 = ccr;
		break;
	case TIM_CHANNEL_2:
		servo->htim->Instance->CCR2 = ccr;
		break;
	case TIM_CHANNEL_3:
		servo->htim->Instance->CCR3 = ccr;
		break;
	case TIM_CHANNEL_4:
		servo->htim->Instance->CCR4 = ccr;
		break;
	}
}

void Servo_Set(Servo_TypeDef* servo, uint8_t angle){
	servo->Angle = angle;
	Servo_Control(servo);
}

void Servo_Init(Servo_TypeDef *servo, TIM_HandleTypeDef* htim, uint32_t channel, uint8_t idle_angle){
	servo->htim =  htim;
	servo->Channel = channel;
	servo->Angle = 0;
	htim->Instance->PSC = 71;
	htim->Instance->ARR = 20000 - 1;
	HAL_TIM_PWM_Start(servo->htim, servo->Channel);
	Servo_Set(servo, idle_angle);
}	
