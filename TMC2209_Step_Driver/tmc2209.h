#ifndef TMC2209_H
#define TMC2209_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define TMC2209_REG_GCONF         0x00
#define TMC2209_REG_IHOLD_IRUN    0x10
#define TMC2209_REG_TPOWERDOWN    0x11
#define TMC2209_REG_CHOPCONF      0x6C
#define TMC2209_REG_PWMCONF       0x70

#define MAX_RPM 250

typedef enum {
    TMC2209_DIR_FORWARD = 0,
    TMC2209_DIR_REVERSE = 1
} TMC2209_Dir_t;

typedef struct {
    UART_HandleTypeDef *huart;

    TIM_HandleTypeDef  *htim;
    uint32_t tim_channel;

    GPIO_TypeDef *dir_port;
    uint16_t      dir_pin;

    GPIO_TypeDef *en_port;
    uint16_t      en_pin;

    uint8_t slave_addr;
    uint8_t en_active_low;
    uint8_t dir_inverted;

    uint8_t running;
    uint16_t microsteps;
	
	TMC2209_Dir_t direction;
	float velocity;
} TMC2209_Handle_t;

/* basic */
HAL_StatusTypeDef TMC2209_Init(TMC2209_Handle_t *h, UART_HandleTypeDef *huart,
                  TIM_HandleTypeDef *htim_pwm, uint32_t tim_channel,
                  GPIO_TypeDef *dir_port, uint16_t dir_pin,
                  GPIO_TypeDef *en_port,  uint16_t en_pin,
                  uint8_t slave_addr, uint16_t microstep);

void TMC2209_SetPinPolarity(TMC2209_Handle_t *h, uint8_t en_active_low, uint8_t dir_inverted);
void TMC2209_Enable(TMC2209_Handle_t *h);
void TMC2209_Disable(TMC2209_Handle_t *h);
void TMC2209_SetDirection(TMC2209_Handle_t *h, TMC2209_Dir_t dir);

/* speed / motion */
HAL_StatusTypeDef TMC2209_Control(TMC2209_Handle_t* h, TMC2209_Dir_t dir, float rpm);
HAL_StatusTypeDef TMC2209_SetRPM(TMC2209_Handle_t *h, float rpm);
HAL_StatusTypeDef TMC2209_Start(TMC2209_Handle_t *h);
HAL_StatusTypeDef TMC2209_Stop(TMC2209_Handle_t *h);

/* config */
HAL_StatusTypeDef TMC2209_SetCurrentRaw(TMC2209_Handle_t *h, uint8_t ihold, uint8_t irun, uint8_t iholddelay);
HAL_StatusTypeDef TMC2209_SetMicrosteps(TMC2209_Handle_t *h, uint16_t microsteps);
HAL_StatusTypeDef TMC2209_ApplySilentSmoothProfile(TMC2209_Handle_t *h, uint16_t microsteps, uint8_t ihold, uint8_t irun, uint8_t iholddelay);

#ifdef __cplusplus
}
#endif

#endif
