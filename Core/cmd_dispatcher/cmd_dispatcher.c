#include "cmd_commands.h"
#include "cmd_defs.h"
#include "cmd_rda5807m.h"
#include "ledc_if.h"
#include "../esp8266/esp8266_functions.h"
#include "eeprom_25aa1024.h"

#define CMD_TABLE_SIZE  (uint8_t)(16)

static const CmdDisp_t cmdTable[CMD_TABLE_SIZE] = {

/*01*/    {CMD_METHOD_DO,  CMD_RDA5807M_INIT, CmdRDA5807mDoInit},
/*02*/    {CMD_METHOD_DO,  CMD_RDA5807M_RSET, CmdRDA5807mDoReset},

/*03*/    {CMD_METHOD_SET, CMD_RDA5807M_VOLM, CmdRDA5807mSetVolm},
/*04*/    {CMD_METHOD_SET, CMD_RDA5807M_FREQ, CmdRDA5807mSetFreq},
/*05*/    {CMD_METHOD_SET, CMD_RDA5807M_MUTE, CmdRDA5807mSetMute},

/*06*/    {CMD_METHOD_GET, CMD_RDA5807M_RSSI, CmdRDA5807mGetRSSI},
/*07*/    {CMD_METHOD_GET, CMD_RDA5807M_RDSR, CmdRDA5807mGetRDSR},
/*08*/    {CMD_METHOD_GET, CMD_RDA5807M_RDSS, CmdRDA5807mGetRDSS},
/*09*/    {CMD_METHOD_GET, CMD_RDA5807M_STRO, CmdRDA5807mGetSTRO},
/*10*/    {CMD_METHOD_GET, CMD_RDA5807M_CHST, CmdRDA5807mGetCHST},

/*11*/    {CMD_METHOD_GET, CMD_RDA5807M_BLKA, CmdRDA5807mGetBLKA},
/*12*/    {CMD_METHOD_GET, CMD_RDA5807M_BLKB, CmdRDA5807mGetBLKB},
/*13*/    {CMD_METHOD_GET, CMD_RDA5807M_BLKC, CmdRDA5807mGetBLKC},
/*14*/    {CMD_METHOD_GET, CMD_RDA5807M_BLKD, CmdRDA5807mGetBLKD},

/*15*/    {CMD_METHOD_GET, CMD_RDA5807M_ERRA, CmdRDA5807mGetERRA},
/*16*/    {CMD_METHOD_GET, CMD_RDA5807M_ERRB, CmdRDA5807mGetERRB}

};

uint16_t CmdDispatch(const uint8_t* const pStrCmd, const uint8_t lng) {

    uint16_t result = CMD_RET_UKN;
    char strBuff[32];


    for(uint8_t idx = 0; idx < CMD_TABLE_SIZE; idx++) {

        if (!memcmp(pStrCmd, cmdTable[idx].cmdMethod, CMD_METHOD_LNG) &&
        !memcmp(pStrCmd + CMD_METHOD_LNG + CMD_DELIMITER_LNG, cmdTable[idx].cmdName, CMD_NAME_LNG)) {

            result = cmdTable[idx].cmdFunc(pStrCmd, lng);
            break;
        }
    }

    /*Testing interface for ESP8266*/
    if('A' == pStrCmd[0] && 'T' ==  pStrCmd[1])
    {
    	char buff[64] = {0};
    	char buff2[64] = {0};
    	memcpy(buff, pStrCmd, lng);
    	buff[lng - 1] = '\0';
    	sprintf(buff2, "%s", pStrCmd);
    	ESP_SendCommand((char*)pStrCmd, lng);
    	return 0;
    }
    if(!memcmp(pStrCmd, "STR_", strlen("STR_")))
    {
    	ESP_SendCommand((char*)(pStrCmd+4), lng-4);
    	return 0;
    }

    if(!memcmp(pStrCmd, "EEPROM_W_", strlen("EEPROM_W_")) && lng > strlen("EEPROM_W_ADR_000000_"))
    {
    	// EEPROM_W_ADR_000000_
    	uint32_t addr = 0;
    	ESP_ExtractValue("ADR_", (const uint8_t*)pStrCmd, lng, &addr);
    	EEPROM_WriteData(
    			addr,
				(uint8_t*)pStrCmd + strlen("EEPROM_W_ADR_000000_"),
				lng - strlen("EEPROM_W_ADR_000000_")
				);
    }

    if(!memcmp(pStrCmd, "EEPROM_R", strlen("EEPROM_R")) && lng > strlen("EEPROM_R_ADR_000000_LNG_000000"))
    {
    	// EEPROM_R_ADR_000000_LNG_000000
    	uint32_t addr = 0, readLng = 0;
    	uint8_t auxBuff[EEPROM_PAGE_SIZE] = {0};
    	ESP_ExtractValue("ADR_", (const uint8_t*)pStrCmd, lng, &addr);
    	ESP_ExtractValue("LNG_", (const uint8_t*)pStrCmd, lng, &readLng);

    	uint16_t subLng = (readLng > EEPROM_PAGE_SIZE) ? EEPROM_PAGE_SIZE : readLng;

    	while(readLng)
    	{
        	EEPROM_ReadData(addr, auxBuff, subLng);
        	readLng -= subLng;
        	printf("%s", (char*)auxBuff);
        	HAL_Delay(100);
    	}
    }

    /* printf redirected to UART in uart_interface.c*/
#if OLD_LONG_RESPONSE
    sprintf(strBuff, "<< %s  >> RET = 0x%04x\n", pStrCmd, result);

#else
    switch(result)
    {
    case CMD_RET_UKN: { sprintf(strBuff, "CMD_RET_UKN"); } break;
    case CMD_RET_ERR: { sprintf(strBuff, "CMD_RET_ERR"); } break;
    case CMD_RET_OK: { sprintf(strBuff, "%s", pStrCmd); } break;
    default : {  sprintf(strBuff, "TBD"); }
    }
#endif
    LEDC_SetNewRollingString(strBuff, strlen(strBuff));

    return result;
}
