
#pragma once

#include "common.hh"
#include "6502.hh"
#include "bytescan.hh"

struct AddressBlock
{
    std::uint16_t start;
    std::uint16_t size;

    constexpr bool contains(std::uint16_t address) const
    {
        return address >= start && address <= start + (size - 1);
    }

    constexpr bool operator < (AddressBlock const& r) const
    {
        return start < r.start;
    }
};

struct DataBlock
{
    std::uint16_t address;
    std::vector<byte_type> data;

    constexpr bool contains(std::uint16_t address) const
    {
        return address >= this->address && address <= this->address + (data.size() - 1);
    }

    operator AddressBlock () const
    {
        return { address, (std::uint16_t) data.size() };
    }

    inline SpanScanner bytes(AddressBlock const& block) const
    {
        if (block.start < address || block.start + block.size > address + data.size())
            throw std::logic_error(std::string("This is a bug! Found block at ") + hex_string<4>(block.start) + " of size " + hex_string<4>(block.size));

        return SpanScanner::from_vector(data, block.start - address, block.size);
    }
};

bool address_blocks_contain(std::vector<AddressBlock> const& blocks, std::uint16_t address);

std::vector<AddressBlock> inverted_blocks(AddressBlock const& range, std::vector<AddressBlock> const& blocks);

Instr decode_instr_operand(std::size_t addr, std::uint8_t opcode, ByteScanner& input);
Instr decode_instruction(std::size_t addr, ByteScanner& input);

template<typename Func>
static void for_each_instr(ByteScanner&& bytes, AddressBlock const& block, Func func)
{
    while (bytes.tell() < bytes.last())
    {
        std::uint16_t const addr = block.start + bytes.tell();
        func(addr, decode_instruction(addr, bytes));
    }
}
