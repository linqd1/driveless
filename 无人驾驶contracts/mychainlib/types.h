
#pragma once
#include <vector>
#include "serialize.hpp"

namespace mychain {

#define KEY_LIMIT 1024 * 1024
#define STORAGE_LIMIT 1024 * 1024

enum ACCOUNT_STATUS : uint32_t { NORMAL = 0, FREEZE, RECOVERING };
enum EXECUTE_STATUS : int32_t { SUCCESS = 0, NOTFOUND = -1 };
typedef std::vector<uint8_t> bytes;

}  // namespace mychain
