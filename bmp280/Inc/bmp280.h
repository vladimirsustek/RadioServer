/*
 * bmp280.h
 *
 *  Created on: Apr 2, 2019
 *      Author: Vladik
 */

#ifndef PLATFORM_BMP280_BMP280_H_
#define PLATFORM_BMP280_BMP280_H_

#include "spi.h"

#define BMP280_FAILED_MEAS		(uint32_t)(0xFFFF0000)
#define BMP280_OK_MEAS			(uint32_t)(0u)

#define BMP280_KIT_ADDR				(uint8_t)(0x76)

#define BMP280_CAL_T1_MSB			(uint8_t)(0x88)
#define BMP280_CAL_T2_MSB			(uint8_t)(0x8A)
#define BMP280_CAL_T3_MSB			(uint8_t)(0x8C)

#define BMP280_CAL_P1_MSB			(uint8_t)(0x8E)
#define BMP280_CAL_P2_MSB			(uint8_t)(0x90)
#define BMP280_CAL_P3_MSB			(uint8_t)(0x92)
#define BMP280_CAL_P4_MSB			(uint8_t)(0x94)
#define BMP280_CAL_P5_MSB			(uint8_t)(0x96)
#define BMP280_CAL_P6_MSB			(uint8_t)(0x98)
#define BMP280_CAL_P7_MSB			(uint8_t)(0x9A)
#define BMP280_CAL_P8_MSB			(uint8_t)(0x9C)
#define BMP280_CAL_P9_MSB			(uint8_t)(0x9E)

#define BMP280_TEMP_MSB				(uint8_t)(0xFA)
#define BMP280_TEMP_LSB				(uint8_t)(0xFB)
#define BMP280_TEMP_XLSB			(uint8_t)(0xFC)

#define BMP280_PRESS_MSB			(uint16_t)(0xF7)
#define BMP280_PRESS_LSB			(uint16_t)(0xF8)
#define BMP280_PRESS_XLSB			(uint16_t)(0xF9)

#define BMP280_CHIP_ID				(uint8_t)(0xD0);

#define BMP280_CTRL_MEAS_REG_ADR	(uint8_t)(0xF4)
#define BMP280_CTRL_MEAS_OSRS_T_2	(uint8_t)(7)
#define BMP280_CTRL_MEAS_OSRS_T_1	(uint8_t)(6)
#define BMP280_CTRL_MEAS_OSRS_T_0	(uint8_t)(5)
#define BMP280_CTRL_MEAS_OSRS_P_2	(uint8_t)(4)
#define BMP280_CTRL_MEAS_OSRS_P_1	(uint8_t)(3)
#define BMP280_CTRL_MEAS_OSRS_P_0	(uint8_t)(2)
#define BMP280_CTRL_MEAS_MODE_1		(uint8_t)(1)
#define BMP280_CTRL_MEAS_MODE_0		(uint8_t)(0)

#define BMP280_CONFIG_REG_ADR		(uint8_t)(0xF5)
#define BMP280_CONFIG_T_SB_2		(uint8_t)(7)
#define BMP280_CONFIG_T_SB_1		(uint8_t)(6)
#define BMP280_CONFIG_T_SB_0		(uint8_t)(5)
#define BMP280_CONFIG_FILTER_2		(uint8_t)(4)
#define BMP280_CONFIG_FILTER_1		(uint8_t)(3)
#define BMP280_CONFIG_FILTER_0		(uint8_t)(2)
#define BMP280_CONFIG_SPIEN			(uint8_t)(0)

#define BMP280_SPI_WR_ADR(ADR)		((uint8_t)(0x7F) & (uint8_t)(ADR))
#define BMP280_SPI_RD_ADR(ADR)		(((uint8_t)(0x7F) & (uint8_t)(ADR)) | (uint8_t)(0x80))

int32_t bmp280_start_press_temp(void);
int32_t bmp280_get_calib_temp(void);
int32_t bmp280_get_calib_press(void);
int32_t bmp280_get_temp_press(int32_t* temp, int32_t* press);
void BMP280_SensorInit(void);
#endif /* PLATFORM_BMP280_BMP280_H_ */
