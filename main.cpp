#include <SPN.h>
#include <ReadSPN.hpp>
#include <ReadLogFormat.h>

#include <thread>
#include <chrono>

#include <fstream>
#include <cstddef>
#include <string>

#define SHOW_PROGRESS

#define LOADING_EMPTY 176
#define LOADING_READY 219

#define LOG_FORMAT_FLAG "-F"
#define SPN_FORMAT_FLAG "-S"
#define LOG_FLAG "-L"

#define HELP_FLAG "-help"
#define H_FLAG "-h"

typedef void (*StringElementCallback) (const char*, std::ptrdiff_t);

bool isIncluded(const char* one, const char* two, size_t size);
#ifdef SHOW_PROGRESS
void ConsoleThread(uint32_t& StringCounter, uint32_t& StringAmount);
#endif // SHOW_PROGRESS

void IDcallback(const char* ID, std::ptrdiff_t IDstringSize);
void DLCcallback(const char* DLC, std::ptrdiff_t IDstringSize);
void DataCallback(const char* Data, std::ptrdiff_t DataStringSize);

static std::map<uint32_t, std::vector<SPN>*> SPNs;
static logFormat Format;
static std::ifstream Log;

static std::vector<std::pair<uint32_t, uint64_t>> CANrawDataVec;
std::pair<uint32_t, uint64_t> CANrawData;
uint8_t currentDLC;

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (isIncluded(argv[i], HELP_FLAG, sizeof(HELP_FLAG) - 1) ||
            isIncluded(argv[i], H_FLAG, sizeof(H_FLAG) - 1))
        {
            printf("Flags\n\t%s<path to format cfg file>\n\t%s<path to SPN settings cfg file>\n\t%s<path to text file with data from CAN bus>\n",
                LOG_FORMAT_FLAG, SPN_FORMAT_FLAG, LOG_FLAG);
            return 0;
        }
    }

    for (int i = 0; i < argc; i++)
    {
        if (isIncluded(argv[i], LOG_FORMAT_FLAG, sizeof(LOG_FORMAT_FLAG) - 1))
            Format = ReadFormat(argv[i] + sizeof(LOG_FORMAT_FLAG) - 1);

        if (isIncluded(argv[i], SPN_FORMAT_FLAG, sizeof(SPN_FORMAT_FLAG) - 1))
            SPNs = ReadSPN(argv[i] + sizeof(SPN_FORMAT_FLAG) - 1);

        if (isIncluded(argv[i], LOG_FLAG, sizeof(SPN_FORMAT_FLAG) - 1))
            Log.open(argv[i] + sizeof(SPN_FORMAT_FLAG) - 1);
    }

    StringElementCallback* Callbacks = new StringElementCallback[0xFF]{nullptr};
    Callbacks[Format.ID] = IDcallback;
    Callbacks[Format.DLC] = DLCcallback;
    Callbacks[Format.Data] = DataCallback;
    
    char* buffer = new char[0xFF];
    char* bufferBeginning = buffer;
    char* prevPtr = buffer;

    volatile uint8_t currentElement = 0;

    uint32_t StringCounter = 1;
    uint32_t StringAmount = 0;

    while (Log.getline(bufferBeginning, 0xFF, '\n'))
    {
        StringAmount++;
    }

#ifdef SHOW_PROGRESS
    std::thread ConsoleProgressThread(ConsoleThread, std::ref(StringCounter), std::ref(StringAmount));
#endif // SHOW_PROGRESS
    Log.clear();
    Log.seekg(std::ios_base::_Seekbeg);


    while (Log.getline(bufferBeginning, 0xFF, '\n'))
    {
        prevPtr = buffer;
        currentElement = 0;
        StringCounter++;
        while (*buffer++)
        {
            if (*buffer == (char)Format.Divider)
            {
                while (*prevPtr == (char)Format.Divider && *prevPtr)
                    prevPtr++;
                if (Callbacks[currentElement])
                    Callbacks[currentElement](prevPtr, buffer - prevPtr);

                prevPtr = buffer;
                currentElement++;
            }
            while (*buffer == (char)Format.Divider && *buffer)
                buffer++;
        }
        buffer = bufferBeginning;
    }

#ifdef SHOW_PROGRESS
    ConsoleProgressThread.join();
#endif // SHOW_PROGRESS

}

bool isIncluded(const char* one, const char* two, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (one[i] != two[i])
            return false;
    }
    return true;
}

#ifdef SHOW_PROGRESS
void ConsoleThread(uint32_t& StringCounter, uint32_t& StringAmount)
{
    uint32_t percent = 0;
    while (percent != 100)
    {
        percent = 100 * StringCounter / StringAmount;
        printf("%d percent done: ", percent);
        for (uint8_t i = 0; i < percent; i += 4)
            printf("%c", LOADING_READY);
        for (uint8_t i = percent; i < 100; i += 4)
            printf("%c", LOADING_EMPTY);
        printf("\r");

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    printf("\n");
    printf("%d strings has been parsed\n", StringCounter);
}
#endif

void IDcallback(const char* ID, std::ptrdiff_t IDstringSize)
{
    if(std::isalpha(*ID))
        return;
    CANrawData.first = std::stoi(ID, nullptr, 16);
}

void DLCcallback(const char* DLC, std::ptrdiff_t IDstringSize)
{
    if (std::isalpha(*DLC))
    {
        currentDLC = 0;
        return;
    }
    currentDLC = *DLC - '0';
}

static char SymbArray[16];
static char* SymbPtr;
void DataCallback(const char* Data, std::ptrdiff_t DataStringSize)
{
    SymbPtr = SymbArray;
    if (!currentDLC)
        return;
    while (currentDLC)
    {
        if (*Data == Format.Divider)
            currentDLC--;
        while (*Data == Format.Divider)
            Data++;
        *SymbPtr++ = *Data++;
    }
    try
    {
        CANrawData.second = std::stoull(SymbArray, nullptr, 16);
    }
    catch (std::out_of_range)
    {
        CANrawData.second = 0;
    }
}