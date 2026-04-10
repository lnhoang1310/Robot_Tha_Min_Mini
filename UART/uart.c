#include "uart.h"
#include "response.h"
#include <string.h>
#include "main.h"
#include "Servo.h"

extern Servo_TypeDef servo_land_mine;
extern Servo_TypeDef servo_tank_mine_push;
extern Servo_TypeDef servo_tank_mine_left_door;
extern Servo_TypeDef servo_tank_mine_right_door;
uint8_t uart_buffer[UART_BUFFER_SIZE];
uint8_t uart_flag_cplt = 0;
uint16_t buffer_index = 0;
uint8_t flag_error_message = 0;
int8_t speed_left = 0;
float vel_left = 0;
float vel_right = 0;
HAL_StatusTypeDef status = HAL_OK;
uint32_t time_servo = 0;
extern uint8_t num_mine;

static float map(float x, float a, float b){
	return x * (b / a);
}

void uart_receive_data(uint8_t data){
	if(data == '\n'){
		if(flag_error_message){
			flag_error_message = 0;
			return;
		}
		uart_buffer[buffer_index] = '\0';
		uart_flag_cplt = 1;
		buffer_index = 0;
		return;
	}
	if(buffer_index >=  UART_BUFFER_SIZE){
		flag_error_message = 1;
		buffer_index = 0;
		return;
	}
	uart_buffer[buffer_index++] = data;
}

void uart_handle(Robot_Typedef* robot, uint8_t* flag_mine_laying, Mine_Type* type){
	if(uart_flag_cplt){
		*flag_mine_laying = 0;
		uart_flag_cplt = 0;
		char* argv[5];
		int8_t speed_right = 0;
		uint8_t state_robot = 0;
		uint8_t argv_index = 0;
		
		char* token = strtok((char*)uart_buffer, " ");
		while(token != NULL){
			argv[argv_index++] = token;
			token = strtok(NULL, " ");
		}
		argv[argv_index] = "0";
		if(!response_uart(argv, &speed_left, &speed_right, &state_robot)) return;
		if(argv[2][0] == '1') *flag_mine_laying = 1;
		
		if(!state_robot){
			robot->status = ROBOT_STOP;
			Robot_Control(robot, 0, 0);
		}else robot->status = ROBOT_RUN;
		
		vel_left = map(speed_left, 100, MAX_RPM);
		vel_right = map(speed_right, 100, MAX_RPM);
		status = Robot_Control(robot, vel_left, vel_right);
		
		if(*flag_mine_laying){
			if(num_mine > 0){
				if(servo_tank_mine_push.Angle != IDLE_POSITION_ANGLE_TYPE_TANK_MINE) Servo_Set(&servo_tank_mine_push, IDLE_POSITION_ANGLE_TYPE_TANK_MINE);
				if(servo_tank_mine_left_door.Angle != CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE) Servo_Set(&servo_tank_mine_left_door, CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE);
				if(servo_tank_mine_right_door.Angle != CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE) Servo_Set(&servo_tank_mine_right_door, CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE);
				
				Servo_Set(&servo_tank_mine_push, PUSH_POSITION_ANGLE_TYPE_TANK_MINE);
				time_servo = HAL_GetTick();
			}
		}
	}
	if(servo_tank_mine_push.Angle != IDLE_POSITION_ANGLE_TYPE_TANK_MINE && HAL_GetTick() - time_servo >= 1000){
		Servo_Set(&servo_tank_mine_push, IDLE_POSITION_ANGLE_TYPE_LAND_MINE);
		Servo_Set(&servo_tank_mine_left_door, OPEN_DOOR_POSITION_ANGLE_TYPE_TANK_MINE);
		Servo_Set(&servo_tank_mine_right_door, OPEN_DOOR_POSITION_ANGLE_TYPE_TANK_MINE);
		time_servo = HAL_GetTick();
	}
	
	if((servo_tank_mine_left_door.Angle != CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE || servo_tank_mine_right_door.Angle!= CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE) && HAL_GetTick() - time_servo >= 1000){
		Servo_Set(&servo_tank_mine_left_door, CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE);
		Servo_Set(&servo_tank_mine_right_door, CLOSE_DOOR_POSITION_ANGLE_TYPE_TANK_MINE);
	}
}
