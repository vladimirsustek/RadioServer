/*
 * eeprom_25aa_1024.c
 *
 *  Created on: Aug 28, 2022
 *      Author: 42077
 */


#include "eeprom_25aa1024.h"

/* STM32 typical SPI handle */
SPI_HandleTypeDef hspi1;

uint32_t SPI1_NCSactivate(void)
{
	HAL_GPIO_WritePin(SPI1_NCS_GPIO_Port, SPI1_NCS_Pin, GPIO_PIN_RESET);
	return 0u;
}

uint32_t SPI1_NCSdeactivate(void)
{
	HAL_GPIO_WritePin(SPI1_NCS_GPIO_Port, SPI1_NCS_Pin, GPIO_PIN_SET);
	return 0u;
}

uint32_t EEPROM_WriteOperation(uint8_t *pDataTx, uint16_t Size)
{
	if (NULL == pDataTx)
	{
		return (uint32_t)(-1);
	}

	return (uint32_t)HAL_SPI_Transmit(&hspi1, pDataTx, Size, HAL_MAX_DELAY);
}

uint32_t EEPROM_ReadOperation(uint8_t* pDataRx, int16_t Size)
{
	if (NULL == pDataRx)
	{
		return (uint32_t)(-1);
	}

	return (uint32_t)HAL_SPI_Receive(&hspi1, pDataRx, Size, HAL_MAX_DELAY);
}

uint32_t EEPROM_WriteEnable(void)
{
	uint8_t data = EEPROM_WREN;
	uint32_t result[3] = {0};

	result[0] = SPI1_NCSactivate();
	result[1] = EEPROM_WriteOperation(&data, sizeof(EEPROM_WREN));
	result[2] = SPI1_NCSdeactivate();

	if (0 == result[0] && 0 == result[1] && 0 == result[2])
		return 0u;
	else
		return (uint32_t)(-1);
}

uint32_t EEPROM_WriteDisaable(void)
{
	uint8_t data = EEPROM_WRDI;
	uint32_t result[3] = {0};

	result[0] = SPI1_NCSactivate();
	result[1] = EEPROM_WriteOperation(&data, sizeof(EEPROM_WRDI));
	result[2] = SPI1_NCSdeactivate();

	if (0 == result[0] && 0 == result[1] && 0 == result[2])
		return 0u;
	else
		return (uint32_t)(-1);
}

uint32_t EEPROM_ReadStatusRegister(uint8_t *ret)
{
	uint8_t dataTx[2] = {EEPROM_RDSR, 0};
	uint8_t dataRx[2] = {0};

	uint32_t result[4] = {0};

	if (NULL == ret)
	{
		return (uint32_t)(-1);
	}

	result[0] = SPI1_NCSactivate();
	//result[1] = EEPROM_WriteOperation(&dataTx, 1);
	//result[2] = EEPROM_ReadOperation(&dataRx, 1);
	HAL_SPI_TransmitReceive(&hspi1, dataTx, dataRx, 2, HAL_MAX_DELAY);
	result[3] = SPI1_NCSdeactivate();

	*ret = dataRx[1];

	if (0 == result[0] && 0 == result[1] && 0 == result[2] && 0 == result[3])
		return 0u;
	else
		return (uint32_t)(-1);
}


uint32_t EEPROM_WriteStatusRegister(uint8_t value)
{
	uint8_t dataTx[2] = {EEPROM_WRSR, value};

	uint32_t result[3] = {0};

	result[0] = SPI1_NCSactivate();
	result[1] = EEPROM_WriteOperation(dataTx, 2);
	result[2] = SPI1_NCSdeactivate();

	if (0 == result[0] && 0 == result[1] && 0 == result[2])
		return 0u;
	else
		return (uint32_t)(-1);
}

uint32_t EEPROM_Init(void)
{
	uint32_t result[2] = {0};

	/* Turn on the GPIOs powering the EEPROM module */
	//HAL_GPIO_WritePin(EEPROM_VCC_GPIO_Port, EEPROM_VCC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(EEPROM_GND_GPIO_Port, EEPROM_GND_Pin, GPIO_PIN_RESET);

	if (0 == result[0] && 0 == result[1])
		return 0u;
	else
		return (uint32_t)(-1);
}

static uint32_t EEPROM_WriteDataNoLogic(uint32_t address, uint8_t* pData, uint16_t Size)
{
	uint32_t result[5] = {0};
	uint8_t startWrite[4];

	if((address > EEPROM_STOP_ADDRESS) ||
			(address < EEPROM_START_ADDRESS) ||
			(Size > MAX_WRITE_AT_ONCE))
	{
		return (uint32_t)(-1);
	}

	startWrite[0] = EEPROM_WRITE;
	startWrite[1] = (address & 0x00FF0000) >> 16;
	startWrite[2] = (address & 0x0000FF00) >> 8;
	startWrite[3] = (address & 0x000000FF);

	result[0] = EEPROM_WriteEnable();
	result[1] = SPI1_NCSactivate();
	result[2] = EEPROM_WriteOperation(startWrite, 4);
	result[3] = EEPROM_WriteOperation(pData, Size);
	result[4] = SPI1_NCSdeactivate();

	if (0 == result[0] && 0 == result[1] && 0 == result[2] && 0 == result[3])
		return 0u;
	else
		return (uint32_t)(-1);

}

uint32_t EEPROM_WriteData(uint32_t address, uint8_t* pData, uint16_t size)
{
	uint8_t status = 0;
	uint16_t result = 0;

	uint16_t neareastAdress = address - (address % EEPROM_PAGE_SIZE);
	uint16_t offSet = address - neareastAdress;
	uint16_t bytesToWrite = size;
	uint16_t maxCycleSize = EEPROM_PAGE_SIZE - (address % EEPROM_PAGE_SIZE);
    uint16_t cycleSize = (bytesToWrite > maxCycleSize) ? maxCycleSize : bytesToWrite;

    /* Turn on the EEPROM and wait (just guessed 100ms )to start up*/
	HAL_GPIO_WritePin(EEPROM_VCC_GPIO_Port, EEPROM_VCC_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	while(bytesToWrite)
	{
		result += EEPROM_WriteDataNoLogic(
				(uint32_t)neareastAdress + offSet,
				pData + (size - bytesToWrite),
				cycleSize
				);

		HAL_Delay(10);
		result += EEPROM_ReadStatusRegister(&status);

		if(0 != status)
		{
			result = (uint16_t)(-1);
			break;
		}

	    bytesToWrite -= cycleSize;
	    neareastAdress += EEPROM_PAGE_SIZE;
	    address = neareastAdress;
	    offSet = address - neareastAdress;
	    maxCycleSize = EEPROM_PAGE_SIZE - (address % EEPROM_PAGE_SIZE);
	    cycleSize = (bytesToWrite > EEPROM_PAGE_SIZE) ? EEPROM_PAGE_SIZE : bytesToWrite;
	}

	result = (result != 0) ? -1 : 0;

    /* Turn off the EEPROM */
	HAL_GPIO_WritePin(EEPROM_VCC_GPIO_Port, EEPROM_VCC_Pin, GPIO_PIN_RESET);

	return result;
}


uint32_t EEPROM_ReadData(uint32_t address, uint8_t *pData, uint16_t Size)
{
	uint32_t result[5] = {0};
	uint8_t startWrite[5];

	if((address > EEPROM_STOP_ADDRESS) ||
			(address < EEPROM_START_ADDRESS) ||
			(Size > MAX_WRITE_AT_ONCE))
	{
		return (uint32_t)(-1);
	}

    /* Turn on the EEPROM and wait (just guessed 100ms )to start up*/
	HAL_GPIO_WritePin(EEPROM_VCC_GPIO_Port, EEPROM_VCC_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	startWrite[0] = EEPROM_READ;
	startWrite[1] = (address & 0x00FF0000) >> 16;
	startWrite[2] = (address & 0x0000FF00) >> 8;
	startWrite[3] = (address & 0x000000FF);

	result[1] = SPI1_NCSactivate();
	result[2] = EEPROM_WriteOperation(startWrite, 4);
	result[3] = EEPROM_ReadOperation(pData, Size);
	result[4] = SPI1_NCSdeactivate();

	if (0 == result[0] && 0 == result[1] && 0 == result[2] && 0 == result[3] && 0 == result[4])
		return 0u;
	else
		return (uint32_t)(-1);

    /* Turn off the EEPROM*/
	HAL_GPIO_WritePin(EEPROM_VCC_GPIO_Port, EEPROM_VCC_Pin, GPIO_PIN_RESET);

}
