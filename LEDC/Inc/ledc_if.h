/*
 * ledc_if.h
 *
 *  Created on: Jul 15, 2022
 *      Author: 42077
 */

#ifndef LEDC_IF_H_
#define LEDC_IF_H_

#include "ledc_port.h"



#define MAX_STRING_LNG (uint32_t)(32)
#define OFFSET_WHITESPACE (uint32_t)(6)

#define LEDC_RET_BUSY (uint32_t)(1)
#define LEDC_RET_ERR  (uint32_t)(2)
#define LEDC_RET_OK   (uint32_t)(0)

/* Kind of "normal" display - output functions */
uint32_t LEDC_InitHW(void);
uint32_t LEDC_SetNewRollingString(const char * str, uint32_t length);
uint32_t LEDC_GetRollingStatus(void);
uint32_t LEDC_SetNewStandingText(const char * str);
uint32_t LEDC_StopStandingText(void);

/*To be called periodically using timer, isr ... whatever (cca 200x second)*/
uint32_t LEDC_PeriodicDisplayService(void);

#endif /* LEDC_IF_H_ */
