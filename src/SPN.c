#include <stdint.h>

#include <SPN.h>

float getValue(SPN spn, uint64_t rawData)
{
    #ifdef LITTLE_ENDIAN
        float Data = (float)((rawData << (64 - spn.Position - spn.Length)) >> (64 - spn.Length));
    #else
        float Data = (float)((rawData << spn.Position) >> (64 - spn.Length));
    #endif

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
