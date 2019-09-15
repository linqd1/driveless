

#pragma once
#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <stdio.h>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <mychainlib/varint.hpp>

#ifdef datastream_test
#define stream_assert(condition, msg) \
    if (not(condition)) {             \
        printf("%s", (msg));          \
        assert(false);                \
    }
#else
#define stream_assert(condition, msg) MyAssert((condition), (msg));
#endif

namespace mychain {
extern void MyAssert(int, const char*);

/**
 *  %A data stream for reading and writing data in the form of bytes
 *  @brief %A data stream for reading and writing data in the form of bytes.
 *  @tparam T - Type of the datastream buffer
 */
template <typename D>
class datastream {
public:
    datastream(D start, size_t s) : _start(start), _pos(start), _end(start + s) {}

    inline void skip(size_t s) { _pos += s; }

    inline bool read(char* d, size_t s) {
        stream_assert(size_t(_end - _pos) >= (size_t)s, "read");
        memcpy(d, _pos, s);
        _pos += s;
        return true;
    }

    inline bool write(const char* d, size_t s) {
        stream_assert(_end - _pos >= (int32_t)s, "write");
        memcpy((void*)_pos, d, s);
        _pos += s;
        return true;
    }

    inline bool put(char c) {
        stream_assert(_pos < _end, "put");
        *_pos = c;
        ++_pos;
        return true;
    }

    inline bool get(unsigned char& c) { return get(*(char*)&c); }

    inline bool get(char& c) {
        stream_assert(_pos < _end, "get");
        c = *_pos;
        ++_pos;
        return true;
    }

    D pos() const { return _pos; }
    inline bool valid() const { return _pos <= _end && _pos >= _start; }

    inline bool seekp(size_t p) {
        _pos = _start + p;
        return _pos <= _end;
    }

    inline size_t tellp() const { return size_t(_pos - _start); }

    inline size_t remaining() const { return _end - _pos; }

private:
    D _start;
    D _pos;
    D _end;
};

/**
 * @brief Specialization of datastream used to help determine the final size of
 * a serialized value. Specialization of datastream used to help determine the
 * final size of a serialized value
 */
template <>
class datastream<size_t> {
public:
    datastream(size_t init_size = 0) : _size(init_size) {}

    inline bool skip(size_t s) {
        _size += s;
        return true;
    }

    inline bool write(const char*, size_t s) {
        _size += s;
        return true;
    }

    inline bool put(char) {
        ++_size;
        return true;
    }

    inline bool valid() const { return true; }

    inline bool seekp(size_t p) {
        _size = p;
        return true;
    }

    inline size_t tellp() const { return _size; }

    inline size_t remaining() const { return 0; }

private:
    size_t _size;
};

// Serialize a bool into a stream
template <typename D>
inline datastream<D>& operator<<(datastream<D>& ds, const bool& d) {
    return ds << uint8_t(d);
}

// Deserialize a bool from a stream
template <typename D>
inline datastream<D>& operator>>(datastream<D>& ds, bool& d) {
    uint8_t t;
    ds >> t;
    d = t;
    return ds;
}

// Serialize a string into a stream
template <typename D>
datastream<D>& operator<<(datastream<D>& ds, const std::string& v) {
    ds << unsigned_int(v.size());
    if (v.size())
        ds.write(v.data(), v.size());
    return ds;
}

// Deserialize a string from a stream
template <typename D>
datastream<D>& operator>>(datastream<D>& ds, std::string& v) {
    std::vector<char> tmp;
    ds >> tmp;
    if (tmp.size())
        v = std::string(tmp.data(), tmp.data() + tmp.size());
    else
        v = std::string();
    return ds;
}

// Serialize a fixed size array into a stream
template <typename D, typename T, std::size_t N>
datastream<D>& operator<<(datastream<D>& ds, const std::array<T, N>& v) {
    ds << unsigned_int(N);
    for (const auto& i : v)
        ds << i;
    return ds;
}

// Deserialize a fixed size array from a stream
template <typename D, typename T, std::size_t N>
datastream<D>& operator>>(datastream<D>& ds, std::array<T, N>& v) {
    unsigned_int s;
    ds >> s;
    stream_assert(N == s.value, "std::array size and unpacked size don't match");
    for (auto& i : v)
        ds >> i;
    return ds;
}

namespace _datastream_detail {
// Check if type T is a pointer
template <typename T>
constexpr bool is_pointer() {
    return std::is_pointer<T>::value || std::is_null_pointer<T>::value ||
           std::is_member_pointer<T>::value;
}

// Check if type T is a primitive type
template <typename T>
constexpr bool is_primitive() {
    return std::is_arithmetic<T>::value || std::is_enum<T>::value;
}
}  // namespace _datastream_detail

// Pointer should not be serialized, so this function will always throws an error
template <typename D,
          typename T,
          std::enable_if_t<_datastream_detail::is_pointer<T>()>* = nullptr>
datastream<D>& operator>>(datastream<D>& ds, T) {
    static_assert(!_datastream_detail::is_pointer<T>(),
                  "Pointers should not be serialized");
    return ds;
}

// Serialize a fixed size array of non-primitive and non-pointer type
template <typename D,
          typename T,
          std::size_t N,
          std::enable_if_t<!_datastream_detail::is_primitive<T>() &&
                           !_datastream_detail::is_pointer<T>()>* = nullptr>
datastream<D>& operator<<(datastream<D>& ds, const T (&v)[N]) {
    ds << unsigned_int(N);
    for (uint32_t i = 0; i < N; ++i)
        ds << v[i];
    return ds;
}

// Serialize a fixed size array of non-primitive type
template <typename D,
          typename T,
          std::size_t N,
          std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
datastream<D>& operator<<(datastream<D>& ds, const T (&v)[N]) {
    ds << unsigned_int(N);
    ds.write((char*)&v[0], sizeof(v));
    return ds;
}

// Deserialize a fixed size array of non-primitive and non-pointer type
template <typename D,
          typename T,
          std::size_t N,
          std::enable_if_t<!_datastream_detail::is_primitive<T>() &&
                           !_datastream_detail::is_pointer<T>()>* = nullptr>
datastream<D>& operator>>(datastream<D>& ds, T (&v)[N]) {
    unsigned_int s;
    ds >> s;
    stream_assert(N == s.value, "T[] size and unpacked size don't match");
    for (uint32_t i = 0; i < N; ++i)
        ds >> v[i];
    return ds;
}

// Deserialize a fixed size array of non-primitive type
template <typename D,
          typename T,
          std::size_t N,
          std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
datastream<D>& operator>>(datastream<D>& ds, T (&v)[N]) {
    unsigned_int s;
    ds >> s;
    stream_assert(N == s.value, "T[] size and unpacked size don't match");
    ds.read((char*)&v[0], sizeof(v));
    return ds;
}

// Serialize a vector of uint8_t
template <typename D>
datastream<D>& operator<<(datastream<D>& ds, const std::vector<uint8_t>& v) {
    ds << unsigned_int(v.size());
    ds.write(reinterpret_cast<const char*>(v.data()), v.size());
    return ds;
}

// Serialize a vector
template <typename D, typename T>
datastream<D>& operator<<(datastream<D>& ds, const std::vector<T>& v) {
    ds << unsigned_int(v.size());
    for (const auto& i : v)
        ds << i;
    return ds;
}

//  Deserialize a vector of uint8_t
template <typename D>
datastream<D>& operator>>(datastream<D>& ds, std::vector<uint8_t>& v) {
    unsigned_int s;
    ds >> s;
    v.resize(s);
    ds.read(reinterpret_cast<char*>(v.data()), v.size());
    return ds;
}

// Deserialize a vector
template <typename D, typename T>
datastream<D>& operator>>(datastream<D>& ds, std::vector<T>& v) {
    unsigned_int s;
    ds >> s;
    v.resize(s);
    for (auto& i : v)
        ds >> i;
    return ds;
}

//  Serialize a set
template <typename D, typename T>
datastream<D>& operator<<(datastream<D>& ds, const std::set<T>& s) {
    ds << unsigned_int(s.size());
    for (const auto& i : s) {
        ds << i;
    }
    return ds;
}

// Deserialize a set
template <typename D, typename T>
datastream<D>& operator>>(datastream<D>& ds, std::set<T>& s) {
    s.clear();
    unsigned_int sz;
    ds >> sz;

    for (uint32_t i = 0; i < sz; ++i) {
        T v;
        ds >> v;
        s.emplace(std::move(v));
    }
    return ds;
}
//  Serialize a map
template <typename D, typename K, typename V>
datastream<D>& operator<<(datastream<D>& ds, const std::map<K, V>& m) {
    ds << unsigned_int(m.size());
    for (const auto& i : m) {
        ds << i.first << i.second;
    }
    return ds;
}

//  Deserialize a map
template <typename D, typename K, typename V>
datastream<D>& operator>>(datastream<D>& ds, std::map<K, V>& m) {
    m.clear();
    unsigned_int s;
    ds >> s;

    for (uint32_t i = 0; i < s; ++i) {
        K k;
        V v;
        ds >> k >> v;
        m.emplace(std::move(k), std::move(v));
    }
    return ds;
}

//  Serialize a tuple
template <typename D, typename... Args>
datastream<D>& operator<<(datastream<D>& ds, const std::tuple<Args...>& t) {
    boost::fusion::for_each(t, [&](const auto& i) { ds << i; });
    return ds;
}

//  Deserialize a tuple
template <typename D, typename... Args>
datastream<D>& operator>>(datastream<D>& ds, std::tuple<Args...>& t) {
    boost::fusion::for_each(t, [&](auto& i) { ds >> i; });
    return ds;
}

//  Serialize a primitive type

template <typename D,
          typename T,
          std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
datastream<D>& operator<<(datastream<D>& ds, const T& v) {
    ds.write(reinterpret_cast<const char*>(&v), sizeof(T));
    return ds;
}

//  Deserialize a primitive type
template <typename D,
          typename T,
          std::enable_if_t<_datastream_detail::is_primitive<T>()>* = nullptr>
datastream<D>& operator>>(datastream<D>& ds, T& v) {
    ds.read(reinterpret_cast<char*>(&v), sizeof(T));
    return ds;
}

// Unpack data inside a fixed size buffer as T
template <typename T>
inline T unpack(const uint8_t* buffer, size_t len) {
    T result;
    datastream<const uint8_t*> ds(buffer, len);
    ds >> result;
    return result;
}

// Unpack data inside a variable size buffer as T
template <typename T>
T unpack(const std::vector<uint8_t>& bytes) {
    return unpack<T>(bytes.data(), bytes.size());
}

// Get the size of the packed data
template <typename T>
size_t pack_size(const T& value) {
    datastream<size_t> ps;
    ps << value;
    return ps.tellp();
}

// Get packed data
template <typename T>
std::vector<uint8_t> pack(const T& value) {
    // FIXME(dongwei.ldw):
    // Root Cause:
    //      every elem in result are Assigned twice
    // ToDo:
    //      reduce twice to once
    //      marked on aone: https://aone.alipay.com/task/19159428
    std::vector<uint8_t> result(pack_size(value));
    datastream<uint8_t*> ds(result.data(), result.size());
    ds << value;
    return result;
}
}  // namespace mychain
