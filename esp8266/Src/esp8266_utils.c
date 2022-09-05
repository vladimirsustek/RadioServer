/*
 * esp8266_utils.c
 *
 *  Created on: Aug 14, 2022
 *      Author: 42077
 */

#include "esp8266_utils.h"

uint32_t ESP_CheckForKeyWord(char * key, char * buff, uint32_t buff_lng, char **ppRetStr, uint32_t *retVal)
{
	uint32_t result = ESP_RSP_ERR, key_lng = strlen(key);
	char* pBuff = buff;

	UNUSED(ppRetStr);

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
			*ppRetStr = pBuff;
			*retVal = key_lng;
			break;
		}

		(uint8_t*)pBuff++;

	}
	if(result!= ESP_OK)
	{
		*retVal = 0;
		*ppRetStr = NULL;
	}

	return result;
}

char* IsESP_httpRequest(char* pStr, uint32_t strLng, uint32_t *pReqLng)
{
    uint32_t flag = 0;
    char* pReqBegin = NULL;
    char* pBrowse = pStr;

    const char* pHeader = "+IPD,1,397:GET HTTP/1.1";

    const char* pIPD = "+IPD";
    const char* HTTP11 = "HTTP/1.1";

    if (strLng < strlen(pHeader) || pStr == NULL)
        return 0;

    for (uint32_t idx = 0; idx < strLng; idx++)
    {
        if (!memcmp(pBrowse, pIPD, strlen(pIPD)))
        {
            pReqBegin = pBrowse;
            flag++;
            break;
        }

        (char*)(pBrowse++);
    }

    pBrowse = pStr;

    for (uint32_t idx = 0; idx < strLng; idx++)
    {
        if (!memcmp(pBrowse, HTTP11, strlen(HTTP11)))
        {
            flag++;
            break;
        }
        (char*)(pBrowse++);
        *pReqLng = idx;
    }

    if (flag != 2)
    {
        *pReqLng = 0;
        pReqBegin = NULL;
    }
    return pReqBegin;
}


char* ESP_ExtractString(char* pKeyWord, char* pBUff, uint32_t buffLng, uint32_t *strLng)
{
    char* pBrowse = pBUff;

    uint32_t keyWordFound = 0, autxStrLng = 0;

    /* Is keyword in pBUff ? */
    for (uint32_t idx = 0; idx < buffLng; idx++)
    {
        if (!memcmp(pBrowse, pKeyWord, strlen(pKeyWord)))
        {
            keyWordFound = 1;
            break;
        }
        (char*)(pBrowse++);
    }

    //if ((!keyWordFound) || ((pBUff - pBrowse) >= buffLng))
    if ((!keyWordFound))
    {
        /* Keyword is either not found or there is no data after keyword*/
        *strLng = 0;
        return NULL;
    }

    /* Move pBrowse on the number */
    pBrowse = (char*)(pBrowse + strlen(pKeyWord));

    for (uint32_t idx = 0; (pBrowse[idx] != ' ') && (pBrowse[idx] != '&'); idx++)
    {
        autxStrLng++;
    }

    *strLng = autxStrLng;

    return pBrowse;
}

uint32_t ESP_ExtractValue(char* pKeyWord, char* pBUff, uint32_t buffLng, uint32_t* val)
{
    char* pBrowse = pBUff;

    uint32_t strLng = 0, value = 0, decOrder = 1, keyWordFound = 0;

    /* Is keyword in pBUff ? */
    for (uint32_t idx = 0; idx < buffLng; idx++)
    {
        if (!memcmp(pBrowse, pKeyWord, strlen(pKeyWord)))
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
    pBrowse = (char*)(pBrowse + strlen(pKeyWord));

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
