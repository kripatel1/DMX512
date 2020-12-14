// UART1 LIBRARY
// kripatel1
// DMX512 TRANSMITING 8N2 AT 250 KBAUD

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

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart1_tx.h"
#include "timer1.h"
#include "gpio.h"
#include "wait.h"
#include "DMX.h"






// PortC masks
#define UART_RX_MASK 16           // PC 4
#define UART_TX_MASK 32           // PC 5
#define UART_EN_MASK 128          // PC 7
#define DE PORTC,7
#define URX PORTC,4
#define UTX PORTC,5

void initUart1_Interrupts_ENABLED()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
       SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

       // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
       SYSCTL_GPIOHBCTL_R = 0;

       // Enable clocks
       SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1; // UART1
       SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2; // PORTC
       _delay_cycles(3);

       // Configure UART1 pins
       GPIO_PORTC_DIR_R   |= UART_TX_MASK | UART_EN_MASK;                 // enable output on UART1 TX pin
       GPIO_PORTC_DIR_R   &= ~UART_RX_MASK;                               // enable input on UART1 RX pin
       GPIO_PORTC_DR2R_R  |= UART_TX_MASK | UART_EN_MASK;                 // set drive strength to 2mA (not needed since default configuration -- for clarity)
       GPIO_PORTC_PUR_R   |= UART_RX_MASK | UART_TX_MASK;
       GPIO_PORTC_DEN_R   |= UART_TX_MASK | UART_EN_MASK | UART_RX_MASK; // enable digital on UART1 pins
       GPIO_PORTC_AFSEL_R |= UART_TX_MASK | UART_RX_MASK ;                               // use peripheral to drive PC4, PC5
       GPIO_PORTC_PCTL_R  &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M);        // clear bits 16-23
       GPIO_PORTC_PCTL_R  |=  GPIO_PCTL_PC5_U1TX | GPIO_PCTL_PC4_U1RX;
                                                           // select UART1 to drive pins PA0 and PA1: default, added for clarity

       // Configure UART1 to 115200 baud, 8N2 format
       UART1_CTL_R = 0;                                    // turn-off UART1 to allow safe programming
       UART1_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (40 MHz)
       UART1_IBRD_R = 10;                                  // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
       UART1_FBRD_R = 0;                                  // round(fract(r)*64)=45
       UART1_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_STP2;   // configure for 8N2 and 2 STOP BITS 16-level FIFO [DISABLED]

       // Configuring for intterrupt
       UART1_IFLS_R |= UART_IFLS_TX1_8 | UART_IFLS_RX1_8;  // specifies how much of the FIFO full is needed to trigger INTERRUPT
       UART1_IM_R   |= UART_IM_TXIM;                       // Specifies TX EOT to trigger interrup
       NVIC_EN0_R   |= 1 << 6;                             // INTERRUPT NUMBER IS 6

}
void setUart1_toGPIO()
{
    UART1_CTL_R        &= ~(UART_CTL_EOT | UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN);
    GPIO_PORTC_DEN_R   &= ~(UART_TX_MASK| UART_RX_MASK);
    GPIO_PORTC_PUR_R   &= ~(UART_RX_MASK |UART_TX_MASK) ;
    GPIO_PORTC_PCTL_R  &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M); // clear bits 16-23
    GPIO_PORTC_AFSEL_R  = 0;  // use peripheral to drive PA0, PA1
    GPIO_PORTC_DEN_R   |= UART_TX_MASK| UART_RX_MASK;

}
void setUart1_toUart1()
{
    GPIO_PORTC_DEN_R   &= ~(UART_TX_MASK| UART_RX_MASK);
    GPIO_PORTC_PUR_R   |= UART_RX_MASK | UART_TX_MASK ;
    GPIO_PORTC_AFSEL_R |= UART_TX_MASK |UART_RX_MASK ;               // use peripheral to drive PA0, PA1
    GPIO_PORTC_PCTL_R  &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M);      // clear bits 16-23
    GPIO_PORTC_PCTL_R  |=  GPIO_PCTL_PC5_U1TX| GPIO_PCTL_PC4_U1RX;   // select UART1 to drive pins PA0 and PA1: default, added for clarity
    GPIO_PORTC_DEN_R   |=  UART_TX_MASK | UART_RX_MASK;
    UART1_CTL_R = UART_CTL_EOT | UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;


}
void transmitBREAK()
{
    setPinValue(DE,true);
    setPinValue(UTX,false);
    setPinValue(URX,true);


}
void transmitMAB()
{
    setPinValue(UTX,true);
    setPinValue(URX,false);
}

void setUart1BaudRate(uint32_t baudRate, uint32_t fcyc)
{
    uint32_t divisorTimes128 = (fcyc * 8) / baudRate;     // calculate divisor (r) in units of 1/128,
                                                          // where r = fcyc / 16 * baudRate
    UART1_IBRD_R = divisorTimes128 >> 7;                  // set integer value to floor(r)
    UART1_FBRD_R = ((divisorTimes128 + 1)) >> 1 & 63;     // set fractional value to round(fract(r)*64)
}
void putcUart1(uint8_t val)
{
    while (UART1_FR_R & UART_FR_TXFF);
//    while (!(UART1_FR_R & UART_FR_RXFE));
    UART1_DR_R = val;                                     // write character to fifo
}
char getcUart1()
{
    while (UART1_FR_R & UART_FR_RXFE);               // wait if UART1 rx fifo empty
    return UART1_DR_R & 0xFF;                        // get character from fifo
}



