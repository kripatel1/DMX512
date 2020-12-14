// EEPROM functions
// 

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    -

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Block Number for EEPROM to RW
#define BLOCK_NUMBER    2
#define MAX_OFFSET      16


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


void initEEPROM_Module();
void writeEeprom(uint16_t add, uint32_t data);
uint32_t readEeprom(uint16_t add);

#endif
