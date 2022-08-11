/*
 * esp8266_port.c
 *
 *  Created on: Jul 19, 2022
 *      Author: 42077
 */

#include "esp8266_port.h"

/* STM32 Hardware dependent UART handle */

#define COM_USR_RX_MESSAGES_MAX				(uint32_t)(8u)

#define BITS_OF_UART_8N1_TRANSACTION		(uint32_t)(10u)
#define UART_BAUDRATE						(uint32_t)(115200u)
#define INTO_MILISECONDS					(uint32_t)(1000u)
#define TIMEOUT_FOR_LONGEST_TRANSACTION		(uint32_t)((ESP_COM_BUFF_LNG*BITS_OF_UART_8N1_TRANSACTION*INTO_MILISECONDS)/UART_BAUDRATE)


UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;


/* User Buffer */
static uint8_t comUsrBuffer[COM_USR_RX_MESSAGES_MAX][ESP_COM_BUFF_LNG + 1] = {0};
static uint32_t comUserBufferMsgIdx = 0;
static uint32_t comUserBufferMsgReadIdx = 0;

/* Internal buffer*/
static uint32_t uartX_rx_read_ptr = 0;
static uint8_t uartX_rx_buf[ESP_COM_BUFF_LNG] = {0};

/* Send command timeout */
static uint32_t sendTimeOut = 0;
static uint32_t sendTimeOutStarted = 0;

uint32_t ESP_ComInit()
{
	uint32_t result;
	result = (uint32_t)HAL_UART_Receive_DMA(&huart3, uartX_rx_buf, ESP_COM_BUFF_LNG);
	return result;
}

uint32_t refresh_uartX(void)
{

	uint32_t result = ESP_NEVER_TESTING_VALUE;
	char prev_rec_c = '\0';
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
			if (timeOut + TIMEOUT_FOR_LONGEST_TRANSACTION < HAL_GetTick())
			{
				/* cancel timer to receive all bytes */
				timeOutStarted = 0;

				while (uartX_rx_read_ptr != (ESP_COM_BUFF_LNG - hdma_usart3_rx.Instance->CNDTR))
				{
					/* Said my teacher is better ... to really get DMA content actualized .. Don't Know*/
					__ISB();
					/* Write received char into user buffer */
					comUsrBuffer[comUserBufferMsgIdx][writtenChars] = uartX_rx_buf[uartX_rx_read_ptr];
					/* Always push in front a null character (buffer element is UART_BUFFSIZE + 1) for better string handling */
					comUsrBuffer[comUserBufferMsgIdx][writtenChars+1] = '\0';

					uartX_rx_read_ptr = (uartX_rx_read_ptr + (uint32_t)1u) % ESP_COM_BUFF_LNG;

					/* If the end of the string arrived or string is too long for one line, jump on the next line of comUsrBuffer*/
					if((('\r' == prev_rec_c) && ('\n' ==  comUsrBuffer[comUserBufferMsgIdx][writtenChars] )) || (writtenChars >= ESP_COM_BUFF_LNG-1))
					{
						/* With ring wrap around*/
						comUserBufferMsgIdx = (comUserBufferMsgIdx + (uint32_t)1u) % COM_USR_RX_MESSAGES_MAX;
						result = ESP_OK;
					}
					else
					{
						if (uartX_rx_read_ptr == (ESP_COM_BUFF_LNG - hdma_usart3_rx.Instance->CNDTR))
						{
							/* Non-<CR><LF> message received> .. some exception mechanism */
							result = ESP_INCORRECT_CRLF;
						}

					}

					prev_rec_c = comUsrBuffer[comUserBufferMsgIdx][writtenChars];

					writtenChars++;

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
		result = ESP_RX_SILENT;
	}


	while(result == ESP_NEVER_TESTING_VALUE);

	return result;
}

int processUserBuffer()
{
	while(comUserBufferMsgIdx != comUserBufferMsgReadIdx)
	{
		printf((char*)comUsrBuffer[comUserBufferMsgReadIdx]);
		comUserBufferMsgReadIdx = (comUserBufferMsgReadIdx + (uint32_t)1u) % COM_USR_RX_MESSAGES_MAX;
	}
	return 0;
}
uint32_t ESP_SendCommand(uint8_t* pStrCmd, uint32_t lng)
{
	uint32_t result = ESP_NEVER_TESTING_VALUE;

	if(!sendTimeOutStarted)
	{
		result = (uint32_t)HAL_UART_Transmit(&huart3, pStrCmd, lng, HAL_MAX_DELAY);
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

uint32_t ESP_CheckReceived()
{
	uint32_t result = refresh_uartX();
	processUserBuffer();
	return 0;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
		printf("UART RX %ld bytes overflow\r\n", ESP_COM_BUFF_LNG);
	}

}

