/*
 * esp8266_http_server.c
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */

#include "esp8266_port.h"
#include "esp8266_utils.h"
#include "esp8266_http_server.h"

#include "main.h"

#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOC

/*
const char * pageIndex = \
		"<!DOCTYPE html>\n\
		<html>\n\
		<body>\n\
		<h1>RDA5807 control page</h1>\n\
		<form action=\"index LEDON\">\n\
		<input type=\"submit\" value=\"BluePill LED ON\">\n\
		</form><br><br>\n\\
		<form action=\"index LEDOFF\">\n\
		<input type=\"submit\" value=\"BluePill LED OFF\">\n\
		</form><br><br>\n\
		</body></html>";
*/
char *pageIndex ="<!DOCTYPE html>\n<html>\n\
				<body>\n\
				<h1>RDA5807 control page</h1>\n\
				<form action=\"/pageIndex LEDON\">\n\
				<input type=\"submit\" value=\"BluePill LED ON  \">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex LEDOFF\">\n\
				<input type=\"submit\" value=\"BluePill LED OFF\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex DO_INITrn\">\n\
				<input type=\"submit\" value=\"Initialize\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex DO_RSETrn\">\n\
				<input type=\"submit\" value=\"Reset\">\n\
				</form>\n\
				<br>\n\
				<br>\n\
				<form action=\"/pageIndex ST_VOLM\">\n\
				<input type=\"text\" id=\"volm\" name=\"volm\" value=\"\"><br><br>\n\
				<input type=\"submit\" value=\"SendVolume\">\n\
				</form>\n\
				<br>\n\
				<form action=\"/pageIndex\ ST_FREQ\">\n\
				<input type=\"text\" id=\"freq\" name=\"freq\" value=\"08920\"><br><br>\n\
				<input type=\"submit\" value=\"SendFrequency\">\n\
				</form>\n\
				</body></html>";

const char * atCmd = "AT\r\n";
const char * atCmd_RST = "AT+RST\r\n";
const char * atCmd_CWMODE = "AT+CWMODE=1\r\n";
const char * atCmd_CIPMUX = "AT+CIPMUX=1\r\n";
#if STATIC_IP_AND_NEW_WIFI
const char * atCmd_CWSTAIP = "AT+CWSTAIP=0.0.0.0";
const char * atCmd_CWJAP = "AT+CWJAP=\"WIFI?\",\"Password\"";
#endif
const char * atCmd_CIPSERVER = "AT+CIPSERVER=1,80\r\n";
const char * atRsp_OK = "AT+OK";
const char * atRsp_ready = "ready";

extern void RDA5807mInit(void);
extern uint16_t RDA5807mSetFreq(uint16_t freq);
extern uint16_t RDA5807mSetVolm(uint8_t volume);
extern void RDA5807mReset(void);

char httpReqBuff[MAX_HTTP_REQ_SIZE + 1] = {0};

uint32_t ESP_httpInit (void)
{
	uint32_t result[7] = {0};

    ESP_SendCommand(atCmd_RST, strlen(atCmd_RST));
    result[0] = ESP_CheckResponse((char*)atRsp_ready, strlen(atRsp_ready), ESP_TIMEOUT_2s);

	/********* AT **********/
    ESP_SendCommand(atCmd, strlen(atCmd));
    result[1] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);


	/********* AT+CWMODE=1 **********/
    ESP_SendCommand(atCmd_CWMODE, strlen(atCmd_CWMODE));
    result[2] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

#if STATIC_IP_AND_NEW_WIFI
	/* Set Static IP Address */
	/********* AT+CWSTAIP=IPADDRESS **********/
    ESP_SendCommand(atCmd_CWSTAIP, strlen(atCmd_CIPSTA));
    result[3] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    /* Connect to WIFI */
	/********* AT+CWJAP="SSID","PASSWD" **********/
    ESP_SendCommand(atCmd_CWJAP, strlen(atCmd_CWJAP));
    result[4] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), 2*ESP_TIMEOUT_2s);
#endif
	/********* AT+CIPMUX **********/
    ESP_SendCommand(atCmd_CIPMUX, strlen(atCmd_CIPMUX));
    result[5] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	/********* AT+CIPSERVER **********/
    ESP_SendCommand(atCmd_CIPSERVER, strlen(atCmd_CIPSERVER));
    result[6] = ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    for (uint32_t idx = 0; idx < 6; idx++)
    {
    	if (result[idx] != 0)
    		return ESP_HARD_ERR;
    }
    return ESP_OK;
}


uint32_t Server_Send (char *str, uint32_t Link_ID)
{
	uint32_t len = strlen (str);
	char data[80];

	sprintf (data, "AT+CIPSEND=%lu,%lu\r\n", Link_ID, len);

    ESP_SendCommand(data, strlen(data));
    ESP_CheckResponse(">", strlen(">"), ESP_TIMEOUT_300ms);

    ESP_SendCommand(str, strlen(str));
    ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

	sprintf (data, "AT+CIPCLOSE=%lu\r\n",Link_ID);

    ESP_SendCommand(data, strlen(data));
    ESP_CheckResponse((char*)atRsp_OK, strlen(atRsp_OK), ESP_TIMEOUT_300ms);

    return 1;
}

/* This function needs no timemout ...
 * Function may just provide string of the request if IPD and HTTP is there
 * and can be placed anywhere else lower ... (not in app server module)
 * so maybe some universal function with selectebale (mutable) timeout
 * which checks for keyword and (optionally) may copy out searched string
 * */

extern uint32_t ESP_CheckRX(uint32_t timeOut,
				     uint32_t blockingTimeOut,
					 U32_pFn_pC_pC_U32_pC_pU32 processFn,
					 char * keyWord,
					 char **retStr,
					 uint32_t * retU32);

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
	const uint32_t timeOut = 300u;
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
	char auxStr[16];

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
			sprintf(auxStr, "VOLUME %02ld", value);
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		if(ESP_ExtractValue("freq=", pHTTPReq, hhhtReqLng, &value) && ESP_ExtractString("ST_FREQ", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mSetFreq((uint16_t)value);
			sprintf(auxStr, "FREQUENCY %05ld", value);
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		if(ESP_ExtractString("DO_INIT", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mInit();
			sprintf(auxStr, "INIT");
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		if(ESP_ExtractString("DO_RSET", pHTTPReq, hhhtReqLng, &DummyLng))
		{
			RDA5807mReset();
			sprintf(auxStr, "RESET");
			LEDC_SetNewRollingString(auxStr, strlen(auxStr));
		}
		Server_Send((char*)pageIndex, linkNum);
	}

	return pBegin;
}
