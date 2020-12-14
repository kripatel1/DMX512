
// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// UART Interface:
//   U1TX (PC5) and U1RX (PC4) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------


#ifndef UART1_TX_H_
#define UART1_TX_H_


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initUart1_Interrupts_ENABLED();
void setUart1BaudRate(uint32_t baudRate, uint32_t fcyc);
void putcUart1(uint8_t val);

void setUart1_toUart1();
void setUart1_toGPIO();
void transmitBREAK();
void transmitMAB();
void step6_iskillingme();


#endif


