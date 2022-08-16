/*
 * esp8266_functions.c
 *
 *  Created on: Aug 16, 2022
 *      Author: 42077
 */

#include "esp8266_functions.h"

#define PRINT_EACH_RECEIVE 0

extern char comUsrBuffer[COM_USR_RX_MESSAGES_MAX][ESP_COM_BUFF_LNG + 1];
extern uint32_t comUsrBufferLen[COM_USR_RX_MESSAGES_MAX];
extern uint32_t comUserBufferMsgIdx;
extern uint32_t comUserBufferMsgReadIdx;

uint32_t ESP_CheckRX(uint32_t timeOut,
				     uint32_t blockingTimeOut,
					 U32_pFn_pC_pC_U32_pC_pU32 processFn,
					 char * keyWord,
					 char **retStr,
					 uint32_t * retU32)
{
	uint32_t rxResult = ESP_HARD_ERR;
	uint32_t processingResult = ESP_HARD_ERR;
	uint32_t okAlreadyArrived = 0;

	if(blockingTimeOut)
	{
		do
		{
			rxResult = ESP_CheckRX_DMA_XUART(timeOut);
		}
		while(rxResult == ESP_RX_PENDING || rxResult == ESP_TX_TIMEOUT || rxResult == ESP_RX_SILENT);

		if(rxResult == ESP_OK)
		{
			while(comUserBufferMsgIdx != comUserBufferMsgReadIdx)
			{
#if PRINT_EACH_RECEIVE
				CDC_Transmit_FS((uint8_t*)comUsrBuffer[comUserBufferMsgReadIdx], (uint16_t)comUsrBufferLen[comUserBufferMsgReadIdx]);
				CDC_Transmit_FS((uint8_t*)("\r\n"), 2);
#endif
				processingResult = processFn(keyWord,
											comUsrBuffer[comUserBufferMsgReadIdx],
											comUsrBufferLen[comUserBufferMsgReadIdx],
											retStr,
											retU32);
				comUserBufferMsgReadIdx = (comUserBufferMsgReadIdx + (uint32_t)1u) % COM_USR_RX_MESSAGES_MAX;
			}
		}
		else
		{
			processingResult = rxResult;
		}
	}
	else
	{
		if ((rxResult = ESP_CheckRX_DMA_XUART(timeOut)) == ESP_OK)
		{
			while(comUserBufferMsgIdx != comUserBufferMsgReadIdx)
			{
#if PRINT_EACH_RECEIVE
				CDC_Transmit_FS((uint8_t*)comUsrBuffer[comUserBufferMsgReadIdx], (uint16_t)comUsrBufferLen[comUserBufferMsgReadIdx]);
				CDC_Transmit_FS((uint8_t*)("\r\n"), 2);
#endif
				processingResult = processFn(keyWord,
											comUsrBuffer[comUserBufferMsgReadIdx],
											comUsrBufferLen[comUserBufferMsgReadIdx],
											retStr,
											retU32);
				comUserBufferMsgReadIdx = (comUserBufferMsgReadIdx + (uint32_t)1u) % COM_USR_RX_MESSAGES_MAX;
			}
		}
		else
		{
			processingResult = rxResult;
		}
	}


	if(okAlreadyArrived && processingResult != ESP_OK)
	{
		processingResult = ESP_OK;
	}

	return processingResult;
}

uint32_t ESP_CheckResponse(char *pCmd, uint32_t cmdLng, uint32_t timeOut)
{
	const uint32_t blockingTimeoutYes = 1u;

	uint32_t result = ESP_RSP_ERR;
	uint32_t pDummyU32;

	char *pDummyC = NULL;

	result = ESP_CheckRX(timeOut, blockingTimeoutYes, ESP_CheckForKeyWord, pCmd, &pDummyC, &pDummyU32);

	return result;
}
