
#pragma once

#include "common.hh"

struct Symbol
{
    enum
    {
        FLAG_READ  = (1 << 0),
        FLAG_WRITE = (1 << 1),
        FLAG_EXEC  = (1 << 2),
    };

    std::string name;
    std::uint16_t value;
    std::uint8_t flags;

    constexpr bool operator < (Symbol const& other) const
    {
        return value < other.value;
    }

    struct Compare
    {
        constexpr bool operator () (Symbol const& l, Symbol const& r) const
        {
            return l.value < r.value;
        }

        constexpr bool operator () (Symbol const& l, std::uint16_t r) const
        {
            return l.value < r;
        }

        constexpr bool operator () (std::uint16_t l, Symbol const& r) const
        {
            return l < r.value;
        }
    };
};

inline auto symbols_at(std::vector<Symbol> const& symbols, std::uint16_t address)
{
    return std::equal_range(symbols.begin(), symbols.end(), address, Symbol::Compare {});
}
