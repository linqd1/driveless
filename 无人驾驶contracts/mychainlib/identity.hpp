


#pragma once

#include <utility>
#include "serialize.hpp"
#include "tools.h"

namespace mychain {
class Identity {
public:
    Identity() : data_(kConstIdLength) {}

    Identity(const char* id_hex) {
        data_ = Hex2Bin(std::string(id_hex));
        MyAssert(data_.size() == kConstIdLength, "invalid hex format Identity");
    }

    Identity(const std::string& id_hex) {
        data_ = Hex2Bin(id_hex);
        MyAssert(data_.size() == kConstIdLength, "invalid hex format Identity");
    }

    explicit Identity(const bytes& id_bin) {
        MyAssert(id_bin.size() == kConstIdLength, "invalid bin format Identity");
        data_ = id_bin;
    }

    explicit Identity(bytes&& id_bin) {
        data_ = std::move(id_bin);
        MyAssert(data_.size() == kConstIdLength, "invalid bin format Identity");
    }

    Identity(const Identity& rl) { data_ = rl.data_; }

    Identity(Identity&& rl) { data_ = std::move(rl.data_); }

    Identity& operator=(const Identity& rl) {
        if (this != &rl) {
            data_ = rl.data_;
        }
        return *this;
    }

    Identity& operator=(Identity&& rl) {
        data_ = std::move(rl.data_);
        return *this;
    }

    Identity& operator=(const bytes& id_bin) {
        MyAssert(id_bin.size() == kConstIdLength, "invalid bin format Identity");
        data_ = id_bin;
        return *this;
    }

    Identity& operator=(bytes&& id_bin) {
        data_ = std::move(id_bin);
        MyAssert(data_.size() == kConstIdLength, "invalid bin format Identity");
        return *this;
    }

    Identity& operator=(const std::string& id_hex) {
        data_ = Hex2Bin(id_hex);
        MyAssert(data_.size() == kConstIdLength, "invalid hex format Identity");
        return *this;
    }

    inline bool operator<(const Identity& x) { return data_ < x.data_; }

    inline bool operator==(const Identity& x) { return data_ == x.data_; }

    inline bool operator!=(const Identity& x) { return data_ != x.data_; }

    inline const bytes& get_data() const { return data_; }

    inline std::string to_hex(bool upppercase = false) const {
        return Bin2Hex(data_, upppercase);
    }

private:
    bytes data_;

    static const uint32_t kConstIdLength = 32;

    SERIALIZE(Identity, (data_))
};
};  // namespace mychain
