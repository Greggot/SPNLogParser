#include "inc/SPN.h"
#include "inc/ReadSPN.hpp"
#include "inc/ReadLogFormat.h"

#include <map>
#include <vector>

int main(int argc, char* argv[])
{
    if(argc < 3)
        return -1;
    ReadSPN(argv[1]);
    ReadFormat(argv[2]);
}