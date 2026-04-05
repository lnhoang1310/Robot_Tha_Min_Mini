#include "robot.h"
#include "i2c.h"
#include "as5600.h"
#include <math.h>

#define RATIO_MOTOR 3.7f

TMC2209_Handle_t motor_left, motor_right;
Soft_I2C_TypeDef i2c_as5600_left, i2c_as5600_right;
AS5600_Typedef encoder_left, encoder_right;

HAL_StatusTypeDef Robot_Init(Robot_Typedef* robot, UART_HandleTypeDef* huart_left, TIM_HandleTypeDef* htim_left, uint16_t channel_left, 
												UART_HandleTypeDef* huart_right, TIM_HandleTypeDef* htim_right, uint16_t channel_right)
{
	HAL_StatusTypeDef state = TMC2209_Init(&motor_left, huart_left, htim_left, channel_left, STEP_LEFT_DIR_GPIO_Port, STEP_LEFT_DIR_Pin, STEP_LEFT_EN_GPIO_Port, STEP_LEFT_EN_Pin, 0x00, 16);
	if(state != HAL_OK) return state;
	TMC2209_SetPinPolarity(&motor_left, 1, 0);
    TMC2209_Enable(&motor_left);
    HAL_Delay(10);
    state = TMC2209_ApplySilentSmoothProfile(&motor_left, 16, 6, 22, 8);
	if(state != HAL_OK) return state;
	
	state = TMC2209_Init(&motor_right, huart_right, htim_right, channel_right, STEP_RIGHT_DIR_GPIO_Port, STEP_RIGHT_DIR_Pin, STEP_RIGHT_EN_GPIO_Port, STEP_RIGHT_EN_Pin, 0x00, 16);
	if(state != HAL_OK) return state;
	TMC2209_SetPinPolarity(&motor_right, 1, 1);
    TMC2209_Enable(&motor_right);
    HAL_Delay(10);
    state = TMC2209_ApplySilentSmoothProfile(&motor_right, 16, 6, 22, 8);
	if(state != HAL_OK) return state;
	
	i2c_soft_init(&i2c_as5600_left, AS5600_LEFT_SCL_GPIO_Port, AS5600_LEFT_SCL_Pin, AS5600_LEFT_SDA_GPIO_Port, AS5600_LEFT_SDA_Pin);
	i2c_soft_init(&i2c_as5600_right, AS5600_RIGHT_SCL_GPIO_Port, AS5600_RIGHT_SCL_Pin, AS5600_RIGHT_SDA_GPIO_Port, AS5600_RIGHT_SDA_Pin);
	
	AS5600_Init(&encoder_left, &i2c_as5600_left, AS5600_I2C_SLAVE_ADDRESS);
	AS5600_Init(&encoder_right, &i2c_as5600_right, AS5600_I2C_SLAVE_ADDRESS);
	
	robot->motorLeft = &motor_left;
	robot->motorRight = &motor_right;
	Robot_Control(robot, 0, 0);
	
	return HAL_OK;
}

HAL_StatusTypeDef Robot_Control(Robot_Typedef* robot, float speed_left, float speed_right){
	HAL_StatusTypeDef status;
	if(robot->status == ROBOT_RUN){
		TMC2209_Dir_t dir_left = (speed_left > 0) ? TMC2209_DIR_FORWARD : TMC2209_DIR_REVERSE;
		TMC2209_Dir_t dir_right = (speed_right > 0) ? TMC2209_DIR_FORWARD : TMC2209_DIR_REVERSE;
		
		if(TMC2209_Control(robot->motorLeft, dir_left, fabs(speed_left)) != HAL_OK) status = HAL_ERROR;
		if(TMC2209_Control(robot->motorRight, dir_right, fabs(speed_right)) != HAL_OK) status = HAL_ERROR;
		
	}else{
		if(TMC2209_Stop(robot->motorLeft) != HAL_OK) status = HAL_ERROR;
		if(TMC2209_Stop(robot->motorRight) != HAL_OK) status = HAL_ERROR;
	}
	return status;
}

void Calculate_Velocity(Robot_Typedef* robot){
	robot->motorLeft->velocity = ((robot->motorLeft->direction == TMC2209_DIR_REVERSE) ? (AS5600_CalVelocity(&encoder_left) * (-1.0f)) : AS5600_CalVelocity(&encoder_left)) / RATIO_MOTOR;
	robot->motorRight->velocity = ((robot->motorRight->direction == TMC2209_DIR_REVERSE) ? (AS5600_CalVelocity(&encoder_right) * (-1.0f)) : AS5600_CalVelocity(&encoder_right)) / RATIO_MOTOR;
}
