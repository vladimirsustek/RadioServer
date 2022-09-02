/*
 * eeprom_nonvol.h
 *
 *  Created on: Sep 2, 2022
 *      Author: 42077
 */

#ifndef EEPROM_NONVOL_H_
#define EEPROM_NONVOL_H_

#include "eeprom_25aa1024.h"

#define SSID_MAX		(uint32_t)(32)
#define PASSWORD_MAX	(uint32_t)(32)
#define RADIONAME_MAX	(uint32_t)(14)

#pragma pack(4)
typedef struct wifi_entry
{
	char SSID[32];
	char password[64];
}wifi_entry_t;

#pragma pack(4)
typedef struct radio_entry
{
	char name[14];
	uint16_t freq;
};

#endif /* EEPROM_NONVOL_H_ */
