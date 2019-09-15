

#pragma once
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/mp11/tuple.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <mychainlib/types.h>

namespace mychain {

#ifdef __cplusplus
extern "C" {
#endif
extern "C" uint8_t* get_chain_data_buffer();
namespace detail {

int ReadInterfaceName(const uint8_t* name_data, uint32_t* name_len_ptr);
int ReadInterfaceParams(const uint8_t* params_data, uint32_t* params_len_ptr);

}  // namespace detail

#ifdef __cplusplus
}
#endif

template <typename T, typename... Args>
inline bool execute_interface(void (T::*func)(Args...)) {
    uint8_t* buffer = get_chain_data_buffer();
    uint32_t ps = STORAGE_LIMIT;
    detail::ReadInterfaceParams(buffer, &ps);
    std::tuple<std::decay_t<Args>...> args;
    datastream<const uint8_t*> ds(buffer, ps);
    ds >> args;

    T inst;

    auto f2 = [&](auto... a) { ((&inst)->*func)(a...); };
    boost::mp11::tuple_apply(f2, args);

    return true;
}

#define INTERFACE_EXPORT_INTERNAL(r, OP, elem)        \
    {                                                 \
        const char* pname = BOOST_PP_STRINGIZE(elem); \
        if (name == pname) {                          \
            execute_interface(&OP::elem);             \
            return EXECUTE_STATUS::SUCCESS;           \
        }                                             \
    }

#define INTERFACE_EXPORT(TYPE, MEMBERS)                                 \
    extern "C" {                                                        \
    int apply() {                                                       \
        uint8_t* buffer = get_chain_data_buffer();                      \
        uint32_t ns = STORAGE_LIMIT;                                    \
        detail::ReadInterfaceName(buffer, &ns);                         \
        std::string name(buffer, buffer + ns);                          \
        BOOST_PP_SEQ_FOR_EACH(INTERFACE_EXPORT_INTERNAL, TYPE, MEMBERS) \
        if (name == "Init") {                                           \
            Contract c;                                                 \
            c.Init();                                                   \
            return EXECUTE_STATUS::SUCCESS;                             \
        }                                                               \
        return EXECUTE_STATUS::NOTFOUND;                                \
    }                                                                   \
    }

}  // namespace mychain
