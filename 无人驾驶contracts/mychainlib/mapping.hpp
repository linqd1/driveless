

#pragma once

#include "storage.hpp"
#include "tools.h"

namespace mychain {
#define MAPPING_TAG 0x01

#define MAPPING(K, V, M) DMap<K, V> M{#M};

template <typename KeyType, typename ValueType>
class DMap {
public:
    // initialize mapping
    DMap(const char* prefix) {
        prefix_.push_back(MAPPING_TAG);
        size_t name_len = strlen(prefix);
        MyAssert(name_len <= 255, "DMap key too long, should be less than 255");
        prefix_.push_back(name_len);
        prefix_.insert(prefix_.end(), prefix, prefix + name_len);
    }

    virtual ~DMap() = default;

    bool get(const KeyType& key_literal, ValueType& val) {
        GenerateKey(key_literal);

        // We don't have to pack val here.
        return GetStorage(key_, val) == 0;
    }

    void set(const KeyType& key_literal, const ValueType& val) {
        GenerateKey(key_literal);

        // disgard return value due to we had defined SetStorage should revert when
        // failure was met, it won't return back to contract user.
        SetStorage(key_, val);
    }

    void del(const KeyType& key_literal) {
        GenerateKey(key_literal);

        DeleteStorage(key_);
    }

protected:
    inline void GenerateKey(const KeyType& key_literal) {
        key_.clear();
        // We leave serialization part to decide whether the key/value's too long.

        // KeyType should be able to pack into bytes, which was restricted to
        // the types of predefined supporting datastream::operator<<() and
        // datastream::operator>>()
        key_ = pack<KeyType>(key_literal);
        key_.insert(key_.begin(), prefix_.begin(), prefix_.end());

        // Finally, when we wrote
        // MAPPING(int, std::string, books)
        // books. get("c++ primer", val)
        // we generate key like 0x01 + "5books" + "c++ primer"
        // TLV designed to prevent clash.
    }
    bytes prefix_;
    bytes key_;
};

}  // namespace mychain
