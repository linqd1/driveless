
 

#pragma once
#include <string>
#include "datastream.hpp"
#include "identity.hpp"
#include "types.h"

#define INTERFACE [[mychain::interface]] void
namespace mychain {

#ifdef __cplusplus
extern "C" {
#endif

namespace detail {
int DelegateCall(const uint8_t* id_data,
                 uint32_t id_len,
                 const uint8_t* method_data,
                 uint32_t method_len,
                 const uint8_t* params_data,
                 uint32_t params_len);
}  // namespace detail

#ifdef __cplusplus
}
#endif

class Contract {
public:
    void Init() {}

protected:
    virtual void AntChainSaveValue() {}
    virtual void AntChainLoadValue() {}

    template <typename... Args>
    inline int DelegateCall(const Identity& contract_id,
                            const std::string& method,
                            Args... args) {
        AntChainSaveValue();
        auto params = pack(std::tuple<std::decay_t<Args>...>(args...));
        int ret = detail::DelegateCall(contract_id.get_data().data(),
                                       contract_id.get_data().size(),
                                       reinterpret_cast<const uint8_t*>(method.data()),
                                       method.size(), params.data(), params.size());
        AntChainLoadValue();
        return ret == 0 ? 0 : 1;
    }
};
}  // namespace mychain
