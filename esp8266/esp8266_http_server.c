/*
 * esp8266_http_server.c
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */

#include "esp8266_port.h"
#include "esp8266_utils.h"
#include "esp8266_http_server.h"

#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOC

#include "stm32f1xx_hal.h"
const char *page1 = "<!DOCTYPE html>\n\
		<html>\n\
		<body>\n\
		<h1>This is page 1</h1>\n\
		<form action=\"page2\">\n\
		<input type=\"submit\" value=\"Page2\">\n\
		</form><br><br>\n\
		</body></html>";

const char *page2 = "<!DOCTYPE html>\n\
		<html>\n\
		<body>\n\
		<h1>This is page 2</h1>\n\
		<form action=\"page1\">\n\
		<input type=\"submit\" value=\"Page1\">\n\
		</form><br><br>\n\
		</body></html>";

const char *page3 = "<!DOCTYPE html>\n\
		<html>\n\
		<body>\n\
		<h1>Here you can control green BluePill's LED </h1>\n\
		<form action=\"page1 LEDON\">\n\
		<input type=\"submit\" value=\"LED ON\">\n\
		</form><br><br>\n\\
		<form action=\"page1 LEDOFF\">\n\
		<input type=\"submit\" value=\"LED OFF\">\n\
		</form><br><br>\n\
		</body></html>";

const char * atCmd = "AT\r\n";
const char * atCmd_RST = "AT+RST\r\n";
const char * atCmd_CWMODE = "AT+CWMODE=1\r\n";
const char * atCmd_CIPMUX = "AT+CIPMUX=1\r\n";
const char * atCmd_CIPSERVER = "AT+CIPSERVER=1,80\r\n";

const char * atRsp_OK = "AT+OK";
const char * atRsp_ready = "ready";

extern uint8_t comUsrBuffer[COM_USR_RX_MESSAGES_MAX][ESP_COM_BUFF_LNG + 1];
extern uint32_t comUsrBufferLen[COM_USR_RX_MESSAGES_MAX];
extern uint32_t comUserBufferMsgIdx;
extern uint32_t comUserBufferMsgReadIdx;


uint32_t ESP_httpInit (void)
{
	uint32_t result[6] = {0};

    ESP_SendCommand(atCmd_RST, strlen(atCmd_RST));
    result[0] = ESP_CheckResponse(atRsp_ready, strlen(atRsp_ready), ESP_TIMEOUT_2s);

	/********* AT **********/
    ESP_SendCommand(atCmd, strlen(atCmd));
    result[1] = ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);


	/********* AT+CWMODE=1 **********/
    ESP_SendCommand(atCmd_CWMODE, strlen(atCmd_CWMODE));
    result[2] = ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	/* Set Static IP Address */
	/********* AT+CWSTAIP=IPADDRESS **********/
    //ESP_SendCommand(atCmd_CIPSTA, strlen(atCmd_CIPSTA));
    //if(ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK))) while(1);

	/********* AT+CWJAP="SSID","PASSWD" **********/
    //ESP_SendCommand(atCmd_CWJAP, strlen(atCmd_CWJAP));
    //result[3] = ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK));

	/********* AT+CIPMUX **********/
    ESP_SendCommand(atCmd_CIPMUX, strlen(atCmd_CIPMUX));
    result[4] = ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	/********* AT+CIPSERVER **********/
    ESP_SendCommand(atCmd_CIPSERVER, strlen(atCmd_CIPSERVER));
    result[5] = ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    for (uint32_t idx = 0; idx < 6; idx++)
    {
    	if (result[idx] != 0)
    		return ESP_HARD_ERR;
    }
    return ESP_OK;
}


uint32_t Server_Send (char *str, int Link_ID)
{
	uint32_t len = strlen (str);
	char data[80];

	sprintf (data, "AT+CIPSEND=%d,%d\r\n", Link_ID, len);

    ESP_SendCommand(data, strlen(data));
    ESP_CheckResponse(">", strlen(">"), ESP_TIMEOUT_300ms);

    ESP_SendCommand(str, strlen(str));
    ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	sprintf (data, "AT+CIPCLOSE=%d\r\n",Link_ID);

    ESP_SendCommand(data, strlen(data));
    ESP_CheckResponse(atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    return 1;
}

uint32_t CheckRX_DMA_XUART(uint32_t timeout);

uint32_t ESP_ServerProcess(uint32_t timeout)
{
	uint32_t rxResult;
	uint32_t processingResult;
	uint32_t okAlreadyArrived;
	uint32_t httpReqLng;

	char *pBegin = NULL;
	uint32_t pageNum, linkNum;

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

			if(NULL != (pBegin = IsESP_httpRequest((char*)comUsrBuffer[comUserBufferMsgReadIdx],comUsrBufferLen[comUserBufferMsgReadIdx], &httpReqLng)))
			{
		        if (ESP_ExtractValue("+IPD,", pBegin, httpReqLng, &linkNum))
		        {
		        	uint32_t lng;

		        	if(ESP_ExtractString("LEDON", pBegin, httpReqLng, &lng))
		        	{
		        		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
		        	}
		        	if(ESP_ExtractString("LEDOFF", pBegin, httpReqLng, &lng))
		        	{
		        		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
		        	}
		        	Server_Send(page3, linkNum);
#if 0
		        	if(ESP_ExtractValue("page", pBegin, httpReqLng, &pageNum))
		        	{
						switch(pageNum)
						{
						case 1:
						{
							Server_Send(page1, linkNum);
						}
						break;
						case 2:
						{
							Server_Send(page2, linkNum);
						}
						break;
						}
		        	}
				    else
				    {
						Server_Send(page1, linkNum);
				    }
#endif
		        }

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

