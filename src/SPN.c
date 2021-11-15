#include <stdint.h>

#include <SPN.h>

float getValue(SPN spn, uint64_t rawData)
{
    float Data = (rawData << spn.Position) >> (64 - spn.Length);
    for(uint8_t i = 0; i < spn.OperationsSize; i++)
    {
        switch((*(uint32_t*)spn.Operations + i) & 0b11)
        {
            case SUM:
                Data += spn.Operations[i];
                break;
            case SUB:
                Data -= spn.Operations[i];
                break;
            case MUL:
                Data *= spn.Operations[i];
                break;
            case DIV:
                Data /= spn.Operations[i];
                break;
        }
    }
    return Data;
}
