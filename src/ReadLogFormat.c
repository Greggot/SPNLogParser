#include <ReadLogFormat.h>

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

const char* names[5] = {"Divider", "ID", "DLC", "Data", "Time"};

uint8_t isEqual(const uint8_t* one, const uint8_t* two, uint8_t size)
{
    while(size--)
    {
        if(*one++ != *two++)
            return 0;
    }
    return 1;
}


void Copy(uint8_t* Destination, const uint8_t* Source, uint8_t size)
{
    while(size--)
    {
        *Destination++ = *Source++;
    }
}

logFormat ReadFormat(const char* ConfigFileName)
{
    uint8_t rawFormat[5] = {0};
    logFormat Format;

    FILE* SPNsettings = fopen(ConfigFileName, "r");
    size_t SPNfileSize = 0;

    fseek(SPNsettings, 0, SEEK_END);
    SPNfileSize = ftell(SPNsettings);
    fseek(SPNsettings, 0, SEEK_SET);

    char* SPNbuffer = (char*)malloc(sizeof(char) * SPNfileSize);
    char* prevPtr = SPNbuffer;

    fread(SPNbuffer, sizeof(char), SPNfileSize, SPNsettings);
    
    while(*SPNbuffer++)
    {
        if(*SPNbuffer == '=')
        {
            for(uint8_t i = 0; i < 5; i++)
            {
                if(isEqual((uint8_t*)prevPtr, (uint8_t*)names[i], (uint32_t)SPNbuffer - (uint32_t)prevPtr))
                    rawFormat[i] = atoi(SPNbuffer + 1);
            }
        }
        if(*SPNbuffer == '\n')
            prevPtr = SPNbuffer + 1;
    }

    Copy((uint8_t*)&Format, rawFormat, sizeof(Format));
    printf("%d-%d-%d-%d-%d", Format.Divider, Format.ID, Format.DLC, Format.Data, Format.Time);
    return Format;
}
