#include "tmc2209.h"

#define TIMER_CLK_HZ SystemCoreClock
#define MOTOR_FULL_STEPS_PER_REV 200

static uint8_t tmc2209_crc8(const uint8_t *data, uint8_t len){
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t current = data[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (((crc >> 7) & 0x01) ^ (current & 0x01)) {
                crc = (uint8_t)((crc << 1) ^ 0x07);
            } else {
                crc = (uint8_t)(crc << 1);
            }
            current >>= 1;
        }
    }
    return crc;
}

static HAL_StatusTypeDef TMC2209_WriteReg(TMC2209_Handle_t *h, uint8_t reg, uint32_t data){
    uint8_t frame[8];

    frame[0] = 0x05;
    frame[1] = h->slave_addr & 0x03;
    frame[2] = reg | 0x80;
    frame[3] = (uint8_t)((data >> 24) & 0xFF);
    frame[4] = (uint8_t)((data >> 16) & 0xFF);
    frame[5] = (uint8_t)((data >> 8)  & 0xFF);
    frame[6] = (uint8_t)(data & 0xFF);
    frame[7] = tmc2209_crc8(frame, 7);

    return HAL_UART_Transmit(h->huart, frame, 8, 100);
}

HAL_StatusTypeDef TMC2209_Init(TMC2209_Handle_t *h, UART_HandleTypeDef *huart,
                  TIM_HandleTypeDef *htim_pwm, uint32_t tim_channel,
                  GPIO_TypeDef *dir_port, uint16_t dir_pin,
                  GPIO_TypeDef *en_port,  uint16_t en_pin,
                  uint8_t slave_addr, uint16_t microstep)
{
    h->huart = huart;
    h->htim = htim_pwm;
    h->tim_channel = tim_channel;

    h->dir_port = dir_port;
    h->dir_pin  = dir_pin;
    h->en_port  = en_port;
    h->en_pin   = en_pin;

    h->slave_addr = slave_addr & 0x03;
    h->en_active_low = 1;
    h->dir_inverted = 0;
    h->running = 0;
	h->velocity = 0.0f;
	h->direction = TMC2209_DIR_FORWARD;
	
	HAL_StatusTypeDef status = TMC2209_SetMicrosteps(h, microstep);
	if(status != HAL_OK) h->microsteps = 1;
	return status;
}

void TMC2209_SetPinPolarity(TMC2209_Handle_t *h, uint8_t en_active_low, uint8_t dir_inverted){
    h->en_active_low = en_active_low;
    h->dir_inverted  = dir_inverted;
}

void TMC2209_Enable(TMC2209_Handle_t *h){
    HAL_GPIO_WritePin(h->en_port, h->en_pin, h->en_active_low ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void TMC2209_Disable(TMC2209_Handle_t *h){
    HAL_GPIO_WritePin(h->en_port, h->en_pin, h->en_active_low ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void TMC2209_SetDirection(TMC2209_Handle_t *h, TMC2209_Dir_t dir){
    GPIO_PinState state = (dir == TMC2209_DIR_FORWARD) ? GPIO_PIN_RESET : GPIO_PIN_SET;
	h->direction = dir;
    if (h->dir_inverted) {
        state = (state == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    }

    HAL_GPIO_WritePin(h->dir_port, h->dir_pin, state);
}

HAL_StatusTypeDef TMC2209_SetRPM(TMC2209_Handle_t *h, float rpm){
    if (h == NULL) return HAL_ERROR;
    if (rpm < 0.0f) return HAL_ERROR;

    uint32_t steps_per_rev = MOTOR_FULL_STEPS_PER_REV * h->microsteps;
    if (steps_per_rev == 0U) return HAL_ERROR;

    uint32_t step_hz = (uint32_t)((rpm * (float)steps_per_rev) / 60.0f);
    if (step_hz == 0U) step_hz = 1U;

    uint32_t prescaler = (TIMER_CLK_HZ / 1000000U);
    if (prescaler == 0U) prescaler = 1U;
    prescaler -= 1U;

    uint32_t timer_base = TIMER_CLK_HZ / (prescaler + 1U);
    if (timer_base < step_hz) return HAL_ERROR;

    uint32_t arr = timer_base / step_hz;
    if (arr == 0U) arr = 1U;
    arr -= 1U;

    h->htim->Instance->PSC = prescaler;
    __HAL_TIM_SET_AUTORELOAD(h->htim, arr);
    __HAL_TIM_SET_COMPARE(h->htim, h->tim_channel, (arr + 1U) / 2U);

    return HAL_OK;
}

HAL_StatusTypeDef TMC2209_Control(TMC2209_Handle_t* h, TMC2209_Dir_t dir, float rpm){
	TMC2209_SetDirection(h, dir);
	HAL_StatusTypeDef status = TMC2209_SetRPM(h, rpm);
	if(status != HAL_OK) return status;
	return TMC2209_Start(h);
}

HAL_StatusTypeDef TMC2209_Start(TMC2209_Handle_t *h){
	TMC2209_Enable(h);
    if (HAL_TIM_PWM_Start(h->htim, h->tim_channel) != HAL_OK) {
        return HAL_ERROR;
    }
    h->running = 1;
    return HAL_OK;
}

HAL_StatusTypeDef TMC2209_Stop(TMC2209_Handle_t *h){
	if(!h->running) return HAL_OK;
	TMC2209_Disable(h);
    if (HAL_TIM_PWM_Stop(h->htim, h->tim_channel) != HAL_OK) {
        return HAL_ERROR;
    }
    h->running = 0;
    return HAL_OK;
}

HAL_StatusTypeDef TMC2209_SetCurrentRaw(TMC2209_Handle_t *h, uint8_t ihold, uint8_t irun, uint8_t iholddelay){
    uint32_t reg = 0;

    reg |= ((uint32_t)(ihold      & 0x1F) << 0);
    reg |= ((uint32_t)(irun       & 0x1F) << 8);
    reg |= ((uint32_t)(iholddelay & 0x0F) << 16);

    return TMC2209_WriteReg(h, TMC2209_REG_IHOLD_IRUN, reg);
}

HAL_StatusTypeDef TMC2209_SetMicrosteps(TMC2209_Handle_t *h, uint16_t microsteps){
    uint8_t mres;

    switch (microsteps) {
        case 256: mres = 0; break;
        case 128: mres = 1; break;
        case 64:  mres = 2; break;
        case 32:  mres = 3; break;
        case 16:  mres = 4; break;
        case 8:   mres = 5; break;
        case 4:   mres = 6; break;
        case 2:   mres = 7; break;
        case 1:   mres = 8; break;
        default:  return HAL_ERROR;
    }

    uint32_t chopconf = 0;
    chopconf |= (1UL << 28);               /* intpol */
    chopconf |= ((uint32_t)mres << 24);    /* MRES */
    chopconf |= (2UL << 15);               /* TBL = 2 */
    chopconf |= (0UL << 7);                /* HEND = 0 */
    chopconf |= (3UL << 4);                /* HSTRT code 3 => value 4 */
    chopconf |= (5UL << 0);                /* TOFF = 5 */

    HAL_StatusTypeDef status = TMC2209_WriteReg(h, TMC2209_REG_CHOPCONF, chopconf);
	if(status == HAL_OK) h->microsteps = microsteps;
	return status;
}

/* ---------------- Silent + Smooth profile ---------------- */
HAL_StatusTypeDef TMC2209_ApplySilentSmoothProfile(TMC2209_Handle_t *h, uint16_t microsteps, uint8_t ihold, uint8_t irun, uint8_t iholddelay){
    HAL_StatusTypeDef st;
    uint32_t gconf = 0;
    uint32_t pwmconf = 0;

    gconf |= (1UL << 6);
    gconf |= (1UL << 7);
    gconf |= (1UL << 8);

    st = TMC2209_WriteReg(h, TMC2209_REG_GCONF, gconf);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    st = TMC2209_SetCurrentRaw(h, ihold, irun, iholddelay);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    st = TMC2209_SetMicrosteps(h, microsteps);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    pwmconf |= (36UL << 0);    /* PWM_OFS   */
    pwmconf |= (8UL  << 8);    /* PWM_GRAD  */
    pwmconf |= (1UL  << 16);   /* PWM_FREQ  */
    pwmconf |= (1UL  << 18);   /* pwm_autoscale */
    pwmconf |= (1UL  << 19);   /* pwm_autograd  */
    pwmconf |= (8UL  << 24);   /* PWM_REG */
    pwmconf |= (12UL << 28);   /* PWM_LIM */

    st = TMC2209_WriteReg(h, TMC2209_REG_PWMCONF, pwmconf);
    if (st != HAL_OK) return st;
    HAL_Delay(2);

    /* Gi?m d?ng khi ð?ng yên sau 20 ðõn v? th?i gian n?i */
    st = TMC2209_WriteReg(h, TMC2209_REG_TPOWERDOWN, 20);
    if (st != HAL_OK) return st;

    return HAL_OK;
}
