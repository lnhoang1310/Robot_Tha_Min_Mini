#include "imu.h"
#include "main.h"

uint8_t chip_id = 0;
uint8_t calib_stat;
uint8_t calibration_step = 0;
Calib_State calib_state = CALIB_WAIT;
uint8_t check_t = 0;

IMU_State IMU_Write_Register(IMU_Typedef *imu, uint8_t reg, uint8_t data){
    if (HAL_I2C_Mem_Write(imu->hi2c, IMU_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000) != HAL_OK)
        return IMU_ERROR;
    return IMU_OK;
}

IMU_State IMU_Read_Register(IMU_Typedef *imu, uint8_t reg, uint8_t *data, uint8_t len){
    if (HAL_I2C_Mem_Read(imu->hi2c, imu->address, reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000) != HAL_OK)
        return IMU_ERROR;
    return IMU_OK;
}

IMU_State IMU_Init(IMU_Typedef *imu, I2C_HandleTypeDef *_hi2c, uint16_t _address){

    imu->hi2c = _hi2c;
    imu->address = _address;
    imu->sys_calib = 0;
    imu->gyro_calib = 0;
    imu->accel_calib = 0;
    imu->mag_calib = 0;

    // Check chip ID
    if (IMU_Read_Register(imu, 0x00, &chip_id, 1) != IMU_OK || chip_id != 0xA0){
		check_t = 1;
        return IMU_ERROR;
    }

    // Switch to config mode
    if (IMU_Write_Register(imu, IMU_OPR_MODE, 0x00) != IMU_OK){
		check_t = 2;
        return IMU_ERROR;
    }
    HAL_Delay(20);

    // Reset sensor
    if (IMU_Write_Register(imu, IMU_SYS_TRIGGER, 0x20) != IMU_OK){
		check_t = 3;
        return IMU_ERROR;
    }
    HAL_Delay(1000);

    // Set power mode to normal
    if (IMU_Write_Register(imu, IMU_PWR_MODE, 0x00) != IMU_OK){
		check_t = 4;
        return IMU_ERROR;
    }
    HAL_Delay(10);

    // Set page 0
    if (IMU_Write_Register(imu, IMU_PAGE_ID, 0x00) != IMU_OK){
		check_t = 5;
        return IMU_ERROR;
    }
    HAL_Delay(10);

    // Turn on sensor
    if (IMU_Write_Register(imu, IMU_SYS_TRIGGER, 0x00) != IMU_OK){
		check_t = 6;
        return IMU_ERROR;
    }

    // Set NDOF mode
    if (IMU_Write_Register(imu, IMU_OPR_MODE, 0x0C) != IMU_OK){
		check_t = 7;
        return IMU_ERROR;
    }
    HAL_Delay(600);

    // Set default units (degrees, m/s^2, deg/s)
    if (IMU_Write_Register(imu, IMU_UNIT_SEL, 0x00) != IMU_OK){
		check_t = 8;
        return IMU_ERROR;
    }
    HAL_Delay(10);

    return IMU_OK;
}
IMU_State IMU_GetDataRaw(IMU_Typedef *imu){
    uint8_t data[24];
    if (IMU_Read_Register(imu, IMU_ACC_DATA_X_LSB, data, 24) != IMU_OK) return IMU_ERROR;

    // acceleration (ax, ay, az)
    imu->ax = (int16_t)(data[1] << 8 | data[0]) / 100.0f;
    imu->ay = (int16_t)(data[3] << 8 | data[2]) / 100.0f;
    imu->az = (int16_t)(data[5] << 8 | data[4]) / 100.0f;

    // Gyroscope (gx, gy, gz)
    imu->roll = (int16_t)(data[21] << 8 | data[20]) * PI / 2880.0f;
    imu->pitch = (int16_t)(data[23] << 8 | data[22]) * PI / 2880.0f;
    imu->yaw = (int16_t)(data[19] << 8 | data[18]) * PI / 2880.0f;

    // Euler Angle (yaw, roll, pitch)
    imu->gx = (int16_t)(data[13] << 8 | data[12]) * PI / 2880.0f;
    imu->gy = (int16_t)(data[15] << 8 | data[14]) * PI / 2880.0f;
    imu->gz = (int16_t)(data[17] << 8 | data[16]) * PI / 2880.0f;

    return IMU_OK;
}

Calib_State IMU_CheckCalib(IMU_Typedef *imu){

    if (IMU_Read_Register(imu, IMU_CALIB_STAT, &calib_stat, 1) != IMU_OK) return CALIB_ERROR;
    imu->sys_calib = (calib_stat >> 6) & 0x03;
    imu->gyro_calib = (calib_stat >> 4) & 0x03;
    imu->accel_calib = (calib_stat >> 2) & 0x03;
    imu->mag_calib = calib_stat & 0x03;
    return CALIB_OK;
}

IMU_State IMU_Calibrate(IMU_Typedef *imu, uint8_t *calib_data){
    if (imu == NULL) return IMU_ERROR;

    // Set Config Mode
    if (IMU_Write_Register(imu, IMU_OPR_MODE, 0x00) != IMU_OK) return IMU_ERROR;
    HAL_Delay(20);

    // Validate Page 0
    if (IMU_Write_Register(imu, IMU_PAGE_ID, 0x00) != IMU_OK) return IMU_ERROR;
    HAL_Delay(10);

    // Chuy?n sang ch? d? NDOF d? b?t d?u hi?u chu?n
    if (IMU_Write_Register(imu, IMU_OPR_MODE, 0x0C) != IMU_OK) return IMU_ERROR;
    HAL_Delay(600);

    while (imu->sys_calib != 3 || imu->gyro_calib != 3 || imu->accel_calib != 3 || imu->mag_calib != 3){
        if (IMU_CheckCalib(imu) != CALIB_OK)
            return IMU_ERROR;
        switch (calibration_step){
        case 0: // Hi?u chu?n con quay
            if (imu->gyro_calib == 3)
                calibration_step = 1;
            break;
        case 1: // Hi?u chu?n gia t?c k?
            if (imu->accel_calib == 3)
                calibration_step = 2;
            break;
        case 2: // Hi?u chu?n t? k?
            if (imu->mag_calib == 3)
                calibration_step = 3;
            break;
        case 3: // Ki?m tra h? th?ng
            if (imu->sys_calib == 3)
                calibration_step = 0;
            break;
        }
        HAL_Delay(1000);
    }

    // Luu d? li?u hi?u chu?n v?o NVM
    if (IMU_Write_Register(imu, IMU_OPR_MODE, 0x00) != IMU_OK) return IMU_ERROR;
    HAL_Delay(20);

    // ??c d? li?u hi?u chu?n
    if (IMU_Read_Register(imu, IMU_ACC_OFFSET_X_LSB, calib_data, IMU_OFFSET_DATA_LENGTH) != IMU_OK) return IMU_ERROR;

    // Luu v?o NVM
    uint8_t sys_trigger = 0x20; // Bit 5 = 1 d? luu v?o NVM
    if (IMU_Write_Register(imu, IMU_SYS_TRIGGER, sys_trigger) != IMU_OK) return IMU_ERROR;
    HAL_Delay(50);

    // Chuy?n l?i ch? d? NDOF
    if (IMU_Write_Register(imu, IMU_OPR_MODE, 0x0C) != IMU_OK) return IMU_ERROR;
    HAL_Delay(20);

    // Ki?m tra tr?ng th?i hi?u chu?n sau khi luu
    if (IMU_CheckCalib(imu) != CALIB_OK) return IMU_ERROR;

    return IMU_OK;
}
