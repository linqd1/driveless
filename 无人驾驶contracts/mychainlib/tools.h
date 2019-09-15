

#pragma once
#include <string>
#include "types.h"

namespace mychain {

inline bytes String2Bytes(const char* str) {
    return bytes(str, str + strlen(str));
}
std::string Bin2Hex(const bytes& input, bool upppercase = false);
bytes Hex2Bin(const std::string& input);

}  // namespace mychain
