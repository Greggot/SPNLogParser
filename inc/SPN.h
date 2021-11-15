#pragma once
#include <stdint.h>

typedef struct 
{
    char* Name;
    uint8_t NameLength;

    uint8_t Position;
    uint8_t Length;

    float* Operations;
    uint8_t OperationsSize;
} SPN;

enum OPS
{
    SUM,
    SUB,
    MUL,
    DIV,
};

#ifdef __cplusplus
extern "C"{
#endif

float getValue(SPN spn, uint64_t rawData);

#ifdef __cplusplus
}
#endif