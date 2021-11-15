#pragma warning(disable : 4996)
#include <SPN.h>
#include <ReadSPN.hpp>
#include <ReadLogFormat.h>

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <fstream>
#include <cstddef>
#include <string>
#include <list>

#include <cstdio>

#define SHOW_PROGRESS

#define LOADING_EMPTY 176
#define LOADING_READY 219

#define LOG_FORMAT_FLAG "-F"
#define SPN_FORMAT_FLAG "-S"
#define LOG_FLAG "-L"
#define LEADING_ID "-I"

#define HELP_FLAG "-help"
#define H_FLAG "-h"

typedef void (__fastcall *StringElementCallback) (char**, uint8_t&);

bool isIncluded(const char* one, const char* two, size_t size);
#ifdef SHOW_PROGRESS
void ConsoleThread(uint32_t& StringCounter, uint32_t& StringAmount);
#endif // SHOW_PROGRESS
void FileThread(uint32_t& StringCounter, uint32_t& StringAmount);

void __fastcall IDcallback(char** ID, uint8_t& pos);
void __fastcall DLCcallback(char** DLC, uint8_t& pos);
void __fastcall DataCallback(char** Data, uint8_t& pos);

static std::map<uint32_t, std::vector<SPN>*> SPNs;
static logFormat Format;
static std::ifstream Log;
static FILE* Output;

std::mutex CANdataMutex;
std::condition_variable BufferFull;
static std::list<std::map<char*, float>> CANrawDataList;
static uint32_t currentID;
uint8_t currentDLC;

uint32_t LeadingID;
std::map<char*, float> CalculatedData;

uint32_t StringCounter = 1;
uint32_t StringAmount = 0;
int main(int argc, char* argv[])
{
    Output = fopen("Log.txt", "a");
    for (int i = 0; i < argc; i++)
    {
        if (isIncluded(argv[i], HELP_FLAG, sizeof(HELP_FLAG) - 1) ||
            isIncluded(argv[i], H_FLAG, sizeof(H_FLAG) - 1))
        {
            printf("Flags\n\t%s<path to format cfg file>\n\t%s<path to SPN settings cfg file>\n\t%s<path to text file with data from CAN bus>\n",
                LOG_FORMAT_FLAG, SPN_FORMAT_FLAG, LOG_FLAG);
            printf("\nUse %s<HEX ID> to specify syncronization value\n", LEADING_ID);
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

        if (isIncluded(argv[i], LEADING_ID, sizeof(LEADING_ID) - 1))
            LeadingID = std::stoi(argv[i] + sizeof(LEADING_ID) - 1, nullptr, 16);
    }
    for (auto spn : SPNs)
    {
        for (auto vec : *SPNs[spn.first])
        {
            fprintf(Output, "%s\t", vec.Name);
            CalculatedData.insert({ vec.Name, 0 });
        }
    }
    fprintf(Output, "\n");

    StringElementCallback* Callbacks = new StringElementCallback[0xFF]{nullptr};
    Callbacks[Format.ID] = IDcallback;
    Callbacks[Format.DLC] = DLCcallback;
    Callbacks[Format.Data] = DataCallback;
    
    char* buffer = new char[0xFF];
    char* bufferBeginning = buffer;
    char* prevPtr = buffer;

    uint8_t currentElement = 0;

    while (Log.getline(bufferBeginning, 0xFF, '\n'))
    {
        StringAmount++;
    }

#ifdef SHOW_PROGRESS
    std::thread ConsoleProgressThread(ConsoleThread, std::ref(StringCounter), std::ref(StringAmount));
#endif // SHOW_PROGRESS
    std::thread FileWriteThread(FileThread, std::ref(StringCounter), std::ref(StringAmount));

    Log.clear(); 
    Log.seekg(std::ios_base::_Seekbeg);

    clock_t start, stop; 
    start = clock();

    while (Log.getline(bufferBeginning, 0xFF, '\n'))
    {
        prevPtr = buffer;
        currentElement = 0;
        StringCounter++;
        while (*buffer++)
        {
            if (*buffer == (char)Format.Divider)
            {
                currentElement++;
                while (*buffer == (char)Format.Divider && *buffer)
                    buffer++;
                if (Callbacks[currentElement])
                    Callbacks[currentElement](&buffer, currentElement);
            }
            while (*buffer == (char)Format.Divider && *buffer)
                buffer++;
        }
        buffer = bufferBeginning;
    }

#ifdef SHOW_PROGRESS
    ConsoleProgressThread.join();
#endif // SHOW_PROGRESS
    BufferFull.notify_all();
    std::unique_lock<std::mutex> CANlock(CANdataMutex);
    BufferFull.wait(CANlock, [] { return CANrawDataList.size() == 0; });
    FileWriteThread.join();
    fclose(Output);

    stop = clock();
    printf("Time - %ldms\n", stop - start);

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

static size_t IDlen;
void __fastcall IDcallback(char** ID, uint8_t& pos)
{
    if(std::isalpha(**ID))
        return;
    currentID = std::stoi(*ID, &IDlen, 16);
    
    *ID += --IDlen;

    if (SPNs.find(currentID) == SPNs.end())
        **ID = 0;
}

void __fastcall DLCcallback(char** DLC, uint8_t& pos)
{
    if (std::isalpha(**DLC))
    {
        currentDLC = 0;
        return;
    }
    currentDLC = **DLC - '0';
}

static char SymbArray[16]{0};
static char* SymbPtr;
static char OutputFileData[200];
void __fastcall DataCallback(char** Data, uint8_t& pos)
{
    SymbPtr = SymbArray;
    if (!currentDLC)
        return;
    while (currentDLC)
    {
        if (**Data == Format.Divider)
            currentDLC--;
        while (**Data == Format.Divider)
            (*Data)++;
        *SymbPtr++ = *(*Data)++;
    }

    for (auto spn : *SPNs[currentID])
        CalculatedData[spn.Name] = getValue(spn, strtoull(SymbArray, nullptr, 16));
    
    if (currentID == LeadingID)
    {
        CANrawDataList.push_back(CalculatedData);
    }
    if (!(StringCounter % 150000))
        BufferFull.notify_all();
}

uint32_t WrittenData;
void FileThread(uint32_t& StringCounter, uint32_t& StringAmount)
{
    while (StringCounter - 1 != StringAmount)
    {
        std::unique_lock<std::mutex> CANlock(CANdataMutex);
        BufferFull.wait(CANlock, [] { return CANrawDataList.size() != 0; });
        for (auto map : CANrawDataList)
        {
            for(auto e : map)
                fprintf(Output, "%.3f\t", e.second);
            fprintf(Output, "\n");
        }
        CANrawDataList.clear();
    }
}