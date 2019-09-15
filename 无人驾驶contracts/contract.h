
#ifndef MYCHAIN_SRC_CPP_OBJ_CHAIN_CONTRACT_H_
#define MYCHAIN_SRC_CPP_OBJ_CHAIN_CONTRACT_H_

#include <string>
#include <vector>

#include "libraries/common/codec_message_dec.h"
#include "protocol/chain/state_obj.h"

namespace mychainsdk {

class Contract : public StateObject {
public:
    Contract() = default;
    Contract(const Identity& id, uint64_t balance, const AuthMap& auth_map, const bytes& code);

    Contract(const Identity& id, uint64_t balance, const AuthMap& auth_map, const bytes& code,
             const PublicKey& recover_key, uint64_t recover_time);

    Contract(const Identity& id, uint64_t balance, const AuthMap& auth_map, const bytes& code,
             const PublicKey& recover_key, uint64_t recover_time, uint32_t status, const h256& storage_root,
             const h256& code_hash);

public:
    static bool RLPDecodeContract(const bytes& data, Contract& contract);
    static bool RLPEncodeContract(const Contract& contract, bytes& data);
    static bool RLPDecodeContracts(const bytes& data, std::vector<Contract>& contracts);
    static bool RLPEncodeContracts(const std::vector<Contract>& contracts, bytes& data);
    static bool RLPContract(const Contract& contract, bytes& rlp_contract);

public:
    std::string ToString();

public:
    inline bytes GetCode() const {
        return code_;
    }

    inline h256 GetStorageRoot() const {
        return storage_root_;
    }
    inline h256 GetCodeHash() const {
        return code_hash_;
    }

public:
    h256 storage_root_;  // MPT root hash for storage, used for contract
    h256 code_hash_;     // Code hash for contract
    bytes code_;
};
}  // namespace mychainsdk

// RLP_STREAM_MESdSAGE_DERIVED(mychainsdk::Contract, mychainsdk::StateObject,
// (storage_root_)(code_hash_)(code_)(status_))

// RLPStream& operator << (RLPStream& stream, const mychainsdk::Contract& so);
// RLP& operator >> (RLP& stream, mychainsdk::Contract& so);

#endif  // MYCHAIN_SRC_CPP_OBJ_CHAIN_CONTRACT_H_
