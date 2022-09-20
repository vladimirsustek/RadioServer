/*
 * eeprom_25aa_1024.c
 *
 *  Created on: Aug 28, 2022
 *      Author: 42077
 */
#include "eeprom_25aa1024.h"

/* STM32 typical SPI handle */
SPI_HandleTypeDef hspi1;

#define ESP_RESULT_CUSTOM 		(uint16_t)(0x5A5A)
#define EEPROM_R_ERROR			(char*)("EEPROM_R_ERROR\r\n")
#define EEPROM_W_ERROR			(char*)("EEPROM_W_ERROR\r\n")

#define ESP_CMD_ADR 			(uint8_t*)"ADR_"
#define ESP_CMD_LNG 			(uint8_t*)"LNG_"

#define EEPROM_W_CMD_OFFSET		(uint16_t)(strlen("EE_WRBT_ADR_000000_"))
#define EEPROM_R_CMD_OFFSET		(uint16_t)(strlen("EE_RDBT_ADR_000000_LNG_000000_"))

#define IS_NUM(x)   ((x) >= '0' && (x) <= '9')

#ifndef CMD_CUSTOM
#define CMD_CUSTOM			((uint16_t)(0x5A5A))
#endif

static uint32_t ExtractValue(const uint8_t* const pKeyWord,
		const uint8_t* const pBUff,
		const uint32_t buffLng,
		uint32_t* val);

sys_state_t systemGlobalState = {0};

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
	/*mocked*/
	return 0u;
}

/*Write across the 256-byte page not handled */
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

	uint32_t neareastAdress = address - (address % EEPROM_PAGE_SIZE);
	uint32_t offSet = address - neareastAdress;
	uint32_t maxCycleSize = EEPROM_PAGE_SIZE - (address % EEPROM_PAGE_SIZE);

	uint16_t bytesToWrite = size;
    uint16_t cycleSize = (bytesToWrite > maxCycleSize) ? maxCycleSize : bytesToWrite;

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

	return result;
}


uint32_t EEPROM_ReadData(uint32_t address, uint8_t *pData, uint16_t Size)
{
	uint8_t startWrite[4];
	uint32_t result[5] = {0};

	if((address > EEPROM_STOP_ADDRESS) ||
			(address < EEPROM_START_ADDRESS) ||
			(Size > MAX_WRITE_AT_ONCE))
	{
		return (uint32_t)(-1);
	}

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

}


uint16_t Cmd25AA1024WrBytes(const uint8_t* const cmd, const uint16_t lng)
{
	// expected e.g. EEPROM_W_ADR_000000_HelloWorld

	uint16_t payloadLng = lng - EEPROM_W_CMD_OFFSET;

	uint32_t addr = 0;
	uint32_t subResult = (uint32_t)(-1);

	ExtractValue(ESP_CMD_ADR, cmd, lng, &addr);

	subResult = EEPROM_WriteData(
			addr,
			(uint8_t*)cmd + EEPROM_W_CMD_OFFSET,
			payloadLng
			);

	if(0 == subResult)
	{
    	printf("EEPROM_W address %ld = 0x%06lx : %d bytes\r\n",
    			addr,
				addr,
				lng - EEPROM_W_CMD_OFFSET);

	}
	else
	{
    	printf(EEPROM_W_ERROR);
	}

	subResult = (subResult == 0) ? CMD_CUSTOM : (uint16_t)(-1);

	return subResult;
}

uint16_t Cmd25AA1024RdBytes(const uint8_t* const cmd, const uint16_t lng)
{
	// expected e.g. EEPROM_R_ADR_000000_LNG_000012 (To read HelloWorld\r\n)
	// if was previously sucessfully done EEPROM_W_ADR_000000_HelloWorld\r\n

	uint8_t auxBuff[EEPROM_PAGE_SIZE] = {0};

	uint16_t subLng = 0;
	uint32_t addr = 0;
	uint32_t readLng = 0;
	uint32_t subResult = (uint32_t)(-1);

	ExtractValue(ESP_CMD_ADR, cmd, lng, &addr);
	ExtractValue(ESP_CMD_LNG, cmd, lng, &readLng);

	subLng = (readLng > EEPROM_PAGE_SIZE) ? EEPROM_PAGE_SIZE : readLng;

	while(readLng)
	{
		subResult = EEPROM_ReadData(addr, auxBuff, subLng);
    	readLng -= subLng;
    	printf("%s\r\n", (char*)auxBuff);
    	HAL_Delay(100);
	}

	if(0 != subResult)
	{
    	printf(EEPROM_R_ERROR);
	}

	subResult = (subResult == 0) ? CMD_CUSTOM : (uint16_t)(-1);

	return subResult;
}

uint8_t* EEPROM_GetWIfi(uint32_t address, uint32_t offset, uint8_t* pSSIDpassword)
{
	uint8_t* result = NULL;
	uint8_t quotationMarkCnt = 0;

	uint32_t subResult = (uint32_t)(-1);
	uint16_t idx = 0;

	if(offset <= 32)
	{
		subResult = EEPROM_ReadData(address, pSSIDpassword + offset, EEPROM_PAGE_SIZE/2);

		for(idx = 0; idx < EEPROM_PAGE_SIZE/2; idx++)
		{
			if (pSSIDpassword[idx + offset] == '\"')
			{
				quotationMarkCnt++;
			}
			if(4 == quotationMarkCnt)
			{
				idx++;
				break;
			}
		}

	}

	if (0 == subResult && 4 == quotationMarkCnt)
	{
		if(!memcmp(pSSIDpassword + offset + idx, "\r\n", 2))
		{
			idx +=2;
			memset(pSSIDpassword + offset + idx, '\0', EEPROM_PAGE_SIZE/2 - offset - idx);
			result = pSSIDpassword;
		}
	}

	return result;
}

uint32_t EEPROM_GetSystemState(void)
{
	uint32_t subResult;

	subResult = EEPROM_ReadData(EEPROM_SYS_STATE_ADR, (uint8_t*)&systemGlobalState, sizeof(sys_state_t));

	subResult = (0 == subResult) ? 0 : (uint32_t)-1;

	return subResult;
}

uint32_t EEPROM_SetSystemState(void)
{
	uint32_t subResult;

	subResult = EEPROM_WriteData(EEPROM_SYS_STATE_ADR, (uint8_t*)&systemGlobalState, sizeof(sys_state_t));

	subResult = (0 == subResult) ? 0 : (uint32_t)-1;

	return subResult;
}

uint32_t ExtractValue(const uint8_t* const pKeyWord,
		const uint8_t* const pBUff,
		const uint32_t buffLng,
		uint32_t* val)
{
    const uint8_t* pBrowse = pBUff;

    uint32_t strLng = 0;
    uint32_t value = 0;
	uint32_t decOrder = 1;
	uint32_t keyWordFound = 0;

    /* Is keyword in pBUff ? */
    for (uint32_t idx = 0; idx < buffLng; idx++)
    {
        if (!memcmp(pBrowse, (char*)pKeyWord, strlen((char*)pKeyWord)))
        {
            keyWordFound = 1;
            break;
        }
        (char*)(pBrowse++);
    }

    //if ((!keyWordFound) || ((pBUff - pBrowse) >= buffLng))
    if (!keyWordFound)
    {
        /* Keyword is either not found or there is no data after keyword*/
        *val = 0;
        return 0;
    }

    /* Move pBrowse on the number */
    pBrowse = (uint8_t*)(pBrowse + strlen((char*)pKeyWord));

    for (uint32_t idx = 0; IS_NUM(pBrowse[idx]); idx++)
    {
        strLng++;
    }

    /* Create highest decimal order of the string */
    for (uint32_t idx = 0; idx < strLng - 1; idx++)
    {
        decOrder = decOrder * 10;
    }

    /* Go from left to right through the string and create the int value */
    for (uint32_t idx = 0; idx < strLng; idx++)
    {
        value += (pBrowse[idx] - '0') * decOrder;

        decOrder = decOrder / 10;
    }

    *val = value;

    return keyWordFound;
}

