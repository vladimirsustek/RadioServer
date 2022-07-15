#include "cmd_rda5807m.h"

uint16_t CmdRDA5807mDoInit(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

	RDA5807mInit();

	return CMD_RET_OK;
}

uint16_t CmdRDA5807mDoReset(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {

        return CMD_RET_ERR;
    }

	RDA5807mReset();
    return CMD_RET_OK;

}

uint16_t CmdRDA5807mSetMute(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG*2 + CMD_ARG1_LNG+
         CMD_EOL_LNG) != lng) {

        return CMD_RET_ERR;
    }

    uint8_t mute = pStrCmd[CMD_ARG_OFFSET + 0] - '0';

	uint16_t result = RDA5807mMute(mute);

	result = (RDA5807M_FN_OK == result) ? CMD_RET_OK : CMD_RET_ERR;

	return result;
}

uint16_t CmdRDA5807mSetFreq(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG*2 + CMD_ARG5_LNG+
         CMD_EOL_LNG) != lng) {

        return CMD_RET_ERR;
    }

    uint16_t freq = (pStrCmd[CMD_ARG_OFFSET + 0] - '0')*10000;

    freq += (pStrCmd[CMD_ARG_OFFSET + 1] - '0')*1000;
    freq += (pStrCmd[CMD_ARG_OFFSET + 2] - '0')*100;
    freq += (pStrCmd[CMD_ARG_OFFSET + 3] - '0')*10;
    freq += (pStrCmd[CMD_ARG_OFFSET + 4] - '0')*1;

	uint16_t result = RDA5807mSetFreq(freq);

    result = (RDA5807M_FN_OK == result) ? CMD_RET_OK : CMD_RET_ERR;

    return result;
}

uint16_t CmdRDA5807mSetVolm(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG*2 + CMD_ARG2_LNG+
         CMD_EOL_LNG) != lng) {

        return CMD_RET_ERR;
    }

    uint8_t volm;

    volm = (pStrCmd[CMD_ARG_OFFSET + 0] - '0')*10;
    volm += (pStrCmd[CMD_ARG_OFFSET + 1] - '0')*1;

	uint16_t result = RDA5807mSetVolm(volm);

    result = (RDA5807M_FN_OK == result) ? CMD_RET_OK : CMD_RET_ERR;

    return result;
}

uint16_t CmdRDA5807mGetRSSI(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

	return RDA5807mGetRSSI();
}

uint16_t CmdRDA5807mGetRDSR(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

	return RDA5807mGetRDSR();
}

uint16_t CmdRDA5807mGetRDSS(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

	return RDA5807mGetRDSS();
}

uint16_t CmdRDA5807mGetSTRO(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

	return RDA5807mIsChannelStereo();
}

uint16_t CmdRDA5807mGetCHST(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

	return RDA5807mIsChannelStation();
}

uint16_t CmdRDA5807mGetBLKA(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

    return RDA5807mGetRDSBlockA();

}

uint16_t CmdRDA5807mGetBLKB(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

    return RDA5807mGetRDSBlockB();

}

uint16_t CmdRDA5807mGetBLKC(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

    return RDA5807mGetRDSBlockD();

}

uint16_t CmdRDA5807mGetBLKD(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

    return RDA5807mGetRDSBlockD();

}

uint16_t CmdRDA5807mGetERRA(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

    return RDA5807mGetRDSBlockD();

}

uint16_t CmdRDA5807mGetERRB(const uint8_t* const pStrCmd, const uint8_t lng) {

    if ((CMD_METHOD_LNG + CMD_NAME_LNG +
         CMD_DELIMITER_LNG + CMD_EOL_LNG) != lng) {
        return CMD_RET_ERR;
    }

    return RDA5807mGetRDSBlockD();

}
