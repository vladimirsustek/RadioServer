/*
 * bmp280.c
 *
 *  Created on: Apr 2, 2019
 *      Author: Vladimir Sustek */

#include "bmp280.h"

/* Static variables representing manufacture calibration values
 * of the temperature measurement - must not be changed anyway ! */
static int32_t dig_T1 = 0;
static int32_t dig_T2 = 0;
static int32_t dig_T3 = 0;

/* Static variables representing manufacture calibration values
 * of the pressure measurement - must not be changed anyway ! */
static int32_t dig_P1 = 0;
static int32_t dig_P2 = 0;
static int32_t dig_P3 = 0;
static int32_t dig_P4 = 0;
static int32_t dig_P5 = 0;
static int32_t dig_P6 = 0;
static int32_t dig_P7 = 0;
static int32_t dig_P8 = 0;
static int32_t dig_P9 = 0;

#define ACTIVATE_CS() 		(HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_RESET))
#define DEACTIVATE_CS()		(HAL_GPIO_WritePin(BMP_CS_GPIO_Port, BMP_CS_Pin, GPIO_PIN_SET))

static uint32_t bmp280_spi_trx(uint8_t* pTx, uint8_t* pRx, uint8_t lng)
{
	uint32_t result = 0;

	ACTIVATE_CS();
	result = HAL_SPI_TransmitReceive(&hspi1, pTx, pRx, lng, HAL_MAX_DELAY);
	DEACTIVATE_CS();

	return result;
}

/* Fine calculated temperature used in formula to obtain pressure */
static int32_t T_Fine = 0;

/*!
 * @brief  Start measurement with BMP280 temperature + pressure sensor
 *
 * @return       result 0 in case of correct function processing
 *               other than 0 signalizes error
 *
 * Function sets the lowest precision (0.005 �C) of the temperature and lowest pressure
 * precision (2.62 Pa) in normal mode (auto triggered measurement).  Then the standby-time
 * is set (measurement period) to 500ms. In case of other settings change also this comment.
 *
 */
int32_t bmp280_start_press_temp(void) {

	/* Variables to set BMP280's registers */
	uint8_t ctrl_meas = 0;
	uint8_t config = 0;

	/* Function result*/
	int32_t result = 0;

	/* SPI sub-variables */
    uint8_t tx[2];
    uint8_t rx[2];

	/* Set the address to KIT-Fixed adddress*/
	//result += (int32_t)i2c_slave_addr(BMP280_KIT_ADDR);

	/* Set the lowest precision (0.005 degrses C) of the temperature and
	 * lowest pressure precision (2.62 Pa) in normal mode (auto triggered measurement) */
	ctrl_meas = (1 << BMP280_CTRL_MEAS_OSRS_T_2) |
			(1 << BMP280_CTRL_MEAS_OSRS_T_0)|(1 << BMP280_CTRL_MEAS_OSRS_P_0) |
			(1 << BMP280_CTRL_MEAS_MODE_1)|(1 << BMP280_CTRL_MEAS_MODE_0);
	/* Set the standby-time (measurement period) to 500ms*/
	config = (1 << BMP280_CONFIG_T_SB_2);


	/***************************************************************************************************/
	/***************************************************************************************************/

	/* 2x I2C transaction, at the first BMP280_CONFIG_REG_ADR is modified using config value*/

    tx[0] = BMP280_SPI_WR_ADR(BMP280_CONFIG_REG_ADR);
    tx[1] = config;

    rx[0] = 0;
    rx[1] = 0;

    result += bmp280_spi_trx(tx, rx, 2);

	/* Secondly, BMP280_CTRL_MEAS_REG_ADR is modified using ctrl_meas value*/

    tx[0] = BMP280_SPI_WR_ADR(BMP280_CTRL_MEAS_REG_ADR);
    tx[1] = ctrl_meas;

    rx[0] = 0;
    rx[1] = 0;

    result += bmp280_spi_trx(tx, rx, 2);
	/***************************************************************************************************/
	/***************************************************************************************************/

    return result;
}

/*!
 * @brief  Get vendor-defined manufacter calibration constants - temperature
 *
 * @return       result 0 in case of correct function processing
 *               other than 0 signalizes error
 *
 * Function reads in total 3 constants to used in further temperature
 * measurement: dig_T1, dig_T2, dig_T3. Not intended to change.
 *
 * @ Calibration values does not need to be read more than once (during start-up)
 *
 */
int32_t bmp280_get_calib_temp(void) {

	/* Function result*/
	int32_t result = 0;

	uint8_t tx[3];
	uint8_t rx[3];

	/***************************************************************************************************/
	/***************************************************************************************************/

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_T1_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_T1 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_T2_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_T2 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_T3_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_T3 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    return result;

}
/*!
 * @brief  Get vendor-defined manufacter calibration constants - pressure
 *
 * @return       result 0 in case of correct function processing
 *               other than 0 signalizes error
 *
 * Function reads in total 3 constants to used in further temperature
 * measurement: dig_P1 up to, dig_P9. Not intended to change.
 *
 * @ Calibration values does not need to be read more than once (during start-up)
 *
 *
 */
int32_t bmp280_get_calib_press(void) {

	/* Todo check calibration correctness */

	/* Function result*/
	int32_t result = 0;

	uint8_t rx[3];
	uint8_t tx[3];


	/***************************************************************************************************/
	/***************************************************************************************************/

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P1_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P1 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P2_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P2 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P3_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P3 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P4_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P4 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P5_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P5 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P6_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P6 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P7_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P7 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P8_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P8 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_CAL_P9_MSB);
    tx[1] = 0;
    tx[2] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 3);
    dig_P9 = (int32_t)(rx[2] << 8) + (int32_t)(rx[1]);

    return result;

}

/*!
 * @brief  Get temperature and pressure by BMP280 sensor
 *
 * @param [in] temp - pointer on the int32_t number to store measured temperature
 * @param [in] press - pointer on the int32_t number to store measured pressure
 *
 * @return       result 0 in case of correct function processing
 *               other than 0 signalizes error
 *
 * Function reads temperature and pressure registers of the BMP280 and using
 * previously obtained calibration values dig_T1 up to dig_T3 and dig_P1 up to
 * dig_P9 calculates real temperature and pressure.
 *
 * @note To get correct values functions bmp280_get_calib_temp and bmp280_get_calib_press
 * have to be called before this function. Function sets slave I2C address of the device
 * on its own - does not need to be handled by user.
 *
 * @sa bmp280_get_calib_temp, bmp280_get_calib_press
 *
 */
int32_t bmp280_get_temp_press(int32_t* temp, int32_t* press) {

	/* Todo check pressure measurement-calculation correctness */

	int32_t result = 0;

	if(temp == NULL || press == NULL) {
		return BMP280_FAILED_MEAS;
	}

	/* Variables necessary for I2C transaction*/
	uint8_t rx[4];
	uint8_t tx[4];

	/* Function result*/
	int32_t uncomp_temp = 0;
	int32_t uncomp_press = 0;
	int32_t comp_press = 0;
	int32_t accu1 = 0;
	int32_t accu2 = 0;
	int32_t accu3 = 0;

    tx[0] = BMP280_SPI_RD_ADR(BMP280_TEMP_MSB);
    tx[1] = 0;
    tx[2] = 0;
    tx[3] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;
    tx[3] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 4);
	uncomp_temp = ((uint32_t)rx[1] << 12) | ((uint32_t)rx[2] << 4) | ((uint32_t)rx[3] >> 4);

    tx[0] = BMP280_SPI_RD_ADR(BMP280_PRESS_MSB);
    tx[1] = 0;
    tx[2] = 0;
    tx[3] = 0;

    rx[0] = 0;
    rx[1] = 0;
    rx[2] = 0;
    tx[3] = 0;

    /* Do SPI read transaction and store received data to calibration value */
    result += (int32_t)bmp280_spi_trx(tx, rx, 4);
    uncomp_press = (int32_t) ((((int32_t) (rx[1])) << 12) | (((int32_t) (rx[2])) << 4) | (((int32_t) (rx[3])) >> 4));

    if(!result) {

    	/***************************************************************************************************/
    	/***************************************************************************************************/

	    accu1 = ((((uncomp_temp / 8) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) / 2048;

	    accu2 = (((((uncomp_temp / 16) - ((int32_t)dig_T1)) *
        		((uncomp_temp / 16) - ((int32_t)dig_T1))) / 4096) *
             ((int32_t)dig_T3)) / 16384;

        accu3 = accu1 + accu2;

        T_Fine = accu3;

        *temp = (accu3 * 5 + 128) / 256;

    	/***************************************************************************************************/
    	/***************************************************************************************************/

        accu1 = (((int32_t)T_Fine) / 2) - (int32_t) 64000;
        accu2 = (((accu1 / 4) * (accu1 / 4)) / 2048) * ((int32_t)dig_P6);
        accu2 = accu2 + ((accu1 * ((int32_t)dig_P5)) * 2);
        accu2 = (accu2 / 4) + (((int32_t)dig_P4) * 65536);
        accu1 =
            (((dig_P3 * (((accu1 / 4) * (accu1 / 4)) / 8192)) / 8) +
             ((((int32_t)dig_P2) * accu1) / 2)) / 262144;
        accu1 = ((((32768 + accu1)) * ((int32_t)dig_P1)) / 32768);

        comp_press = (((uint32_t) (((int32_t)1048576) - uncomp_press) - (accu2 / 4096))) * 3125;

        /* Avoid exception caused by division with zero */
        if (accu1 != 0)
        {
            /* Check for overflows against UINT32_MAX/2; if pres is left-shifted by 1 */
            if (comp_press < 0x80000000)
            {
                comp_press = (comp_press << 1) / ((uint32_t) accu1);
            }
            else
            {
                comp_press = (comp_press / (uint32_t) accu1) * 2;
            }
            accu1 = (((int32_t)dig_P9) * ((int32_t) (((comp_press / 8) * (comp_press / 8)) / 8192))) /
                   4086;
            accu2 = (((int32_t) (comp_press / 4)) * ((int32_t)dig_P8)) / 8192;
            comp_press = (uint32_t) ((int32_t) comp_press + ((accu1 + accu2 +dig_P7) / 16));

            *press = comp_press;
        }
        else {
            	result = BMP280_FAILED_MEAS;
        }

    } else {
    	result = BMP280_FAILED_MEAS;
    }

	return result;
}

void BMP280_SensorInit(void)
{
	  bmp280_start_press_temp();
	  bmp280_get_calib_temp();
	  bmp280_get_calib_press();
}


