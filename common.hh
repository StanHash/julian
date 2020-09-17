
#pragma once

#include <cstdint>

#include <string>
#include <stdexcept>

#include <vector>
#include <algorithm>

#define JULIAN_VERSION_STRING "1.0"

using byte_type = std::uint8_t;

template<unsigned DigitCount, typename IntType = unsigned>
std::string hex_string(IntType value)
{
    std::string result(DigitCount, ' ');

    auto const to_digit = [] (unsigned val) -> char
    {
        if (val >= 10)
            return 'A' + (val - 10);

        return '0' + val;
    };

    for (unsigned i = 0; i < DigitCount; ++i)
        result[i] = to_digit((value >> ((DigitCount-(i+1))*4)) & 0xF);

    return result;
}

template<unsigned MinLength = 0, typename IteratorType>
std::string hex_string(IteratorType first, IteratorType last)
{
    static_assert(std::is_same_v<typename std::iterator_traits<IteratorType>::value_type, byte_type>);

    std::string result(std::max<std::size_t>(std::distance(first, last)*3-1, MinLength), ' ');

    auto const to_digit = [] (unsigned val) -> char
    {
        if (val >= 10)
            return 'A' + (val - 10);

        return '0' + val;
    };

    std::size_t i = 0;

    while (first != last)
    {
        result[i*3+0] = to_digit((*first >> 4) & 0xF);
        result[i*3+1] = to_digit((*first) & 0xF);

        ++first;
        ++i;
    }

    return result;
}

struct HexDecodeError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template<typename IntType = unsigned>
IntType hex_decode(std::string_view str)
{
    if (str.size() >= 1 && str[0] == '$')
        str = str.substr(1);
    else if (str.size() >= 2 && str[0] == '0' && str[1] == 'x')
        str = str.substr(2);

    if (str.size() == 0)
        throw HexDecodeError("Bad hex string (no number)");

    IntType result = 0;

    for (unsigned i = 0; i < str.size(); ++i)
    {
        char const chr = str[i];

        result = result << 4;

        if (chr >= '0' && chr <= '9')
            result += chr - '0';
        else if (chr >= 'A' && chr <= 'F')
            result += 10 + chr - 'A';
        else if (chr >= 'a' && chr <= 'f')
            result += 10 + chr - 'a';
        else
            throw HexDecodeError("Bad hex string (found invalid digit character)");
    }

    return result;
}

template<typename T, typename Compare = std::less<T>>
bool in_sorted_vector(std::vector<T> const& vec, T const& val, Compare compare = Compare())
{
    return std::binary_search(
        vec.begin(), vec.end(),
        val,
        compare);
}

template<typename T, typename Compare = std::less<T>>
std::vector<T> merge_sorted_vectors(std::vector<T> const& a, std::vector<T> const& b, Compare compare = Compare())
{
    std::vector<T> result;
    result.reserve(a.size() + b.size());

    std::merge(
        a.begin(), a.end(),
        b.begin(), b.end(),
        std::back_inserter(result),
        compare);

    return result;
}

template<typename T, typename Compare = std::less<T>>
std::vector<T> merge_sorted_vectors(std::vector<T>&& a, std::vector<T>&& b, Compare compare = Compare())
{
    std::vector<T> result;
    result.reserve(a.size() + b.size());

    std::size_t i = 0;
    std::size_t j = 0;

    while (i < a.size() && j < b.size())
    {
        if (compare(a[i], b[j]))
            result.push_back(std::move(a[i++]));
        else
            result.push_back(std::move(b[j++]));
    }

    while (i < a.size())
        result.push_back(std::move(a[i++]));

    while (j < b.size())
        result.push_back(std::move(b[j++]));
}
