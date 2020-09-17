
#include "disasm.hh"

bool address_blocks_contain(std::vector<AddressBlock> const& blocks, std::uint32_t address)
{
    // Variant of binary search
    // Assumes `blocks` is sorted and has no overlaps

    int l = 0;
    int r = blocks.size() - 1;

    while (l <= r)
    {
        int const m = (l + r) / 2;

        if (blocks[m].start + blocks[m].size - 1 < address)
        {
            l = m + 1;
            continue;
        }

        if (address < blocks[m].start)
        {
            r = m - 1;
            continue;
        }

        return true;
    }

    return false;
}

std::vector<AddressBlock> inverted_blocks(AddressBlock const& range, std::vector<AddressBlock> const& blocks)
{
    std::vector<AddressBlock> result;

    std::uint32_t start = range.start;

    for (AddressBlock const& block : blocks)
    {
        if (block.start > start)
            result.push_back({ start, (std::uint32_t) (block.start - start) });

        start = block.start + block.size;

        if (start >= range.start + range.size)
            break;
    }

    if (start < range.start + range.size)
    {
        std::uint32_t const new_start = start;
        std::uint32_t const new_size = range.start + range.size - start;

        result.push_back({ new_start, new_size });
    }

    return result;
}


Instr decode_instr_operand(std::size_t addr, std::uint8_t opcode, ByteScanner& input)
{
    Instr result;

    result.opcode = opcode;

    OpInfo const* const info = find_opcode_info(result.opcode);

    if (info != nullptr)
    {
        switch (info->addressing_mode)
        {

        case Am::IMP:
            break;

        case Am::ACC:
            break;

        case Am::IMM:
            result.operand = input.consume();
            break;

        case Am::ZRP:
            result.operand = input.consume();
            break;

        case Am::ZRX:
            result.operand = input.consume();
            break;

        case Am::ZRY:
            result.operand = input.consume();
            break;

        case Am::ABS:
        {
            byte_type const lo = input.consume();
            byte_type const hi = input.consume();

            result.operand = lo | (hi << 8);

            break;
        }

        case Am::ABX:
        {
            byte_type const lo = input.consume();
            byte_type const hi = input.consume();

            result.operand = lo | (hi << 8);

            break;
        }

        case Am::ABY:
        {
            byte_type const lo = input.consume();
            byte_type const hi = input.consume();

            result.operand = lo | (hi << 8);

            break;
        }

        case Am::IAB:
        {
            byte_type const lo = input.consume();
            byte_type const hi = input.consume();

            result.operand = lo | (hi << 8);

            break;
        }

        case Am::INX:
            result.operand = input.consume();
            break;

        case Am::INY:
            result.operand = input.consume();
            break;

        case Am::REL:
        {
            std::int8_t const operand = input.consume();

            result.operand = addr + 2 + operand;

            break;
        }

        }
    }

    return result;
}

Instr decode_instruction(std::size_t addr, ByteScanner& input)
{
    return decode_instr_operand(addr, input.consume(), input);
}
