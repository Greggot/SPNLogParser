#include <SPN.h>
#include <ReadSPN.hpp>
#include <ReadLogFormat.h>

#include <thread>
#include <chrono>

#include <fstream>
#include <cstddef>

#define SHOW_PROGRESS

#define LOADING_EMPTY 176
#define LOADING_READY 219

#define LOG_FORMAT_FLAG "-F"
#define SPN_FORMAT_FLAG "-S"
#define LOG_FLAG "-L"

#define HELP_FLAG "-help"
#define H_FLAG "-h"

bool isIncluded(const char* one, const char* two, size_t size);
typedef void (*StringElementCallback) (uint8_t, const char*, std::ptrdiff_t);

#ifdef SHOW_PROGRESS
void ConsoleThread(uint32_t& StringCounter, uint32_t& StringAmount);
#endif // SHOW_PROGRESS


int main(int argc, char* argv[])
{
    std::map<uint32_t, std::vector<SPN>*> SPNs;
    logFormat Format;

    for (int i = 0; i < argc; i++)
    {
        if (isIncluded(argv[i], HELP_FLAG, sizeof(HELP_FLAG) - 1) ||
            isIncluded(argv[i], H_FLAG, sizeof(H_FLAG) - 1))
        {
            printf("Help\n");
        }
    }

    std::ifstream Log;

    for (int i = 0; i < argc; i++)
    {
        if (isIncluded(argv[i], LOG_FORMAT_FLAG, sizeof(LOG_FORMAT_FLAG) - 1))
            Format = ReadFormat(argv[i] + sizeof(LOG_FORMAT_FLAG) - 1);

        if (isIncluded(argv[i], SPN_FORMAT_FLAG, sizeof(SPN_FORMAT_FLAG) - 1))
            SPNs = ReadSPN(argv[i] + sizeof(SPN_FORMAT_FLAG) - 1);

        if (isIncluded(argv[i], LOG_FLAG, sizeof(SPN_FORMAT_FLAG) - 1))
            Log.open(argv[i] + sizeof(SPN_FORMAT_FLAG) - 1);
    }

    char* buffer = new char[0xFF];
    char* bufferBeginning = buffer;

    char* prevPtr = buffer;

    uint32_t StringCounter = 1;
    uint8_t currentElement = 0;

    uint32_t StringAmount = 0;

    while (Log.getline(bufferBeginning, 0xFF, '\n'))
    {
        StringAmount++;
    }
    Log.clear();
    Log.seekg(std::ios_base::_Seekbeg);

#ifdef SHOW_PROGRESS
    std::thread ConsoleProgressThread(ConsoleThread, std::ref(StringCounter), std::ref(StringAmount));
#endif // SHOW_PROGRESS

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
        for (uint8_t i = 0; i < percent; i += 2)
            printf("%c", LOADING_READY);
        for (uint8_t i = percent; i < 100; i += 2)
            printf("%c", LOADING_EMPTY);
        printf("\r");

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    printf("\n");
    printf("%d strings has been parsed\n", StringCounter);
}
#endif