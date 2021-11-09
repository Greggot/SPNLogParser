// #include "SPN.hpp"
#include <SPN.h>
#include <ReadSPN.hpp>

#include <stdio.h>
#include <string>
#include <cstring>

#include <vector>
#include <map>


#define SkipTo(data, goalSymb, endSymb) while(*data++ != goalSymb) \
                            {                        \
                                if(*data == endSymb)\
                                {                    \
                                    break;          \
                                }                    \
                            }                        \


bool CreateSPN(const char* data, size_t size, std::map<uint32_t, std::vector<SPN>*>* SPNs)
{
    SPN *spn = new SPN;

    uint8_t NameLength = 0;
    uint8_t elementCount = 1;
    while(--size)
    {
        if(data[size] == ' ')
            elementCount++;
    }

    if(elementCount < MIN_ELEMENT_AMOUNT)
    {
        return false;
    }

    elementCount -= BASIC_ELEMENT_AMOUNT;
    spn->OperationsSize = elementCount;
    spn->Operations = new float[elementCount];

    while(data[NameLength] != ' ')
        NameLength++;
    
    spn->Name = new char[++NameLength] {0};
    memcpy(spn->Name, data, NameLength - 1);

    data += NameLength;

    uint32_t ID = std::stoi(data, 0, 16);
    SkipTo(data, ' ', '\n');

    spn->Position = std::stoi(data, 0, 10);
    SkipTo(data, ' ', '\n');

    spn->Length = std::stoi(data, 0, 10);
    SkipTo(data, ' ', '\n');

    float* operand;
    while(elementCount)
    {
        *operand = atof(++data);
        switch(*(--data))
        {
            case '+':
                *(uint32_t*)operand = *(uint32_t*)operand &~ 0b11 | SUM;
                break;
            case '*':
            case 'x':
                *(uint32_t*)operand = *(uint32_t*)operand &~ 0b11 | MUL;
                break;
            case '-':
                *(uint32_t*)operand = *(uint32_t*)operand &~ 0b11 | SUB;
                break;
            case '/':
            case '\\':
                *(uint32_t*)operand = *(uint32_t*)operand &~ 0b11 | DIV;
                break;
        }
        spn->Operations[spn->OperationsSize - elementCount] = *operand;
        elementCount--;
        SkipTo(data, ' ', '\n');
    }
    if(SPNs->find(ID) == SPNs->end())
    {
        std::vector<SPN>* vec = new std::vector<SPN>();
        vec->push_back(*spn);
        SPNs->insert({ID, vec});
    }
    else
    {
        (*SPNs)[ID]->push_back(*spn);
    }
    return true;
}

std::map<uint32_t, std::vector<SPN>*> ReadSPN(const char* ConfigFileName)
{
    std::map<uint32_t, std::vector<SPN>*> SPNs;

    FILE* SPNsettings = fopen(ConfigFileName, "r");
    size_t* SPNfileSize = new size_t;

    fseek(SPNsettings, 0, SEEK_END);
    *SPNfileSize = ftell(SPNsettings);
    fseek(SPNsettings, 0, SEEK_SET);
    
    // printf("%d - filesize\n", *SPNfileSize);

    char* SPNbuffer = new char[*SPNfileSize];

    fread(SPNbuffer, sizeof(char), *SPNfileSize, SPNsettings);
    delete SPNfileSize;

    std::string rawData = "";
    while(*SPNbuffer)
    {
        if(*SPNbuffer == ' ' || *SPNbuffer == '\t')
            rawData += ' ';
        while(*SPNbuffer == ' ' || *SPNbuffer == '\t')
            SPNbuffer++;

        rawData += *SPNbuffer;
        
        if(*SPNbuffer++ == '\n')
        {
            if(rawData[rawData.size() - 2] == ' ')
            {
                rawData[rawData.size() - 2] = '\n';
                rawData.pop_back();
            }
            CreateSPN(rawData.c_str(), rawData.size(), &SPNs);
            rawData = "";
        }
    }

    for(auto spn : SPNs)
    {
        printf("%08X: ", spn.first);
        for(auto vec : *spn.second)
        {
            printf("%s (", vec.Name);
            for(uint8_t i = 0; i < vec.OperationsSize; i++)
                printf("%.3f ", vec.Operations[i]);
            printf(") ");
        }
        
        printf("\n");
    }

    delete[] SPNbuffer;
    return SPNs;
}