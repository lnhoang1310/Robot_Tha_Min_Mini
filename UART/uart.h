#ifndef UART_H
#define UART_H
#include "stm32f1xx.h"
#include "robot.h"

#define UART_BUFFER_SIZE 256

void uart_receive_data(uint8_t data_rx);
void uart_handle(Robot_Typedef* robot, uint8_t* flag_mine_laying, Mine_Type* type);

#endif
