/*
 * eeprom_25aa1024.h
 *
 *  Created on: Aug 28, 2022
 *      Author: 42077
 */

#ifndef EEPROM_25AA1024_H_
#define EEPROM_25AA1024_H_

#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "spi.h"


#define EEPROM_READ				(uint8_t)(0b00000011) // Read data from memory array beginning at the selected address
#define EEPROM_WRITE			(uint8_t)(0b00000010) // Write data to memory array beginning at the selected address
#define EEPROM_WREN				(uint8_t)(0b00000110) // Set the write enable latch (enable write operations)
#define EEPROM_WRDI				(uint8_t)(0b00000100) // Reset the write enable latch (enable write operations)
#define EEPROM_RDSR				(uint8_t)(0b00000101) // Read STATUS register
#define EEPROM_WRSR				(uint8_t)(0b00000001) // Write STATUS register
#define EEPROM_PE				(uint8_t)(0b01000010) // Page erase - erase one page memory array
#define EEPROM_SE				(uint8_t)(0b11011000) // Sector erase - erase one sector in memory array
#define EEPROM_CE				(uint8_t)(0b11000111) // Chip erase - erase all sectors in memory array
#define EEPROM_PDID				(uint8_t)(0b10101011) // Release from Deep Power-Down and Read Electronic signature
#define EEPROM_DPD				(uint8_t)(0b10111001) // Deep Power-Down mode


#define EEPROM_START_ADDRESS	(uint32_t)(0x00000000)
#define EEPROM_STOP_ADDRESS		(uint32_t)(0x0001FFFF)

#define MAX_WRITE_AT_ONCE		(uint16_t)(256u)

#define STATUS_PROTECTION_0		(uint8_t)((0) << 2)
#define STATUS_PROTECTION_1		(uint8_t)((1) << 2)
#define STATUS_PROTECTION_2		(uint8_t)((2) << 2)
#define STATUS_PROTECTION_3		(uint8_t)((3) << 2)

#define STATUS_WIP				(uint8_t)(1)
#define STATUS_WEN				(uint8_t)(2)

#define EEPROM_PAGE_SIZE		(uint16_t)(256u)

/* Application's predefined addresses where "SSID","password" are stored
 * distance between addresses and therefore space is 128 bytes what is
 * enough for 32 maximal bytes SSID and maximal 64 bytes password*/
#define EEPROM_WIFI_ADR_0		(uint32_t)(0x018600)
#define EEPROM_WIFI_ADR_1		(uint32_t)(0x018680)
#define EEPROM_WIFI_ADR_2		(uint32_t)(0x018700)
#define EEPROM_WIFI_ADR_3		(uint32_t)(0x018780)

#define EEPROM_SYS_STATE_ADR	(uint32_t)(0x018800)

typedef struct sys_state_flags
{
	uint8_t rdaEnabled : 1, /* To be non-volatile */
			espEnabled : 1, /* To be non-volatile */

			espConnected : 1, /* Being checked on runtime, no reason for non-volatile */
			eepromFunctional : 1, /* Being checked on runtime, no reason for non-volatile */

	        dummy0xA : 4; /* To be non-volatile 0b1010 */
}sys_state_flags_t;

typedef struct sys_state
{
	uint16_t radioFreq;
	uint8_t radioVolm;
	sys_state_flags_t states;
}sys_state_t;

sys_state_t systemGlobalState;

uint32_t EEPROM_Init(void);
uint32_t EEPROM_WriteData(uint32_t address, uint8_t* pData, uint16_t Size);
uint32_t EEPROM_ReadData(uint32_t address, uint8_t *pData, uint16_t Size);
uint32_t EEPROM_ReadStatusRegister(uint8_t *ret);
uint32_t EEPROM_WriteStatusRegister(uint8_t value);
uint16_t Cmd25AA1024WrBytes(const uint8_t* const cmd, const uint16_t lng);
uint16_t Cmd25AA1024RdBytes(const uint8_t* const cmd, const uint16_t lng);
uint8_t* EEPROM_GetWIfi(uint32_t address, uint32_t offset, uint8_t* pSSIDpassword);
uint32_t EEPROM_GetSystemState(void);
uint32_t EEPROM_SetSystemState(void);
#endif /* EEPROM_25AA1024_H_ */
