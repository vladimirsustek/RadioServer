/*
 * ledc_port.h
 *
 *  Created on: Jul 14, 2022
 *      Author: 42077
 */

#ifndef LEDC_PORT_H_
#define LEDC_PORT_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"


/* Bit order for segments of 7 - seg => 0xFF all segments*/

#define LEDC_7SEG_A (uint8_t)(0)
#define LEDC_7SEG_B (uint8_t)(1)
#define LEDC_7SEG_C (uint8_t)(2)
#define LEDC_7SEG_D (uint8_t)(3)
#define LEDC_7SEG_E (uint8_t)(4)
#define LEDC_7SEG_F (uint8_t)(5)
#define LEDC_7SEG_G (uint8_t)(6)
#define LEDC_7SEG_DP (uint8_t)(7)

/* Bit order for anodes => 0x0F all anodes*/
#define LEDC_A1	(uint8_t)(0)
#define LEDC_A2	(uint8_t)(1)
#define LEDC_A3	(uint8_t)(2)
#define LEDC_A4	(uint8_t)(3)

/*
 * LEDC_A_Pin is PA9 at PORTA
 * LEDC_F_Pin is P12 at PORTA
 * LEDC_G_Pin is PA8 at PORTA
 *
 * LEDC_B_Pin is PB6 at PORTB
 * LEDC_C_Pin is PB15 at PORTB
 * LEDC_D_Pin is PB13 at PORTB
 * LEDC_E_Pin is PB12 at PORTB
 * LEDC_DP_Pin is PB14 at PORTB
 *
 * LEDC_A2_Pin is PA15 = PORTA
 *
 * LEDC_A1_Pin is PB3 at PORTB
 * LEDC_A3_Pin is PB7 at PORTB
 * LEDC_A4_Pin is PB4 at PORTB
 *
 */


/* Macros to write segments by direct access the registers
 *
 * This function drives N-FET transistors, so "active high"
 *
 * 1) Zeroing BSRR register bits (GPIO pins) located at 31:16
 * 2) Setting BSSR register bits (GPIO pins) located at 15:00
 *
 * expression (((segments) & (1<<LEDC_7SEG_F)) << 5) means:
 *
 * If segments contains a set bit of the LEDC_7SEG_F, note that
 * LEDC_7SEG_F is has value 5, shift the expression 7x left
 * because LEDC_A_Pin is at 12th position (5+7 = 12)
 */
#define LEDC_WRITE_7SEG(segments) { \
   GPIOA->BSRR = (LEDC_A_Pin | LEDC_F_Pin | LEDC_G_Pin) << 16; \
   \
   GPIOB->BSRR = (LEDC_C_Pin | LEDC_B_Pin | LEDC_D_Pin | LEDC_E_Pin | LEDC_DP_Pin) << 16; \
   \
   GPIOA->BSRR = (((segments) & (1<<LEDC_7SEG_A)) << 9) \
			   | (((segments) & (1<<LEDC_7SEG_F)) << 5) \
			   | (((segments) & (1<<LEDC_7SEG_G)) << 2); \
   \
   GPIOB->BSRR = (((segments) & (1<<LEDC_7SEG_C)) << 13) \
   			   | (((segments) & (1<<LEDC_7SEG_B)) << 5) \
			   | (((segments) & (1<<LEDC_7SEG_D)) << 10) \
			   | (((segments) & (1<<LEDC_7SEG_E)) << 8) \
			   | (((segments) & (1<<LEDC_7SEG_DP)) << 7); \
}

/* Macros to write segments by direct access the registers
 *
 * This function drives P-FET transistors, so "active low"
 *
 * 1) Setting BSRR register bits (GPIO pins) located at 15:00
 * 2) Setting BSSR register bits (GPIO pins) located at 31:16
 */
#define LEDC_WRITE_ANODES(anodes) { \
	GPIOA->BSRR = LEDC_A2_Pin; \
	GPIOB->BSRR = (LEDC_A1_Pin|LEDC_A3_Pin|LEDC_A4_Pin); \
    \
	GPIOA->BSRR = (((anodes) & (1 << LEDC_A2)) << 14) << 16; \
	GPIOB->BSRR = ((((anodes) & (1 << LEDC_A1)) << 3) << 16) | \
			((((anodes) & (1 << LEDC_A3)) << 5) << 16) | \
			((((anodes) & (1 << LEDC_A4)) << 1) << 16); \
}

#endif /* LEDC_PORT_H_ */
