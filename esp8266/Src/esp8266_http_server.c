/*
 * esp8266_http_server.c
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */


#include "esp8266_http_server.h"

#define STATIC_IP_AND_NEW_WIFI 0

#define atCmd_CWJAP_LNG		(uint32_t)(strlen("AT+CWJAP="))

static char httpReqBuff[MAX_HTTP_REQ_SIZE + 1] = {0};

uint32_t ESP_HTTPinit (void)
{

	uint32_t result = 0;
	uint8_t* subResult = 0;
	uint8_t pSSIDpassword[EEPROM_PAGE_SIZE/2];

	// UART init and activate-deactivate RST pin of ESP8266
	ESP_ComInit();

	// Software reset
	for(uint8_t resetAttempts = 0; resetAttempts < 3; resetAttempts++)
	{
	    ESP_SendCommand(atCmd_RST, strlen(atCmd_RST));
	    if(NULL == ESP_CheckResponse((char*)atRsp_ready, strlen(atRsp_ready), ESP_TIMEOUT_2s))
	    {
	    	result++;
	    }
	    else
	    {
	    	result = 0;
	    	break;
	    }
	}

	// If reset 3x did not work well, no reason to continue
	if(0 != result)
	{
    	LEDC_StopStandingText();
		return ESP_HARD_ERR;

	}

    // Disconnecting from current WIFI
    ESP_SendCommand(atCmd_CWQAP, strlen(atCmd_CWQAP));
    if(NULL == ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_2s)) result++;

    // Iterative attempt to connect with predefined EEPROM stored WIFIs
    for (uint32_t adr = EEPROM_WIFI_ADR_0;
    		adr <= EEPROM_WIFI_ADR_3;
    		adr += EEPROM_PAGE_SIZE/2)
    {
    	uint8_t* wifi = EEPROM_GetWIfi(adr, atCmd_CWJAP_LNG, pSSIDpassword);

    	// If no WIFI fetched
    	if (NULL == wifi)
    	{
    		// Attempt set unsuccessful and force a next read
    		subResult = NULL;
    		continue;
    	}
		memcpy(wifi, atCmd_CWJAP, atCmd_CWJAP_LNG);

		ESP_SendCommand((char*)wifi, strlen((char*)wifi));
		subResult = ESP_CheckResponse((char*)atRsp_WifiGotIp,
				strlen(atRsp_WifiGotIp),
				ESP_TIMEOUT_15s);
		if (NULL != subResult)
		{
			EEPROM_GetSystemState();
			systemGlobalState.states.espConnected = 1;
			EEPROM_SetSystemState();

			break;
		}
    }

    if (NULL == subResult)
    {
    	return ESP_WIFI_FAILED;
    }

	/********* AT (just testing no-set/no-get command)**********/
    ESP_SendCommand(atCmd, strlen(atCmd));
    if(NULL == ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms)) result++;

	/********* AT+CWMODE=1 **********/
    ESP_SendCommand(atCmd_CWMODE, strlen(atCmd_CWMODE));
    if(NULL == ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms)) result++;

	/********* AT+CIPMUX **********/
    ESP_SendCommand(atCmd_CIPMUX, strlen(atCmd_CIPMUX));
    if(NULL == ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms)) result++;

	/********* AT+CIPSERVER **********/
    ESP_SendCommand(atCmd_CIPSERVER, strlen(atCmd_CIPSERVER));
    if(NULL == ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms)) result++;

    result = (result) ? ESP_HARD_ERR : ESP_OK;

    return result;
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
	char auxStr[32];
	const uint8_t* pBuff = (uint8_t*)auxStr;
	char *pBegin = NULL;

	uint32_t linkNum = 0, value = 0;

	if (ESP_ExtractValue("+IPD,", pHTTPReq, hhhtReqLng, &linkNum))
	{
		uint32_t DummyLng = 0;

		if(ESP_ExtractString("DO_INIT", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			sprintf(auxStr, "DO_INIT\r\n");
			CmdRDA5807mDoInit(pBuff, strlen(auxStr));
		}
		if(ESP_ExtractValue("time=", pHTTPReq, hhhtReqLng, &value) && ESP_ExtractString("ST_TIME", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RTC_TimeTypeDef rtc;

			rtc.Hours = value / 100;
			rtc.Minutes = value % 100;
			rtc.Seconds = 0;

			HAL_RTC_SetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
		}
		if(ESP_ExtractString("DO_RSET", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			sprintf(auxStr, "DO_RSET\r\n");
			CmdRDA5807mDoReset(pBuff, strlen(auxStr));
		}
		if(ESP_ExtractString("ST_MUTE", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			sprintf(auxStr, "ST_MUTE_%c\r\n", '?');
			CmdRDA5807mSetMute(pBuff, strlen(auxStr));
		}
		if(ESP_ExtractValue("volm=", pHTTPReq, hhhtReqLng, &value) && ESP_ExtractString("ST_VOLM", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			sprintf(auxStr, "ST_VOLM_%02ld\r\n", value);
			CmdRDA5807mSetVolm(pBuff, strlen(auxStr));
		}
		if(ESP_ExtractValue("freq=", pHTTPReq, hhhtReqLng, &value) && ESP_ExtractString("ST_FREQ", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			sprintf(auxStr, "ST_FREQ_%05ld\r\n", value);
			CmdRDA5807mSetFreq(pBuff, strlen(auxStr));
		}

		ESP_SendHTTP((char*)pageIndex, linkNum);
	}

	return pBegin;
}
