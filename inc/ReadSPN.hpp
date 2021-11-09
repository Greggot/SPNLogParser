#pragma once

#include "SPN.h"
#include <map>
#include <vector>

#define MIN_ELEMENT_AMOUNT 5
#define BASIC_ELEMENT_AMOUNT 4

std::map<uint32_t, std::vector<SPN>*> ReadSPN(const char* ConfigFileName);