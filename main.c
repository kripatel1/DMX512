/**
 * NAME: kripatel1
 *
 */

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//  Configured to 115,200 baud, 8N1


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "uart0.h"
#include "rtc.h"
#include "wait.h"

#include "eeprom.h"
#include "user_interface.h"
#include "uart0_tx.h"
#include "uart1_tx.h"
#include "timer1.h"
#include "DMX.h"


#define RED_LED     PORTF,1
#define BLUE_LED    PORTF,2
#define GREEN_LED   PORTF,3
#define PUSH_BUTTON PORTF,4

#define DIGITAL_ENABLE PORTC,7



void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, sysdivider of 5, creating system clock of 40 MHz
    // Reg 8 pg 254
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S); // << 23
    //             0x0540-> 0x15[pg 254]| 0x0                     | 0x00400000[pg 254 bit 22] | [bits 26-23]  0x0200 0000   |

    // Enable clocks
    enablePort(PORTF);


    // Configure LED and pushbutton pins
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(BLUE_LED);
    selectPinDigitalInput(PUSH_BUTTON);
    enablePinPullup(PUSH_BUTTON);


}



int main(void)
{
    char str[200];
    USER_DATA info;
    uint8_t hour;
    uint8_t minute;
    uint8_t seconds;
    uint16_t addr;
    uint16_t data;
    int32_t value;

    uint16_t tempData;
    uint16_t tempData2;

    initHw();
    initUart0_Interrupts_ENABLED();
    initUart1_Interrupts_ENABLED();
    initTimer1HW();
    initRTC_Module();
    initEEPROM_Module();

    // Setup UART0 and UART1 baud rate
    setUart0BaudRate(115200, 40e6);

    setControllerMode();

    // READ OPERATION MODE IN EEPROM AND ADDRESS
    setOperationMode();
    loadDataPackets();

    // Display greeting
    if(getOperationMode() == -1)
    {
        sprintf(str,"MODE = DEVICE|| ADDR %d\n\r", getAddress());
        putsUart0(str);
    }
    else
    {
        sprintf(str,"MODE = CONTROLLER \n\r");
        startTX();
        putsUart0(str);
    }


    putsUart0("Serial Example\r\n");
    putsUart0("Press '0' or '1'\r\n");
    putcUart0('>');
    while(1)
    {
         bool valid = false;
         if(kbhitUart0())
         {

            getsUart0(&info);
//            putsUart0(info.buffer);
//            putsUart0("\n\r>");
            parseFields(&info);

            if((isCommand(&info, "clear", 0)))
            {
                valid = true;
                clearAllValues();
                sprintf(str,"clear command was requested\n\r>");
                displayUart0(str);
            }
            else if (isCommand(&info, "set", 2))
            {
                addr = getFieldIntegerV2(&info, 1);
                data = getFieldIntegerV2(&info, 2);
                valid = true;
                //setAddress(set,V)
                sprintf(str,"set command was requested\n\r address %d with %d value update\n\r",addr,data);
                displayUart0(str);
                setAddressValue(addr,data);
            }
            //***********************************/
            else if (isCommand(&info, "get", 1))
            {
                addr = getFieldIntegerV2(&info, 1);
                valid = true;

                data = getAddressValue(addr);
                sprintf(str,"get command was requested\n\r>");
                displayUart0(str);
                sprintf(str,"Address: %d\n\r Value: %d\n\r>",addr,data);
                displayUart0(str);
            }
            //***********************************/
            else if (isCommand(&info, "on", 0))
            {
                valid = true;
                startTX();
                sprintf(str,"512 Packet Data transmitting\n \r");
                displayUart0(str);

            }
            //***********************************/
            else if (isCommand(&info, "off", 0))
            {
                valid = true;
                stopTX();
                sprintf(str,"OFF command was requested\n\r>");
                displayUart0(str);

            }
            //***********************************/
            else if (isCommand(&info, "max", 1))
            {
                // implement this function//
                valid = true;
                tempData = getFieldIntegerV2(&info,1);
                updateMaxDevices(tempData);

                sprintf(str,"MAX VALUE %d was set\n\r>",tempData);
                displayUart0(str);
            }
            //***********************************/
            else if (isCommand(&info, "poll", 0))
            {
                // implement this function//
                valid = true;
                sprintf(str,"poll command was requested\n\r>",tempData);
                displayUart0(str);
            }
            else if (isCommand(&info, "polled",0))
            {
                valid = true;
                sprintf(str,"polled command was requested\n\r>",tempData);
                displayUart0(str);
                devicesFound();
            }
            else if(isCommand(&info,"time",3))
            {

                hour = getFieldIntegerV2(&info,1);
                minute =  getFieldIntegerV2(&info,2);
                seconds =  getFieldIntegerV2(&info,3);

                set_time(hour, minute, seconds);
                sprintf(str,"Value you entered was H=%d M=%d S=%d\n\r>",hour,minute,seconds);
                putsUart0(str);
                valid = true;

            }
            else if (isCommand(&info,"time",0))
            {

                get_time(&hour,&minute,&seconds);
                sprintf(str, ">Current time is %d : %2d : %d\n\r", hour,minute,seconds);
                displayUart0(str);
                valid = true;
            }
            else if (isCommand(&info, "date", 2))
            {
                // implement this function//
                hour = getFieldIntegerV2(&info,1);
                minute =  getFieldIntegerV2(&info,2);
                valid = true;
                putsUart0("date command was SET\n\r>");
            }
            else if (isCommand(&info, "date", 0))
            {
                // implement this function//
                valid = true;
//                sprintf(str, ">Current date is %d : %2d \n\r", 10,11);
                putsUart0("date");
            }
            else if (isCommand(&info, "setat", 3))
            {
                hour = getFieldIntegerV2(&info,1);
                minute =  getFieldIntegerV2(&info,2);
                seconds =  getFieldIntegerV2(&info,3);

                set_time(hour, minute, seconds);
                sprintf(str,"Value you entered was H=%d M=%d S=%d\n\r>",hour,minute,seconds);
                putsUart0(str);
                valid = true;

            }
            else
            {
                putsUart0("INVALID COMMAND\n\r");
            }

         }
     }
	return 0;
}
