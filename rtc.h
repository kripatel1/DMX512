#ifndef rtc_H_
#define rtc_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#define seconds_in_a_day    86400


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initRTC_Module();
void setCurrentSeconds(uint32_t seconds);
void set_time(uint8_t hour, uint8_t min, uint8_t seconds);
void get_time(uint8_t *hour, uint8_t *min, uint8_t *seconds);
uint32_t getCurrentSeconds();
void setdate(uint8_t month, uint16_t day);
void setAlarm(uint8_t hour, uint8_t min, uint8_t seconds);
#endif
