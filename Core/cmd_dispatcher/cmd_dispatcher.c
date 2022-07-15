#include "cmd_commands.h"
#include "cmd_defs.h"
#include "cmd_rda5807m.h"

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

    for(uint8_t idx = 0; idx < CMD_TABLE_SIZE; idx++) {

        if (!memcmp(pStrCmd, cmdTable[idx].cmdMethod, CMD_METHOD_LNG) &&
        !memcmp(pStrCmd + CMD_METHOD_LNG + CMD_DELIMITER_LNG, cmdTable[idx].cmdName, CMD_NAME_LNG)) {

            result = cmdTable[idx].cmdFunc(pStrCmd, lng);
            break;
        }
    }
    /* printf redirected to UART in uart_interface.c*/
    printf("<< %s  >> RET = 0x%04x\n", pStrCmd, result);

    return result;
}
