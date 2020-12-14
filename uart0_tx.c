
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

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0_tx.h"


#define MAX_BUFFER 256


char TXBUFFER[MAX_BUFFER];
uint16_t WR_IDX = 0;             // dataType should be changed based on buffer size
uint16_t RD_IDX = 0;
uint8_t  WR_WRAPPED_AROUND = 0;


// PortA masks
#define UART_RX_MASK 1
#define UART_TX_MASK 2



//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize UART0
void initUart0_Interrupts_ENABLED()
{
                                // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable clocks
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
    _delay_cycles(3);

    // Configure UART0 pins
    GPIO_PORTA_DIR_R   |= UART_TX_MASK;                   // enable output on UART0 TX pin
    GPIO_PORTA_DIR_R   &= ~UART_RX_MASK;                   // enable input on UART0 RX pin
    GPIO_PORTA_DR2R_R  |= UART_TX_MASK;                  // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTA_DEN_R   |= UART_TX_MASK | UART_RX_MASK;    // enable digital on UART0 pins
    GPIO_PORTA_AFSEL_R |= UART_TX_MASK | UART_RX_MASK;  // use peripheral to drive PA0, PA1
    GPIO_PORTA_PCTL_R  &= ~(GPIO_PCTL_PA1_M | GPIO_PCTL_PA0_M); // clear bits 0-7
    GPIO_PORTA_PCTL_R  |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
                                                        // select UART0 to drive pins PA0 and PA1: default, added for clarity

    // Configure UART0 to 115200 baud, 8N1 format
    UART0_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
    UART0_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (40 MHz)
    UART0_IBRD_R = 21;                                  // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                                  // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8;                    // configure for 8N1 16-level FIFO [DISABLED]

    // Configuring for intterrupt
    UART0_IFLS_R |= UART_IFLS_TX1_8 | UART_IFLS_RX1_8;  // specifies how much of the FIFO full is needed to trigger INTERRUPT
    UART0_IM_R   |= UART_IM_TXIM;                       // Specifies TX EOT to trigger interrup
    NVIC_EN0_R   |= 1<< 5;                              // INTERRUPT NUMBER IS 5

    UART0_CTL_R = UART_CTL_EOT | UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
                            // enable TX, RX, EOT, and module
}
void uart0ISR()
{

    // WR_WRAPPED_AROUND MEANS WR AND RD INDEX IS less than RD-> EMPTY
    // TO CLEAR WR_WRAPPED_AROUND  RD_IDX needs to wrapp around
    if(RD_IDX < WR_IDX) // CHECK IF TX BUFFER IS EMPTY
    {
        UART0_ICR_R |= UART_ICR_TXIC;     // clears TX Interrupt
        UART0_DR_R = TXBUFFER[RD_IDX];
        RD_IDX = (RD_IDX+1) % MAX_BUFFER;
    }
    else if(WR_WRAPPED_AROUND)
    {
        if(((RD_IDX+1) % MAX_BUFFER) == 0)
        {
            UART0_ICR_R |= UART_ICR_TXIC;     // clears TX Interrupt
            UART0_DR_R = TXBUFFER[RD_IDX];
            WR_WRAPPED_AROUND = 0; // CLEARS WR_WRAP BECAUSE RD_IDX WRAPPED
            RD_IDX = (RD_IDX+1) % MAX_BUFFER;

        }
        else
        {
            UART0_ICR_R |= UART_ICR_TXIC;     // clears TX Interrupt
            UART0_DR_R   = TXBUFFER[RD_IDX];
            RD_IDX       = (RD_IDX+1) % MAX_BUFFER;
        }
    }
    else
    {
        UART0_ICR_R  |= UART_ICR_TXIC;          // clears TX Interrupt
        UART0_CTL_R  &= ~UART_CTL_UARTEN;     // TURN OFF UART0
        UART0_IM_R   &= ~UART_IM_TXIM;        // DISABLE TXFE-> EOT INTERRUPT.
        UART0_CTL_R  |= UART_CTL_UARTEN;      // TURN ON UART0
    }
}
// Copies string to buffer
void displayUart0(char* str)
{
    uint16_t i = 0;
    uint8_t primed =0;
    // copy string to buffer
    while (str[i] != '\0')
    {
        if(!WR_WRAPPED_AROUND)
        {
            TXBUFFER[WR_IDX] = str[i];
            i++;
            // peeking next value to see if WR_IDX wraps
            if( ((WR_IDX+1) % MAX_BUFFER) == 0)
            {
                WR_WRAPPED_AROUND = 1;
                primed =0;
            }
            WR_IDX = (WR_IDX+1) % MAX_BUFFER;
        }
        else if(WR_WRAPPED_AROUND)
        {
            if(WR_IDX < RD_IDX)
            {
                TXBUFFER[WR_IDX] = str[i];
                i++;
                WR_IDX = (WR_IDX+1) % MAX_BUFFER;
            }
            else if(!primed)
            {
                primed =1;
                if((UART0_FR_R & UART_FR_TXFE))
                {

                    UART0_CTL_R  &= ~UART_CTL_UARTEN;   // TURN OFF UART0
                    UART0_IM_R   |= UART_IM_TXIM;       // ENABLE TXFE-> EOT INTERRUPT.
                    UART0_CTL_R  |= UART_CTL_UARTEN;    // TURN ON UART0
                    UART0_DR_R    = TXBUFFER[RD_IDX++];   // PRIME CHAR
                }
            }
       }
    }
    if((UART0_FR_R & UART_FR_TXFE))
    {

        UART0_CTL_R  &= ~UART_CTL_UARTEN;   // TURN OFF UART0
        UART0_IM_R   |= UART_IM_TXIM;       // ENABLE TXFE-> EOT INTERRUPT.
        UART0_CTL_R  |= UART_CTL_UARTEN;    // TURN ON UART0
        UART0_DR_R    = TXBUFFER[RD_IDX++];   // PRIME CHAR
    }
}

