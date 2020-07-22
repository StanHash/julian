
#pragma once

#include "common.hh"
#include "6502.hh"
#include "disasm.hh"
#include "symbol.hh"

struct Segment : public AddressBlock
{
    enum
    {
        FLAG_READ  = (1 << 0),
        FLAG_WRITE = (1 << 1),
        FLAG_EXEC  = (1 << 2),
    };

    std::string name;
    std::uint8_t flags;
};

struct AnalConfig : public OpInfoCache
{
    DataBlock main_block;

    std::vector<Segment> segments;
    std::vector<Symbol> symbols;

    bool allow_brk : 1;
};

std::vector<AddressBlock> analyse_code_blocks(AnalConfig const& ctx);
std::vector<Symbol> build_symbols(AnalConfig const& anal, std::vector<AddressBlock> const& blocks, bool extended_symbols);
