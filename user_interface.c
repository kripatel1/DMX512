
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "user_interface.h"



void parseFields(USER_DATA* data)
{
    int count =0;
    data->fieldCount =0;
    int i =0;
    int startedField = 0;

    while(1)
    {
        if(i > MAX_CHARS){return;}
        char currentChar = data->buffer[i];
        //processing field
        if(!startedField)
        {   // recording the startIDX of the field
            // recording the type
            if( ((currentChar >= 'A') && (currentChar <='Z')) ||
                ((currentChar >= 'a') && (currentChar <='z')))
            {
                data->fieldPosition[count] = i;
                data->fieldType[count] = 'A';
                data->fieldCount++;
                startedField = 1;
            }
            else if ((( currentChar >= '0') && (currentChar <= '9')) ||
                    (currentChar == '.') || (currentChar == '-') || (currentChar == ','))
            {
                data->fieldPosition[count] = i;
                data->fieldType[count] = 'N';
                data->fieldCount++;
                startedField = 1;
            }
            else
            {
                data->buffer[i]= NULL; // placing Null everywhere else
            }
        }

        else
        {
            if( data->fieldType[count] == 'A')
            {
                if( ((currentChar >= 'A') && (currentChar <='Z')) ||
                    ((currentChar >= 'a') && (currentChar <='z')))
                {
                    // verify it is the same type
                }
                else
                {
                    startedField =0; // parsing has ended because types changed
                    count++;

                    data->buffer[i]= NULL;
                    if(count == MAX_FIELDS)
                    {
                        return;
                    }
                }
            }
            else if(data->fieldType[count] == 'N')
            {
                if ((currentChar >= '0') && (currentChar <= '9') ||
                    (currentChar == '.') || (currentChar == '-') || (currentChar == ','))
                {
                    // verify it is the same type
                }
                else
                {
                    startedField =0; // parsing has ended because types changed
                    count++;
                    data->buffer[i]= NULL;
                    if(count == MAX_FIELDS)
                    {
                        return;
                    }
                }
            }
        }
        i++;
    }
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    // since idxs are stored in increasing order check against
    // last stored idx for OUT OF RANGE condition

    int i =0;
    int startIdx;
    int size =0;
    // check fieldNumber against the last index
    if(fieldNumber > data->fieldPosition[(data->fieldCount) -1 ] )
    {
        return NULL; // field Number is out of range;
    }
   i =0;
    while(i< (data->fieldCount) )
    {
        if( data->fieldPosition[i] == fieldNumber)
        {
            break;
        }
        i++;
    }
    // position 5 does not exist so the fieldNumber does not exist
    if (i == data->fieldCount)
    {
        return NULL;
    }
    startIdx =data->fieldPosition[i];
    size =0;


    // determining the size for allocating the field
    while(data->buffer[startIdx] != 0)
    {
        size++;
        startIdx++;
    }

    startIdx =data->fieldPosition[i];

    i =0;
    char *ptr = &(data->buffer[startIdx]);
    return ptr;
}
uint32_t getFieldIntegerV2(USER_DATA* data, uint8_t fieldNumber)
{
    int startIdx;
    int value = 0;
    // check fieldNumber against the last index
    if(fieldNumber > (data->fieldCount - 1)   )
    {
        return 0; // field Number is out of range;
    }
    // Verifies the type at that field
    if( data->fieldType[fieldNumber] != 'N')
    {
        return 0; // Not a number
    }

    startIdx = data->fieldPosition[fieldNumber];
    int multipler  = 1;
    int endIdx = startIdx;
    // to convert properly- need to find ones place and go from there
    while(data->buffer[endIdx] != 0)
    {
        endIdx++;
    }
    endIdx--;
    while(endIdx != (startIdx-1))
   {
       value = value + ((data->buffer[endIdx] - '0') * multipler);
       multipler = multipler *10;
       endIdx--;
   }
    return value;
}
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
// minArguments does not count the CMD as an argument
   if(((data->fieldCount) -1) < minArguments)
   {
       return false;
   }

    int i =0;
    char tempData[MAX_CHARS];

    while(strCommand[i] != 0)
    {
        tempData[i] = strCommand[i];
        i++;
    }
    while(i< MAX_CHARS)
    {
        tempData[i] = 0;
        i++;
    }
    tempData[i] = 0;


    // field Position at idx 0 contains the cmd
    // due to parsing; it does not always start at INDEX 0 of the BUFFER
    // so we use fieldPosition
    int startIdx = data->fieldPosition[0];
    i =0;
    while( (data->buffer[startIdx] == strCommand[i]) && (strCommand[i] != 0))
    {
        i++;
        startIdx++;
    }
    if(strCommand[i] != 0)
    {
        return false;
    }

    // uses strcmp function // Not need though because the previous 10 lines take care of this
    // this just proves I can use strcmp
    char *storedCMD = getFieldString(data,data->fieldPosition[0]);
    if(strcmp(storedCMD,tempData) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{

    int startIdx;
    int value = 0;
    // check fieldNumber against the last index
    if(fieldNumber > data->fieldPosition[(data->fieldCount) -1 ] )
    {
        return 0; // field Number is out of range;
    }
    int i =0;
    while(i< (data->fieldCount) )
    {
        if( data->fieldPosition[i] == fieldNumber)
        {
            break;
        }
        i++;
    }
    // position 5 does not exist so the fieldNumber does not exist
    if (i == data->fieldCount)
    {
        return 0;
    }
    if( data->fieldType[i] != 'N')
    {
        return 0;
    }
    startIdx = data->fieldPosition[i];
    int multipler  = 1;
    int endIdx = startIdx;
    // to convert properly- need to find ones place and go from there
    while(data->buffer[endIdx] != 0)
    {
        endIdx++;
    }
    endIdx--;

    while(endIdx != startIdx)
    {
        value = value + ((data->buffer[endIdx] - '0') * multipler);
        multipler = multipler *10;
        endIdx--;
    }
    return value;
}
void getsUart0(USER_DATA* data)
{
    // clearing buffer
    int i = 0;
    for(i =0; i < (MAX_CHARS + 1); i++ )
    {
        data->buffer[i] = 0;
    }
    int count  =0;
    while(1)
    {
        char c = getcUart0();
        // Backspace or delete
        if((c == 8) || (c == 127))
        {
            // empty?
            if(count > 0)
            {
                count--;
            }
            else
            {
                continue;
            }
        }
        // [ENTER] key is pressed
        else if(c == 13)
        {
            data->buffer[count] = 0;
            return;
        }
        else if (c >= 32)
        {
            data->buffer[count]=c;
            count++;
            if(count == MAX_CHARS)
            {
                data->buffer[count] =0;
                return;
            }
        }
    }
}
