#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H_


#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CHARS 80
#define MAX_FIELDS 5

// stores Command for parsing
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void parseFields(USER_DATA* data);
char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
uint32_t getFieldIntegerV2(USER_DATA* data, uint8_t fieldNumber);
int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber);
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments);
void getsUart0(USER_DATA* data);

#endif
