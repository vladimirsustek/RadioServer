/*
 * esp8266_port.c
 *
 *  Created on: Jul 19, 2022
 *      Author: 42077
 */

#include "esp8266_port.h"

/* STM32 Hardware dependent UART handle */

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

static uint8_t uartX_rx_buf[ESP_COM_BUFF_LNG] = {0};

/* User Buffer */
uint8_t comUsrBuffer[COM_USR_RX_MESSAGES_MAX][ESP_COM_BUFF_LNG + 1] = {0};
uint32_t comUsrBufferLen[COM_USR_RX_MESSAGES_MAX] = {0};
uint32_t comUserBufferMsgIdx = 0;
uint32_t comUserBufferMsgReadIdx = 0;

/* Internal buffer*/
static uint32_t uartX_rx_read_ptr = 0;

/* Send command timeout */
static uint32_t sendTimeOut = 0;
static uint32_t sendTimeOutStarted = 0;

uint32_t Start_DMA_XUART(void);
uint32_t CheckRX_DMA_XUART(uint32_t timeout);


static uint32_t Do_2Sec_Reset(void)
{
	HAL_GPIO_WritePin(GPIOA, ESP_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(ESP_TIMEOUT_1s);
	HAL_GPIO_WritePin(GPIOA, ESP_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(ESP_TIMEOUT_1s);
	return 0;
}

uint32_t Start_DMA_XUART(void)
{
	uint32_t result;
	result = (uint32_t)HAL_UART_Receive_DMA(&huart3, uartX_rx_buf, ESP_COM_BUFF_LNG);
	return result;
}


/* @brief Check DMA-based uartX_rx_buf and fetch into comUsrBuffer - must be called periodically
 *
 * @detail Function checks whether hardware based uartX_rx_write_ptr (ESP_COM_BUFF_LNG - hdma_usart3_rx.Instance->CNDTR)
 * is equal to the software served uartX_rx_read_ptr. If these values are not equal, data was received and needs processing.
 * Function tries to save processing time and therefore timeout-based processing is introduced. Therefore, when any receive
 * is signaled by inequality of the pointers mentioned upper, the timeOutStarted flag and timeOut is used to wait period
 * defined by TRANSACTION_TIMEOUT. When the timeOut elapses, loop mechanism fetches data into comUsrBuffer from comUsrBuffer.
 * The comUsrBuffer is intended to be multi-line (can store COM_USR_RX_MESSAGES_MAX-times the ESP_COM_BUFF_LNG) and each time
 * the loop processing starts a new comUsrBuffer "line" is added. Also new line may be added if the received data is bigger
 * then ESP_COM_BUFF_LNG. There is no mechanism to prevent overriding previously stored data if comUsrBuffer is full as the
 * comUsrBuffer is intended to be ring-buffer.
 *
 * Function also interacts with logic of the ESP_SendCommand() and therefore if command is send, the sendTimeOutStarted flag
 * is set and sendTimeOut starts. The CheckRX_DMA_XUART may stop the timeout if any data was received (so even unexpected).
 *
 * @return
 * ESP_RX_PENDING - when timeout is ongoing
 * ESP_OK - receive happened
 * ESP_RX_SILENT - no receive or timeout ongoing
 * ESP_NEVER_VALUE - initial value which must not be ever returned
 *
 */
uint32_t CheckRX_DMA_XUART(uint32_t timeout)
{

	uint32_t result = ESP_NEVER_VALUE;
	uint32_t writtenChars = 0;

	static uint32_t timeOut = 0;
	static uint32_t timeOutStarted = 0;

	/* If any bytes were received */
	if (uartX_rx_read_ptr != (ESP_COM_BUFF_LNG - hdma_usart3_rx.Instance->CNDTR))
	{
		/* and was expected response from previous command, cancel timeout timer of the command */
		if(sendTimeOutStarted)
		{
			sendTimeOutStarted = 0;
		}

		/* If timer to receive all bytes was not started yet*/
		if(!timeOutStarted)
		{
			/* start timer to wait until all bytes can arrive*/
			timeOutStarted = 1;
			timeOut = HAL_GetTick();
			result = ESP_RX_PENDING;
		}
		else
		{
			/* If timer was started and timeout elapsed, retrieve bytes */
			if (timeOut + timeout < HAL_GetTick())
			{
				/* cancel timer to receive all bytes */
				timeOutStarted = 0;
				uint32_t uartX_rx_write_ptr = (ESP_COM_BUFF_LNG - hdma_usart3_rx.Instance->CNDTR);
				while (uartX_rx_read_ptr != uartX_rx_write_ptr)
				{
					/* Said my teacher is better ... to really get DMA content actualized .. Don't Know*/
					__ISB();
					/* Write received char into user buffer */
					comUsrBuffer[comUserBufferMsgIdx][writtenChars] = uartX_rx_buf[uartX_rx_read_ptr];
					/* Always push in front a null character (buffer element is UART_BUFFSIZE + 1) for better string handling */
					comUsrBuffer[comUserBufferMsgIdx][writtenChars+1] = '\0';

					uartX_rx_read_ptr = (uartX_rx_read_ptr + (uint32_t)1u) % ESP_COM_BUFF_LNG;

					/* If the end of the string arrived or string is too long for one line, jump on the next line of comUsrBuffer*/
					if (uartX_rx_read_ptr == uartX_rx_write_ptr || (writtenChars >= (ESP_COM_BUFF_LNG-1)))
					{
						/* Non-<CR><LF> message received> .. some exception mechanism */
						comUsrBufferLen[comUserBufferMsgIdx] = writtenChars+1;
						comUserBufferMsgIdx = (comUserBufferMsgIdx + (uint32_t)1u) % COM_USR_RX_MESSAGES_MAX;
						result = ESP_OK;
					}
					writtenChars++;
					uartX_rx_write_ptr = (ESP_COM_BUFF_LNG - hdma_usart3_rx.Instance->CNDTR);

				}
			}
			else
			{
				/*Otherwise say timeout is pending*/
				result = ESP_RX_PENDING;
			}
		}
	}
	else
	{
		/* and was expected response from previous command, cancel timeout timer of the command */
		if(sendTimeOutStarted && sendTimeOut + TX_TIMETOUT < HAL_GetTick())
		{
			sendTimeOutStarted = 0;
			result = ESP_TX_TIMEOUT;
		}
		else
		{
			result = ESP_RX_SILENT;

		}
	}

	while(result == ESP_NEVER_VALUE);

	return result;
}


uint32_t ESP_CheckForKeyWord(char * key, const uint32_t key_lng, uint8_t * buff, const uint32_t buff_lng)
{
	uint32_t result = ESP_RSP_ERR;
	uint8_t* pBuff = buff;

	if(key == NULL || buff == NULL)
	{
		return ESP_HARD_ERR;
	}
	if(key_lng > buff_lng)
	{
		return result;
	}

	for (uint32_t idx = 0; idx < buff_lng - key_lng + 1; idx++)
	{
		if(!memcmp(pBuff, key, key_lng))
		{
			result = ESP_OK;
			break;
		}

		(uint8_t*)pBuff++;

	}
	return result;
}

uint32_t ESP_ComInit(void)
{
	uint32_t result = 0;

	Do_2Sec_Reset();

	result = Start_DMA_XUART();

	return result;
}

uint32_t ESP_SendCommand(const char* const pStrCmd, const uint32_t lng)
{
	uint32_t result = ESP_NEVER_VALUE;

	if(!sendTimeOutStarted)
	{
		result = (uint32_t)HAL_UART_Transmit(&huart3, (uint8_t*)pStrCmd, lng, HAL_MAX_DELAY);
		sendTimeOutStarted = 1;
		sendTimeOut = HAL_GetTick();
		result = ESP_OK;
	}
	else
	{
		result = ESP_COMMAND_BUSY;
	}

	return result;
}

uint32_t ESP_CheckReceived(void)
{
	return ESP_CheckResponse(ESP_DONT_CHECK_RESPONSE_P, ESP_DONT_CHECK_RESPONSE_V, ESP_TIMEOUT_1s);

}

uint32_t ESP_CheckResponse(char* key, const uint32_t key_lng, uint32_t timeout)
{
	uint32_t rxResult;
	uint32_t processingResult;
	uint32_t okAlreadyArrived;
	do
	{
		rxResult = CheckRX_DMA_XUART(timeout);

	} while(rxResult == ESP_RX_PENDING || rxResult == ESP_TX_TIMEOUT || rxResult == ESP_RX_SILENT);

	if (ESP_OK == rxResult)
	{
		while(comUserBufferMsgIdx != comUserBufferMsgReadIdx)
		{
			/* Just stupid print out what's received */
			CDC_Transmit_FS(comUsrBuffer[comUserBufferMsgReadIdx], comUsrBufferLen[comUserBufferMsgReadIdx]);
			CDC_Transmit_FS((uint8_t*)("\r\n"), 2);

			if(key != ESP_DONT_CHECK_RESPONSE_P)
			{
				processingResult = ESP_CheckForKeyWord(key, key_lng, comUsrBuffer[comUserBufferMsgReadIdx], comUsrBufferLen[comUserBufferMsgReadIdx]);
			}
			else
			{
				processingResult = ESP_OK;
			}

			if (processingResult == ESP_OK)
			{
				okAlreadyArrived = 1;
			}

			/* Logic to iterate in the multi-line buffer */
			comUserBufferMsgReadIdx = (comUserBufferMsgReadIdx + (uint32_t)1u) % COM_USR_RX_MESSAGES_MAX;

		}
	}
	else
	{
		/* timeout occured .. */
	}

	/* If multiline were processed and e.g. second last already had OK - still response arrived*/
	if(okAlreadyArrived && processingResult != ESP_OK)
	{
		processingResult = ESP_OK;
	}

	return processingResult;
}

#if NEEDED_RING_BUFFER_OVERFLOW
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
	}

}
#endif

