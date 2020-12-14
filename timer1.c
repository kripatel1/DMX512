
// TIMER1 LIBRARY
// kripatel1

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "timer1.h"
#include "gpio.h"
#include "uart1_tx.h"

#define DIGITAL_ENABLE PORTC,7
#define RED_LED PORTF,1


void initTimer1HW()
{

    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
   SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

   // Enable clocks
   SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;      // MASK IS  0x10
   _delay_cycles(3);

   TIMER1_CTL_R  &= ~0x01;
   TIMER1_CFG_R   = TIMER_CFG_32_BIT_TIMER;
   TIMER1_TAMR_R  = TIMER_TAMR_TAMR_PERIOD;          // 0x02
   TIMER1_TAILR_R = 40000000*1;                      // 1 SECOND INTERRUPT -> CLOCK*N
   TIMER1_IMR_R   = TIMER_IMR_TATOIM;                // 0x01 turn on intterupts
   NVIC_EN0_R    |= 1<< 21;                          // interrupt 21 bit 21
//   TIMER1_CTL_R  |= TIMER_CTL_TAEN;                  // turn-on timer 0x01
}



void turnONTimer1()
{
    TIMER1_CTL_R  |= TIMER_CTL_TAEN;                  // turn-on timer 0x01
}

void turnOFFTimer1()
{
    TIMER1_CTL_R  &= ~TIMER_CTL_TAEN;                  // turn-on timer 0x01
}
void setTimer1(uint32_t val)
{
    TIMER1_TAILR_R = val;
}
