/*
 * esp8266_http_server.c
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */


#include "esp8266_http_server.h"

#define STATIC_IP_AND_NEW_WIFI 0

static char httpReqBuff[MAX_HTTP_REQ_SIZE + 1] = {0};

uint32_t ESP_HTTPinit (void)
{

	uint32_t result[10] = {0};

	result[0] = ESP_ComInit();

	result[1] = LEDC_SetNewStandingText("ESPI");

    ESP_SendCommand(atCmd_RST, strlen(atCmd_RST));
    result[2] = ESP_CheckResponse((char*)atRsp_ready, strlen(atRsp_ready), ESP_TIMEOUT_2s);

	/********* AT **********/
    ESP_SendCommand(atCmd, strlen(atCmd));
    result[3] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);


	/********* AT+CWMODE=1 **********/
    ESP_SendCommand(atCmd_CWMODE, strlen(atCmd_CWMODE));
    result[4] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

#if STATIC_IP_AND_NEW_WIFI
	/* Set Static IP Address */
	/********* AT+CWSTAIP=IPADDRESS **********/
    ESP_SendCommand(atCmd_CWSTAIP, strlen(atCmd_CIPSTA));
    result[5] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    /* Connect to WIFI */
	/********* AT+CWJAP="SSID","PASSWD" **********/
    ESP_SendCommand(atCmd_CWJAP, strlen(atCmd_CWJAP));
    result[6] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), 2*ESP_TIMEOUT_2s);
#endif
	/********* AT+CIPMUX **********/
    ESP_SendCommand(atCmd_CIPMUX, strlen(atCmd_CIPMUX));
    result[7] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	/********* AT+CIPSERVER **********/
    ESP_SendCommand(atCmd_CIPSERVER, strlen(atCmd_CIPSERVER));
    result[8] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    result[9] = LEDC_StopStandingText();

    for (uint32_t idx = 0; idx < 8; idx++)
    {
    	if (result[idx] != 0)
    		return ESP_HARD_ERR;
    }
    return ESP_OK;
}


uint32_t ESP_SendHTTP (char *str, uint32_t Link_ID)
{
	uint32_t len = strlen (str);
	char data[80];

	sprintf (data, "%s%lu,%lu\r\n", atCmd_CIPSEND, Link_ID, len);

    ESP_SendCommand(data, strlen(data));
    ESP_CheckResponse(">", strlen(">"), ESP_TIMEOUT_300ms);

    ESP_SendCommand(str, strlen(str));
    ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	sprintf (data, "%s%lu\r\n",atCmd_CIPCLOSE, Link_ID);

    ESP_SendCommand(data, strlen(data));
    ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    return 1;
}

uint32_t ESP_DetectedHTTP(char * key, char * buff, uint32_t buff_lng, char **ppRetStr, uint32_t *retVal)
{
	uint32_t result = ESP_RSP_ERR;
	uint32_t httReqLng = 0;
	char* pBeginHTTP = NULL;

	UNUSED(key);

	pBeginHTTP = IsESP_httpRequest(buff, buff_lng, &httReqLng);

	if(pBeginHTTP != NULL && httReqLng < MAX_HTTP_REQ_SIZE)
	{
		memcpy(httpReqBuff, pBeginHTTP, httReqLng);
		httpReqBuff[httReqLng] = '\0';
		*retVal = httReqLng;
		*ppRetStr = httpReqBuff;
		result = ESP_OK;
	}
	else
	{
		*retVal = 0;
		*ppRetStr = NULL;
	}

	return result;
}

uint32_t ESP_CheckReceiveHTTP(char **ppHTTPreq, uint32_t *pHTTPreqLng)
{
	const uint32_t timeOut = 150u;
	const uint32_t timeOutFlag = 0u;

	uint32_t result = ESP_RSP_ERR, HTTPreqLng = 0;

	result = ESP_CheckRX(timeOut, timeOutFlag, ESP_DetectedHTTP, NULL, ppHTTPreq, &HTTPreqLng);

	*pHTTPreqLng = HTTPreqLng;

	return result;
}

char* ESP_ProcessHTTP(char *pHTTPReq, uint32_t hhhtReqLng)
{
	uint32_t linkNum = 0, value = 0;
	char *pBegin = NULL;
	char auxStr[32];

	if (ESP_ExtractValue("+IPD,", pHTTPReq, hhhtReqLng, &linkNum))
	{
		uint32_t DummyLng = 0;

		if((pBegin = ESP_ExtractString("LEDON", pHTTPReq, hhhtReqLng, &DummyLng)) != NULL)
		{
			BLUEPILL_LED(1);
		}
		if((pBegin = ESP_ExtractString("LEDOFF", pHTTPReq, hhhtReqLng, &DummyLng)) != NULL)
		{
			BLUEPILL_LED(0);
		}
		if(ESP_ExtractValue("volm=", pHTTPReq, hhhtReqLng, &value) && ESP_ExtractString("ST_VOLM", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mSetVolm((uint8_t)value);
			sprintf(auxStr, "RDA VOLUME %02ld", value);
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		if(ESP_ExtractValue("freq=", pHTTPReq, hhhtReqLng, &value) && ESP_ExtractString("ST_FREQ", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mSetFreq((uint16_t)value);
			sprintf(auxStr, "RDA FREQUENCY %05ld", value);
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		if(ESP_ExtractString("DO_INIT", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mInit();
			sprintf(auxStr, "RDA INIT");
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		if(ESP_ExtractString("DO_RSET", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mReset();
			sprintf(auxStr, "RDA RESET");
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		ESP_SendHTTP((char*)pageIndex, linkNum);
	}

	return pBegin;
}
