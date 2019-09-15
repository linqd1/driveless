
#pragma once

#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/stringize.hpp>
#include "types.h"

namespace mychain {
int SetStorage(const bytes& key, const bytes& value);

int GetStorage(const bytes& key, bytes& value);

int DeleteStorage(const bytes& key);

template <typename T>
int SetStorage(const bytes& key, const T& value) {
    int ret = SetStorage(key, pack(value));
    return ret == 0 ? 0 : 1;
}

template <typename T>
int GetStorage(const bytes& key, T& value) {
    bytes value_bytes;
    int ret = GetStorage(key, value_bytes);
    if (0 == ret) {
        value = unpack<T>(value_bytes);
    }
    return ret == 0 ? 0 : 1;
}

#define NOT_DAMP 0

#define GET_ELEM_KEY(elem)                                 \
    const char* pname = BOOST_PP_STRINGIZE(elem);          \
    bytes key = {NOT_DAMP};                                \
    size_t elem_size = strlen(pname);                      \
    MyAssert(elem_size <= 255, "member elem is too long"); \
    key.push_back(elem_size);                              \
    key.insert(key.end(), pname, pname + elem_size);

#define GET_STORAGE_MEMBER(r, Class, elem) \
    {                                      \
        elem = decltype(elem)();           \
        GET_ELEM_KEY(elem)                 \
        decltype(elem) value;              \
        if (0 == GetStorage(key, value)) { \
            elem = value;                  \
        }                                  \
    }

#define SET_STORAGE_MEMBER(r, Class, elem) \
    {                                      \
        GET_ELEM_KEY(elem)                 \
        SetStorage(key, elem);             \
    }

#define STORAGE(TYPE, MEMBERS)                                    \
public:                                                           \
    void AntChainLoadValue() override {                           \
        BOOST_PP_SEQ_FOR_EACH(GET_STORAGE_MEMBER, TYPE, MEMBERS); \
    }                                                             \
    void AntChainSaveValue() override {                           \
        BOOST_PP_SEQ_FOR_EACH(SET_STORAGE_MEMBER, TYPE, MEMBERS); \
    }                                                             \
    TYPE() { AntChainLoadValue(); }                               \
    virtual ~TYPE() { AntChainSaveValue(); }
};  // namespace mychain
