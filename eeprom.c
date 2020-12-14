// EEPROM functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    -

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "eeprom.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


void initEEPROM_Module()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;

    SYSCTL_RCGCEEPROM_R = SYSCTL_RCGCEEPROM_R0; // Turn on EEPROM Run Mode Clock Gating
    _delay_cycles(7); // Must wait 6 cycles
    while(EEPROM_EEDONE_R  & EEPROM_EEDONE_WORKING); // DONE working 0x1;
    while((EEPROM_EESUPP_R & 0x8) || (EEPROM_EESUPP_R & 0x4));

    SYSCTL_SREEPROM_R |= 0x1; // Reset EEPROM
    SYSCTL_SREEPROM_R &= ~SYSCTL_SREEPROM_R0; // Complete Reset by clearing bit
    _delay_cycles(7); // Must wait 6 cycles
    while(EEPROM_EEDONE_R  & EEPROM_EEDONE_WORKING); // DONE working 0x1;
    while((EEPROM_EESUPP_R & 0x8) || (EEPROM_EESUPP_R & 0x4));
}

void writeEeprom(uint16_t add, uint32_t data)
{
    EEPROM_EEBLOCK_R = add >> 4;
    EEPROM_EEOFFSET_R = add & 0xF;
    EEPROM_EERDWR_R = data;
    while (EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
}

uint32_t readEeprom(uint16_t add)
{
    EEPROM_EEBLOCK_R = add >> 4;
    EEPROM_EEOFFSET_R = add & 0xF;
    return EEPROM_EERDWR_R;
}
