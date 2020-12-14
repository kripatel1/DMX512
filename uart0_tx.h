// UART0 Library
// 
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef UART0_TX_H_
#define UART0_TX_H_


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initUart0_Interrupts_ENABLED();
void displayUart0(char* str);



#endif
