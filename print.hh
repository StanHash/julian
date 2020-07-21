
#pragma once

#include "common.hh"
#include "disasm.hh"
#include "symbol.hh"

#include <variant>
#include <iostream>

std::string instr_to_string(Instr const& instr, std::vector<Symbol> const& symbols);

struct PrintCode : public AddressBlock
{
    PrintCode(AddressBlock&& a)
        : AddressBlock(std::move(a)) {}
};

struct PrintData : public AddressBlock
{
    PrintData(AddressBlock&& a)
        : AddressBlock(std::move(a)) {}
};

struct PrintName : public std::string
{
    PrintName(std::string&& a)
        : std::string(std::move(a)) {}

    PrintName(std::string const& a)
        : std::string(a) {}
};

using PrintItem = std::variant<PrintCode, PrintData, PrintName>;

std::vector<PrintItem> gen_print_items(AddressBlock const& range, std::vector<AddressBlock> const& code_blocks, std::vector<Symbol> const& symbols);
void print_items(DataBlock const& main_block, std::vector<PrintItem> const& items, std::vector<Symbol> const& symbols, std::ostream& output);
