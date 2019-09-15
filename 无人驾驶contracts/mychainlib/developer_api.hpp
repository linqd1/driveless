

#pragma once
#include <string>
#include "datastream.hpp"
#include "identity.hpp"
#include "types.h"

namespace mychain {

#ifdef __cplusplus
extern "C" {
#endif

int print(const char* format, ...);

namespace detail {
int CallContract(const uint8_t* id_data,
                 uint32_t id_len,
                 const uint8_t* method_data,
                 uint32_t method_len,
                 uint64_t value,
                 uint64_t gas,
                 const uint8_t* params_data,
                 uint32_t params_len);
}  // namespace detail

#ifdef __cplusplus
}
#endif

bool CheckAccount(const Identity& id);

uint64_t GetBlockNumber();

int GetBlockHash(uint64_t block_number, bytes& block_hash);

uint64_t GetBlockTimeStamp();

Identity GetOrigin();

int GetAuthMap(const Identity& id, std::map<bytes, uint32_t>& auth);

int GetBalance(const Identity& id, uint64_t& value);

int GetCode(const Identity& id, bytes& code);

int GetCodeHash(const Identity& id, bytes& hash);

int GetRecoverKey(const Identity& id, bytes& recover_key);

int GetAccountStatus(const Identity& id, uint32_t& status);

bytes GetTxHash();

uint64_t GetGas();

uint64_t GetValue();

bytes GetData();

int TransferBalance(const Identity& to, uint64_t balance);

int Revert(const bytes& exception);

int Result(const bytes& output);

int Log(const bytes& data, const std::vector<bytes>& topics);

template <typename... Args>
inline int CallContract(const Identity& contract_id,
                        const std::string& method,
                        uint64_t value,
                        uint64_t gas,
                        Args... args) {
    auto params = pack(std::tuple<std::decay_t<Args>...>(args...));
    return detail::CallContract(
        contract_id.get_data().data(), contract_id.get_data().size(),
        reinterpret_cast<const uint8_t*>(method.data()), method.size(), value, gas,
        params.data(), params.size());
}

Identity GetSelf();

Identity GetSender();

bool VerifyCommitment(uint32_t type,
                      uint32_t index,
                      const Identity& to,
                      const bytes& value_enc);

bool VerifyRange(uint32_t index, uint64_t min_value);

bool VerifyBalance(uint32_t index);

bytes Sha256(const bytes& data);

bool VerifyRsa2048(const bytes& pk, const bytes& sig, const bytes& msg);

void MyAssert(int expression, const char* msg = "");

}  // namespace mychain
