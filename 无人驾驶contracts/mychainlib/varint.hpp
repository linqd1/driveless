

#pragma once

/**
 * @defgroup varint Variable Length Integer Type
 * @brief Defines variable length integer type which provides more efficient
 * serialization
 * @ingroup types
 * @{/
 */

/**
 * Variable Length Unsigned Integer. This provides more efficient serialization
 * of 32-bit unsigned int. It serialuzes a 32-bit unsigned integer in as few
 * bytes as possible `varuint32` is unsigned and uses
 * [LEB128](https://en.wikipedia.org/wiki/LEB128)
 *
 * @brief Variable Length Unsigned Integer
 */
struct unsigned_int {
    /**
     * Construct a new unsigned int object
     *
     * @brief Construct a new unsigned int object
     * @param v - Source
     */
    unsigned_int(uint32_t v = 0) : value(v) {}

    /**
     * Construct a new unsigned int object from a type that is convertible to
     * uint32_t
     *
     * @brief Construct a new unsigned int object
     * @tparam T - Type of the source
     * @param v - Source
     * @pre T must be convertible to uint32_t
     */
    template <typename T>
    unsigned_int(T v) : value(v) {}

    // operator uint32_t()const { return value; }
    // operator uint64_t()const { return value; }

    /**
     * Convert unsigned_int as T
     * @brief Conversion Operator
     * @tparam T - Target type of conversion
     * @return T - Converted target
     */
    template <typename T>
    operator T() const {
        return static_cast<T>(value);
    }

    /**
     * Assign 32-bit unsigned integer
     *
     * @brief Assignment operator
     * @param v - Soruce
     * @return unsigned_int& - Reference to this object
     */
    unsigned_int& operator=(uint32_t v) {
        value = v;
        return *this;
    }

    /**
     * Contained value
     *
     * @brief Contained value
     */
    uint32_t value;

    /**
     * Check equality between a unsigned_int object and 32-bit unsigned integer
     *
     * @brief Equality Operator
     * @param i - unsigned_int object to compare
     * @param v - 32-bit unsigned integer to compare
     * @return true - if equal
     * @return false - otherwise
     */
    friend bool operator==(const unsigned_int& i, const uint32_t& v) {
        return i.value == v;
    }

    /**
     * Check equality between 32-bit unsigned integer and  a unsigned_int object
     *
     * @brief Equality Operator
     * @param i - 32-bit unsigned integer to compare
     * @param v - unsigned_int object to compare
     * @return true - if equal
     * @return false - otherwise
     */
    friend bool operator==(const uint32_t& i, const unsigned_int& v) {
        return i == v.value;
    }

    /**
     * Check equality between two unsigned_int objects
     *
     * @brief Equality Operator
     * @param i - First unsigned_int object to compare
     * @param v - Second unsigned_int object to compare
     * @return true - if equal
     * @return false - otherwise
     */
    friend bool operator==(const unsigned_int& i, const unsigned_int& v) {
        return i.value == v.value;
    }

    /**
     * Check inequality between a unsigned_int object and 32-bit unsigned integer
     *
     * @brief Inequality Operator
     * @param i - unsigned_int object to compare
     * @param v - 32-bit unsigned integer to compare
     * @return true - if inequal
     * @return false - otherwise
     */
    friend bool operator!=(const unsigned_int& i, const uint32_t& v) {
        return i.value != v;
    }

    /**
     * Check inequality between 32-bit unsigned integer and  a unsigned_int object
     *
     * @brief Equality Operator
     * @param i - 32-bit unsigned integer to compare
     * @param v - unsigned_int object to compare
     * @return true - if unequal
     * @return false - otherwise
     */
    friend bool operator!=(const uint32_t& i, const unsigned_int& v) {
        return i != v.value;
    }

    /**
     * Check inequality between two unsigned_int objects
     *
     * @brief Inequality Operator
     * @param i - First unsigned_int object to compare
     * @param v - Second unsigned_int object to compare
     * @return true - if inequal
     * @return false - otherwise
     */
    friend bool operator!=(const unsigned_int& i, const unsigned_int& v) {
        return i.value != v.value;
    }

    /**
     * Check if the given unsigned_int object is less than the given 32-bit
     * unsigned integer
     *
     * @brief Less than Operator
     * @param i - unsigned_int object to compare
     * @param v - 32-bit unsigned integer to compare
     * @return true - if i less than v
     * @return false - otherwise
     */
    friend bool operator<(const unsigned_int& i, const uint32_t& v) {
        return i.value < v;
    }

    /**
     * Check if the given 32-bit unsigned integer is less than the given
     * unsigned_int object
     *
     * @brief Less than Operator
     * @param i - 32-bit unsigned integer to compare
     * @param v - unsigned_int object to compare
     * @return true -  if i less than v
     * @return false - otherwise
     */
    friend bool operator<(const uint32_t& i, const unsigned_int& v) {
        return i < v.value;
    }

    /**
     * Check if the first given unsigned_int is less than the second given
     * unsigned_int object
     *
     * @brief Less than Operator
     * @param i - First unsigned_int object to compare
     * @param v - Second unsigned_int object to compare
     * @return true -  if i less than v
     * @return false - otherwise
     */
    friend bool operator<(const unsigned_int& i, const unsigned_int& v) {
        return i.value < v.value;
    }

    /**
     * Check if the given unsigned_int object is greater or equal to the given
     * 32-bit unsigned integer
     *
     * @brief Greater or Equal to Operator
     * @param i - unsigned_int object to compare
     * @param v - 32-bit unsigned integer to compare
     * @return true - if i is greater or equal to v
     * @return false - otherwise
     */
    friend bool operator>=(const unsigned_int& i, const uint32_t& v) {
        return i.value >= v;
    }

    /**
     * Check if the given 32-bit unsigned integer is greater or equal to the given
     * unsigned_int object
     *
     * @brief Greater or Equal to Operator
     * @param i - 32-bit unsigned integer to compare
     * @param v - unsigned_int object to compare
     * @return true -  if i is greater or equal to v
     * @return false - otherwise
     */
    friend bool operator>=(const uint32_t& i, const unsigned_int& v) {
        return i >= v.value;
    }

    /**
     * Check if the first given unsigned_int is greater or equal to the second
     * given unsigned_int object
     *
     * @brief Greater or Equal to Operator
     * @param i - First unsigned_int object to compare
     * @param v - Second unsigned_int object to compare
     * @return true -  if i is greater or equal to v
     * @return false - otherwise
     */
    friend bool operator>=(const unsigned_int& i, const unsigned_int& v) {
        return i.value >= v.value;
    }

    /**
     *  Serialize an unsigned_int object with as few bytes as possible
     *
     *  @brief Serialize an unsigned_int object with as few bytes as possible
     *  @param ds - The stream to write
     *  @param v - The value to serialize
     *  @tparam DataStream - Type of datastream
     *  @return DataStream& - Reference to the datastream
     */
    template <typename DataStream>
    friend DataStream& operator<<(DataStream& ds, const unsigned_int& v) {
        uint64_t val = v.value;
        do {
            uint8_t b = uint8_t(val) & 0x7f;
            val >>= 7;
            b |= ((val > 0) << 7);
            ds.write((char*)&b, 1);  //.put(b);
        } while (val);
        return ds;
    }

    /**
     *  Deserialize an unsigned_int object
     *
     *  @brief Deserialize an unsigned_int object
     *  @param ds - The stream to read
     *  @param vi - The destination for deserialized value
     *  @tparam DataStream - Type of datastream
     *  @return DataStream& - Reference to the datastream
     */
    template <typename DataStream>
    friend DataStream& operator>>(DataStream& ds, unsigned_int& vi) {
        uint64_t v = 0;
        char b = 0;
        uint8_t by = 0;
        do {
            ds.get(b);
            v |= uint32_t(uint8_t(b) & 0x7f) << by;
            by += 7;
        } while (uint8_t(b) & 0x80);
        vi.value = static_cast<uint32_t>(v);
        return ds;
    }
};
