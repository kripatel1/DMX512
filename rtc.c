#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "rtc.h"

uint8_t month =0;
uint8_t day = 0;
uint8_t days_inMonths[12] = {31,28,30,31,30,31,30,31,31,30,31,30,31};
void initRTC_Module()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;

    // TURN ON Hibernation Module
    SYSCTL_RCGCHIB_R |= SYSCTL_RCGCHIB_R0; // Hibernation Module 0x01;
    _delay_cycles(3);

    HIB_IM_R |= 0x11; // ENABLE WC INTERRUPT
    HIB_CTL_R = 0x0; // ENABLE OSCILLATOR INPUT |HIB_CTL_CLK32EN
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    HIB_CTL_R = 0x49; // ENABLE OSCILLATOR INPUT |HIB_CTL_CLK32EN
    while(!(HIB_CTL_R & HIB_CTL_WRC)); // WAIT FOR WRC TO BE SET BEFORE MOVING FOWWARD
    HIB_CTL_R;

    HIB_RTCLD_R = 1;
    HIB_IC_R  |= 0x10; // Clear WC interrupt;
}
void setCurrentSeconds(uint32_t seconds)
{
  HIB_RTCLD_R = seconds;
}
// Get current Time of day function
uint32_t getCurrentSeconds()
{
    return (HIB_RTCC_R % seconds_in_a_day);
}
void set_time(uint8_t hour, uint8_t min, uint8_t seconds)
{
    uint32_t value =(hour * 60* 60) + (min * 60) + seconds;
    setCurrentSeconds(value);
}
void get_time(uint8_t *hour, uint8_t *min, uint8_t *seconds)
{
    uint32_t getCurrent_Seconds = getCurrentSeconds();
    *hour = getCurrent_Seconds / (60*60);
    *min = (getCurrent_Seconds - (*hour *(60*60))) /(60);
    *seconds = getCurrent_Seconds - (*hour *(60*60)) -(*min *60);

}
void setdate(uint8_t month, uint16_t day)
{
    uint8_t mon = month -1;
    uint8_t date = day -1;
    uint32_t count = 0;

    int i=0;
    for(i =0; i < mon; i++)
    {
        count = count + days_inMonths[i];
    }
    count = count + date;
    count = count *(24*60*60) + getCurrentSeconds();
    setCurrentSeconds(count);
}
void setAlarm(uint8_t hour, uint8_t min, uint8_t seconds)
{
    uint32_t value =(hour * 60* 60) + (min * 60) + seconds;
    HIB_RTCM0_R = value;
}
