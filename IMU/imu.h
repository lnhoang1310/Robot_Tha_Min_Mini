#ifndef IMU_H
#define IMU_H

#include "stm32f1xx.h"
#include "i2c.h"

#ifndef PI
#define PI 3.14
#endif

#define IMU_ADDRESS (0x28 << 1)
#define IMU_PAGE_ID (0x07)
#define IMU_OPR_MODE (0x3D)
#define IMU_UNIT_SEL (0x3B)
#define IMU_ACC_DATA_X_LSB (0x08)
#define IMU_GYR_DATA_X_LSB (0x14)
#define IMU_EUL_HEADING_LSB (0x1A)
#define IMU_CALIB_STAT (0x35)
#define IMU_ACC_OFFSET_X_LSB (0x55)
#define IMU_SYS_TRIGGER (0x3F)
#define IMU_PWR_MODE (0x3E)
#define IMU_OFFSET_DATA_LENGTH (22)

typedef enum
{
	IMU_OK,
	IMU_ERROR,
	IMU_WAIT
} IMU_State;

typedef enum
{
	CALIB_ERROR,
	CALIB_OK,
	CALIB_WAIT
} Calib_State;

typedef struct
{
	I2C_HandleTypeDef *hi2c;
	uint16_t address;
	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;
	float roll;
	float pitch;
	float yaw;
	uint8_t sys_calib;
	uint8_t gyro_calib;
	uint8_t accel_calib;
	uint8_t mag_calib;
} IMU_Typedef;

IMU_State IMU_Init(IMU_Typedef *imu, I2C_HandleTypeDef *_hi2c, uint16_t _address);
IMU_State IMU_GetDataRaw(IMU_Typedef *imu);
Calib_State IMU_CheckCalib(IMU_Typedef *imu);
IMU_State IMU_Calibrate(IMU_Typedef *imu, uint8_t *calib_data);
IMU_State IMU_Read_Register(IMU_Typedef *imu, uint8_t reg, uint8_t *data, uint8_t len);

#endif // IMU_H
