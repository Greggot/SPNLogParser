#pragma once

#include <stdint.h>

typedef struct 
{
    char Divider;
    uint8_t ID;
    uint8_t DLC;
    uint8_t Data;
    uint8_t Time;
} logFormat;

#ifdef __cplusplus
extern "C"{
#endif

logFormat ReadFormat(const char* ConfigFileName);

#ifdef __cplusplus
}
#endif