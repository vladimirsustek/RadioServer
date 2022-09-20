/*
 * ledc_if.c
 *
 *  Created on: Jul 15, 2022
 *      Author: Vladimir Sustek, MSc
 *
 *
 *      Simple fully functional Common-anode 4-digit LED display driver
 *
 *      1) Displays 4-digits SetNewStandingText("1234")
 *      2) Displays rolling text SetNewRollingString("Whatever you want", strlen("Whatever you want"))
 *
 */

#include "ledc_if.h"


#define ROLLING_SPEED_600_MS (uint8_t)(128 - 1)
#define ROLLING_SPEED_300_MS (uint8_t)(64 - 1)

#define NUMBER_OF_ANODES	 (uint8_t)(4)

#define FIRST_ANODE	         (uint32_t)(0x1)
#define LAST_ANODE	         (uint32_t)(0x8)

static uint32_t rollingStringBusyFlag = 0;

static uint32_t standingStringBusyFlag = 0;

static uint32_t infinitiveRollingStringFlag = 0;

static uint8_t internalStrinBuff[MAX_STRING_LNG + OFFSET_WHITESPACE + 1 + 1];

static uint32_t internalStringLentgh = 0;

static uint8_t textPointer = 0;

static uint8_t multiplexedAnode= 1;

static uint8_t full7Segment = 0;

static uint8_t counter360ms = 0;

static uint8_t internalStandingStringBUff[NUMBER_OF_ANODES];

TIM_HandleTypeDef htim2;

/* Simple conversion between ASCII and 7-seg
 *
 * May happen, this does not fit to your design
 * thus simply change bit's ...
 *              A - 0x01
 *            _____________
 *            |            |
 *  F - 0x20  |            |  B - 0x02
 *            |            |
 *            |____________|
 *            |  G - 0x40  |
 *  E - 0x10  |            |  C - 0x04
 *            |            |
 *            |____________|* - DP 0x80
 *               D - 0x8
 *
 *
 *
 * */
static uint8_t ASCII27SEG(uint8_t ascii)
{
	uint8_t encoded;

	switch(ascii)
	{
	case '0': encoded = 0b00111111; break;
	case '1': encoded = 0b00000110; break;
	case '2': encoded = 0b01011011; break;
	case '3': encoded = 0b01001111; break;
	case '4': encoded = 0b01100110; break;
	case '5': encoded = 0b01101101; break;
	case '6': encoded = 0b01111101; break;
	case '7': encoded = 0b00000111; break;
	case '8': encoded = 0b01111111; break;
	case '9': encoded = 0b01101111; break;
	case 'A': encoded = 0b01110111; break;
	case 'a': encoded = 0b01110111; break;
	case 'B': encoded = 0b01111100; break;
	case 'b': encoded = 0b01111100; break;
	case 'C': encoded = 0b00111001; break;
	case 'c': encoded = 0b00111001; break;
	case 'D': encoded = 0b01011110; break;
	case 'd': encoded = 0b01011110; break;
	case 'E': encoded = 0b01111001; break;
	case 'e': encoded = 0b01111001; break;
	case 'F': encoded = 0b01110001; break;
	case 'f': encoded = 0b01110001; break;
	case 'G': encoded = 0b00111101; break;
	case 'g': encoded = 0b00111101; break;
	case 'H': encoded = 0b01110100; break;
	case 'h': encoded = 0b01110100; break;
	case 'I': encoded = 0b00110000; break;
	case 'i': encoded = 0b00110000; break;
	case 'J': encoded = 0b00011110; break;
	case 'j': encoded = 0b00011110; break;
	case 'K': encoded = 0b01110101; break;
	case 'k': encoded = 0b01110101; break;
	case 'L': encoded = 0b00111000; break;
	case 'l': encoded = 0b00111000; break;
	case 'M': encoded = 0b01010101; break;
	case 'm': encoded = 0b01010101; break;
	case 'N': encoded = 0b01010100; break;
	case 'n': encoded = 0b01010100; break;
	case 'O': encoded = 0b01011100; break;
	case 'o': encoded = 0b01011100; break;
	case 'P': encoded = 0b01110011; break;
	case 'p': encoded = 0b01110011; break;
	case 'Q': encoded = 0b01100111; break;
	case 'q': encoded = 0b01100111; break;
	case 'R': encoded = 0b01010000; break;
	case 'r': encoded = 0b01010000; break;
	case 'S': encoded = 0b01101101; break;
	case 's': encoded = 0b01101101; break;
	case 'T': encoded = 0b01111000; break;
	case 't': encoded = 0b01111000; break;
	case 'U': encoded = 0b00111110; break;
	case 'u': encoded = 0b00111110; break;
	case 'V': encoded = 0b00011100; break;
	case 'v': encoded = 0b00011100; break;
	case 'W': encoded = 0b01101010; break;
	case 'w': encoded = 0b01101010; break;
	case 'X': encoded = 0b01110110; break;
	case 'x': encoded = 0b01110110; break;
	case 'Y': encoded = 0b01101110; break;
	case 'y': encoded = 0b01101110; break;
	case 'Z': encoded = 0b01011011; break;
	case 'z': encoded = 0b01011011; break;
	case '.': encoded = 0b10000000; break;
	case '-': encoded = 0b01000000; break;
	case '>': encoded = 0b01001100; break;
	case '<': encoded = 0b01011000; break;
	case '*': encoded = 0b01100011;	break;
	default: encoded = 0x00; break;
	}
	return encoded;
}


uint32_t LEDC_InitHW(void)
{
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
	return 0;
}

uint32_t LEDC_GetRollingStatus(void)
{
	return rollingStringBusyFlag;
}

/* Shows only (first str's) 4-digits and does not roll*/
uint32_t LEDC_SetNewStandingText(const char * str)
{
	uint32_t retVal  = LEDC_RET_OK;

	if (rollingStringBusyFlag)
	{
		retVal = LEDC_RET_BUSY;

	}
	else if (NULL == str)
	{
		retVal = LEDC_RET_ERR;
	}
	else
	{
		if(!standingStringBusyFlag) /* Do not touch multiplexing if it is already running */
		{
			multiplexedAnode= 1;

			full7Segment = 0;
		}

		standingStringBusyFlag = 1;

		memcpy(internalStandingStringBUff, str, NUMBER_OF_ANODES);



	}
	return retVal;
}


/* Clear the display after standing text - stop's multiplexing*/
uint32_t LEDC_StopStandingText(void)
{
	uint32_t retVal  = LEDC_RET_OK;

	if (!standingStringBusyFlag)
	{
		retVal = LEDC_RET_ERR;

	}
	else
	{
		standingStringBusyFlag = 0;
	}
	return retVal;
}

/* Starts rolling text on the display padded in the beginning and end with 3 WHITESPACES
 * function has a built-in mechanism to reject new set request if the previous text was
 * not rolled/printed yet - can be called periodically and will do action only if it can.
 *
 * Maximal string length is MAX_STRING_LNG
 *
 */
uint32_t LEDC_SetNewRollingString(const char * str, uint32_t length)
{
	uint32_t retVal  = LEDC_RET_OK;

	if (rollingStringBusyFlag || standingStringBusyFlag)
	{
		retVal = LEDC_RET_BUSY;

	}
	else if (NULL == str || MAX_STRING_LNG <= length)
	{
		retVal = LEDC_RET_ERR;
	}
	else
	{
		memset(internalStrinBuff, '\0', MAX_STRING_LNG + OFFSET_WHITESPACE + 1 + 1);

		rollingStringBusyFlag = LEDC_RET_BUSY;

		memset(internalStrinBuff, ' ', OFFSET_WHITESPACE/2);

		memcpy(internalStrinBuff + OFFSET_WHITESPACE/2, str, length);

		memset(internalStrinBuff + length + OFFSET_WHITESPACE/2, ' ', OFFSET_WHITESPACE/2);

		internalStringLentgh = length + OFFSET_WHITESPACE + sizeof('\0');

		textPointer = 0;

		multiplexedAnode= 1;

		full7Segment = 0;

		counter360ms = 1;
	}
	return retVal;
}

/*
 * When routine is called 200x per second display is OK
 * and this means that each digit is 200/4 = 50x per second
 * viewed
 *
 * THUS ROUTINE MUST BE CALLED PERIODICALLY e.g.
 *
 *
 * ISR(I_AM_200HZ_TIMER)
 * {
 *     LEDC_PeriodicDisplayService();
 * }
 *
 * or
 *
 * ISR(I_AM_200HZ_TIMER)
 * {
 *     period200HzElapsedFlag = 1;
 * }
 *
 * main()
 * {
 *     while(1)
 *     {
 *         doSomethingNotBlockingNotTooLong();
 *         if(period200HzElapsedFlag)
 *         {
 *             period200HzElapsedFlag = 0;
 *             LEDC_PeriodicDisplayService();
 *         }
 *     }
 * }
 * */

uint32_t LEDC_PeriodicDisplayService(void)
{
	/* Routine takes  6-12us when core clock 56MHz*/
	uint32_t stop = 0;

	if (rollingStringBusyFlag)
	{

		LEDC_WRITE_7SEG(ASCII27SEG(internalStrinBuff[textPointer + full7Segment]));
		LEDC_WRITE_ANODES(multiplexedAnode);

	    if (multiplexedAnode == LAST_ANODE)
	    {
	    	multiplexedAnode = FIRST_ANODE;
	    }
	    else
	    {
	    	multiplexedAnode = multiplexedAnode << 1;
	    }

	    full7Segment = (full7Segment + 1) & (NUMBER_OF_ANODES - 1);

		if (!counter360ms)
		{
			textPointer++;
		}

		counter360ms = (counter360ms + 1) & (ROLLING_SPEED_300_MS);

		if((internalStringLentgh - 1) == textPointer)
		{
			if(infinitiveRollingStringFlag)
			{
				textPointer = 0;
			}
			else
			{
				rollingStringBusyFlag = LEDC_RET_OK;
			}
		}
	}
	else if (standingStringBusyFlag)
	{
	    if (multiplexedAnode == LAST_ANODE)
	    {
	    	multiplexedAnode = FIRST_ANODE;
	    }
	    else
	    {
	    	multiplexedAnode = multiplexedAnode << 1;
	    }

	    full7Segment = (full7Segment + 1) & (NUMBER_OF_ANODES -1);

	    LEDC_WRITE_7SEG(ASCII27SEG(internalStandingStringBUff[full7Segment]));
		LEDC_WRITE_ANODES(multiplexedAnode);
	}
	else
	{
		LEDC_WRITE_ANODES(0);
	}

	return stop;
}

uint32_t LEDC_SetNewInfiniteRollingString(const char * str)
{
	infinitiveRollingStringFlag = 1;
	return LEDC_SetNewRollingString(str, strlen(str));
}

uint32_t LEDC_StopInfiniteRollingString(void)
{
	infinitiveRollingStringFlag = 0;
	return 0;
}
