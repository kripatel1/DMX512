// DMX LIBRARY
// kripatel1

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "tm4c123gh6pm.h"
#include "DMX.h"
#include "gpio.h"
#include "timer1.h"
#include "wait.h"
#include "uart1_tx.h"
#include "eeprom.h"


#define DIGITAL_ENABLE PORTC,7
#define RED_LED PORTF,1
#define GREEN_LED PORTF,3
#define BLUE_LED PORTF,2
#define DE PORTC,7
#define  MAX_DEVICES 513
// Block 0 -> mode
// CONTROL MODE HAS 0xAAAAAAAA
// Device Mode has  0xffffffff

// Block 1 -> address
uint32_t mode = 0xFFFFFFFF; // Controller/DEVICE
uint32_t address = 0xFFFFFFFF;
uint8_t running =0;
uint16_t phase = -1;
uint16_t maxDevices = 513; // 513 because idx 0 == start code

uint8_t DMX_TXBUFFER[MAX_DEVICES];
uint16_t RD_IDX1 = 1;

bool devices_BUS[MAX_DEVICES];
uint16_t pollcounter = 0;
bool polling =0;

uint32_t timerVal = 0;
void timer1ISR()
{
    if(phase ==1)
    {

        TIMER1_CTL_R  &= ~TIMER_CTL_TAEN;
        TIMER1_TAILR_R = 480;
        TIMER1_ICR_R   = TIMER_ICR_TATOCINT;
        transmitMAB();
        phase = 2;
        TIMER1_CTL_R  |= TIMER_CTL_TAEN;

    }
    else if (phase ==2)
    {

        TIMER1_CTL_R  &= ~TIMER_CTL_TAEN;
        TIMER1_ICR_R   = TIMER_ICR_TATOCINT;
        phase = 3;
        setUart1_toUart1();
        primePumpUart1();
    }
    else if (phase == 7)
    {
        TIMER1_CTL_R  &= ~TIMER_CTL_TAEN;
        TIMER1_ICR_R   = TIMER_ICR_TATOCINT;
        phase = 1;
        uint16_t data = UART1_DR_R;
        if((data & 0x800))
        {
            devices_BUS[pollcounter] = true;
        }
        pollcounter++;
        if(pollcounter >= MAX_DEVICES);
        {
            polling =0;
            running = 1;
        }

    }
    else
    {
        TIMER1_CTL_R  &= ~TIMER_CTL_TAEN;
        TIMER1_ICR_R   = TIMER_ICR_TATOCINT;
   }

}
void uart1ISR()
{

    // WR_WRAPPED_AROUND1 MEANS WR AND RD INDEX IS less than RD-> EMPTY
    // TO CLEAR WR_WRAPPED_AROUND1  RD_IDX1 needs to wrapp around
    if((RD_IDX1 < (maxDevices +1)) && (RD_IDX1 < MAX_DEVICES)) // CHECK IF TX BUFFER IS EMPTY
    {
        UART1_ICR_R |= UART_ICR_TXIC;     // clears TX Interrupt
        if(polling)
        {
            if(pollcounter == RD_IDX1)
            {
                DMX_TXBUFFER[RD_IDX1] = 1;
                DMX_TXBUFFER[RD_IDX1-1] = 0;
            }
        }
        UART1_DR_R = DMX_TXBUFFER[RD_IDX1];
        RD_IDX1++;
    }
    else
    {
        UART1_ICR_R  |= UART_ICR_TXIC;        // clears TX Interrupt
        UART1_CTL_R  &= ~UART_CTL_UARTEN;     // TURN OFF UART1
        UART1_IM_R   &= ~UART_IM_TXIM;        // DISABLE TXFE-> EOT INTERRUPT.
        phase = 5;


        if(polling)
        {
            phase = 7;
            RD_IDX1 = 1;
            DMX_TXBUFFER[0] = 0xF7;

            setTimer1(318);
            setPinValue(DE,false);
            turnONTimer1();

        }
        if(running)
        {
          phase = 1;
          RD_IDX1 = 1;
          setUart1_toGPIO();
          setTimer1(7000);
          transmitBREAK();
          turnONTimer1();
        }

    }
}
void primePumpUart1()
{

    if((UART1_FR_R & UART_FR_TXFE))
    {
        UART1_CTL_R  &= ~UART_CTL_UARTEN;   // TURN OFF UART1
        UART1_IM_R   |= UART_IM_TXIM;       // ENABLE TXFE-> EOT INTERRUPT.
        UART1_CTL_R  |= UART_CTL_UARTEN;    // TURN ON UART1
        UART1_DR_R    = DMX_TXBUFFER[0];   // PRIME CHAR
        phase=4;
    }
}
void loadDataPackets()
{
    uint16_t i =0;
    DMX_TXBUFFER[0] =0x0;
    DMX_TXBUFFER[1] =0xff;
    DMX_TXBUFFER[2] =0xff;
    for(i =3; i< MAX_DEVICES; i++ )
    {
        DMX_TXBUFFER[i] = 0xff;
    }
}
void clearAllValues()
{
    uint16_t i =0;
    DMX_TXBUFFER[0] =0x0;
    DMX_TXBUFFER[1] =0x0;
    DMX_TXBUFFER[2] =0x0;
    for(i =3; i< MAX_DEVICES; i++ )
    {
        DMX_TXBUFFER[i] = 0x0;
    }
}


void startPOLL()
{
    pollcounter = 0;
    phase =0;
    DMX_TXBUFFER[0] =0xF7;
    uint16_t i =0;
    for(i =1; i< MAX_DEVICES; i++ )
    {
        DMX_TXBUFFER[i] = 0;
    }
    transmitPackets();
    setPinValue(RED_LED, true);

}
void devicesFound()
{
    char str[100];
    uint16_t i =0;
    for (i =0; i < MAX_DEVICES; i++)
    {
        if(devices_BUS[i])
        {
            sprintf(str,"Device at Addres %d\n\r>",i);
            displayUart0(str);

        }
    }
}
void rtcISR()
{
    setPinValue(BLUE_LED,true);
}
void setAddressValue(uint16_t addr, uint16_t value)
{

    DMX_TXBUFFER[addr] = value;

}
uint16_t getAddressValue(uint16_t addr)
{
    return DMX_TXBUFFER[addr];
}
void startTX()
{
    phase =0;
    running = 1;
    transmitPackets();
    setPinValue(RED_LED, true);
}
void stopTX()
{
    phase =5;
    running = 0;
    setPinValue(RED_LED, false);
}

void transmitPackets()
{
    if(phase == 0)
    {
        phase = 1;
        setUart1_toGPIO();
        setTimer1(7000);
        transmitBREAK();
        turnONTimer1();
    }
}
void updateMaxDevices(uint16_t max)
{
    maxDevices = max;
}
// current mode
uint32_t getOperationMode()
{
    return mode;
}
// power setting
void setOperationMode()
{
    mode =  readEeprom(0);
    // device needs to get address
    if(mode == 0xFFFFFFFF)
    {
        address = readEeprom(1);
        if( address == 0xFFFFFFFF)
        {
            address = 1;
        }
        else
        {
            address =address & 0x000001FF;
        }
    }
}
// next boot mode
void setControllerMode()
{
   writeEeprom(0,0xAAAAAAAA);
}
void setDeviceMode()
{
   writeEeprom(0,0xFFFFFFFF);
}
void setAddress(uint32_t value)
{
    if(value < 513)
    {
        address = value;
        value = value & 0x000001FF;
        writeEeprom(1,value);
    }

}
uint32_t getAddress()
{
    return address;
}
