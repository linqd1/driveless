
#pragma once

#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/stringize.hpp>
#include "datastream.hpp"

namespace mychain {

#define REFLECT_MEMBER_OP(r, OP, elem) OP t.elem

#define SERIALIZE(TYPE, MEMBERS)                                         \
    template <typename D>                                                \
    friend datastream<D>& operator<<(datastream<D>& ds, const TYPE& t) { \
        return ds BOOST_PP_SEQ_FOR_EACH(REFLECT_MEMBER_OP, <<, MEMBERS); \
    }                                                                    \
    template <typename D>                                                \
    friend datastream<D>& operator>>(datastream<D>& ds, TYPE& t) {       \
        return ds BOOST_PP_SEQ_FOR_EACH(REFLECT_MEMBER_OP, >>, MEMBERS); \
    }

#define SERIALIZE_DERIVED(TYPE, BASE, MEMBERS)                           \
    template <typename D>                                                \
    friend datastream<D>& operator<<(datastream<D>& ds, const TYPE& t) { \
        ds << static_cast<const BASE&>(t);                               \
        return ds BOOST_PP_SEQ_FOR_EACH(REFLECT_MEMBER_OP, <<, MEMBERS); \
    }                                                                    \
    template <typename D>                                                \
    friend datastream<D>& operator>>(datastream<D>& ds, TYPE& t) {       \
        ds >> static_cast<BASE&>(t);                                     \
        return ds BOOST_PP_SEQ_FOR_EACH(REFLECT_MEMBER_OP, >>, MEMBERS); \
    }
}  // namespace mychain
