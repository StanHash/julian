
#include "anal.hh"

#include <set>

#include <iostream> // FIXME: remove this and use a Log class instead

static bool is_valid_jump_target(AnalConfig const& anal, std::uint32_t address)
{
    auto const symbols = symbols_at(anal.symbols, address);

    for (auto it = symbols.first; it != symbols.second; ++it)
    {
        if (it->flags & Symbol::FLAG_EXEC)
            return true;
    }

    for (Segment const& chunk : anal.segments)
    {
        if (!chunk.contains(address))
            continue;

        return !!(chunk.flags & Segment::FLAG_EXEC);
    }

    return false;
}

static bool is_valid_read_target(AnalConfig const& anal, std::uint32_t address)
{
    auto const symbols = symbols_at(anal.symbols, address);

    for (auto it = symbols.first; it != symbols.second; ++it)
    {
        if (it->flags & Symbol::FLAG_READ)
            return true;
    }

    for (Segment const& chunk : anal.segments)
    {
        if (!chunk.contains(address))
            continue;

        return !!(chunk.flags & Segment::FLAG_READ);
    }

    return false;
}

static bool is_valid_write_target(AnalConfig const& anal, std::uint32_t address)
{
    auto const symbols = symbols_at(anal.symbols, address);

    for (auto it = symbols.first; it != symbols.second; ++it)
    {
        if (it->flags & Symbol::FLAG_WRITE)
            return true;
    }

    for (Segment const& chunk : anal.segments)
    {
        if (!chunk.contains(address))
            continue;

        return !!(chunk.flags & Segment::FLAG_WRITE);
    }

    return false;
}

static std::optional<AddressBlock> scan_code(AnalConfig const& anal, AddressBlock const& range)
{
    SpanScanner bytes = anal.main_block.bytes(range);

    std::cerr << "Scanning code at " << hex_string<4>(range.start) << std::endl;

    while (bytes.tell() < bytes.last())
    {
        std::uint32_t const addr = range.start + bytes.tell();

        std::uint8_t const opcode = bytes.consume();
        OpInfo const* info = anal.get_opcode_info(opcode);

        if (info == nullptr || (bytes.tell() + get_addressing_mode_operand_size(info->addressing_mode)) > bytes.last())
        {
            // Illegal instruction: block isn't valid

            std::cerr << "Invalidated " << hex_string<4>(range.start) << ": " << hex_string<4>(addr) << " is not an instruction." << std::endl;

            return {};
        }

        if (info->mnemonic == Mnem::BRK && !anal.allow_brk)
        {
            // We assume BRKs are invalid

            std::cerr << "Invalidated " << hex_string<4>(range.start) << ": BRK is not allowed." << std::endl;

            return {};
        }

        Instr const instr = decode_instr_operand(addr, opcode, bytes);

        switch (info->addressing_mode)
        {

        case Am::ZRP:
        case Am::ZRX:
        case Am::ZRY:
        case Am::ABS:
        case Am::ABX:
        case Am::ABY:
        case Am::REL:
            if (info->flags & OpInfo::FLAG_JUMP)
            {
                if (!is_valid_jump_target(anal, instr.operand))
                {
                    std::cerr << "Invalidated " << hex_string<4>(range.start) << ": " << hex_string<4>(instr.operand) << " is bad jump target." << std::endl;

                    return {};
                }

                break;
            }

            [[fallthrough]];

        case Am::INX:
        case Am::INY:
        case Am::IAB:
            if (info->flags & OpInfo::FLAG_WRITE)
            {
                if (!is_valid_write_target(anal, instr.operand))
                {
                    std::cerr << "Invalidated " << hex_string<4>(range.start) << ": " << hex_string<4>(instr.operand) << " is bad write target." << std::endl;

                    return {};
                }
            }
            else
            {
                if (!is_valid_read_target(anal, instr.operand))
                {
                    std::cerr << "Invalidated " << hex_string<4>(range.start) << ": " << hex_string<4>(instr.operand) << " is bad read target." << std::endl;

                    return {};
                }
            }

            break;

        default:
            break;

        }

        if (info->flags & OpInfo::FLAG_JUMP)
        {
            return AddressBlock { range.start, (std::uint32_t) bytes.tell() };
        }
    }

    std::cerr << "Invalidated " << hex_string<4>(range.start) << ": reached end of analysis range." << std::endl;

    return {};
}

static std::vector<std::uint32_t> list_code_points(AnalConfig const& anal, std::vector<AddressBlock> const& blocks)
{
    std::vector<std::uint32_t> result;

    for (AddressBlock const& block : blocks)
    {
        for_each_instr(anal.main_block.bytes(block), block, [&] (std::uint32_t addr, [[maybe_unused]] Instr const& instr)
        {
            result.push_back(addr);
        });
    }

    return result;
}

static std::vector<std::uint32_t> list_code_xrefs(AnalConfig const& anal, std::vector<AddressBlock> const& blocks)
{
    std::vector<std::uint32_t> result;

    for (AddressBlock const& block : blocks)
    {
        for_each_instr(anal.main_block.bytes(block), block, [&] ([[maybe_unused]] std::uint32_t addr, Instr const& instr)
        {
            OpInfo const* info = get_instr_info(instr, anal);

            if (info->flags & OpInfo::FLAG_JUMP)
            {
                switch (info->addressing_mode)
                {

                case Am::ABS:
                case Am::REL:
                    result.push_back(instr.operand);
                    break;

                default:
                    break;

                }
            }
        });
    }

    return result;
}

static std::vector<AddressBlock> scan_code_at_points(AnalConfig const& anal, std::vector<AddressBlock> const& ranges, std::vector<std::uint32_t> const& scan_points)
{
    std::vector<AddressBlock> result;
    std::set<std::uint32_t> scanned;

    std::size_t i = 0;

    for (AddressBlock range : ranges)
    {
        while (i < scan_points.size() && scan_points[i] < range.start)
            i++;

        while (i < scan_points.size() && range.contains(scan_points[i]))
        {
            if (!scanned.count(scan_points[i]))
            {
                std::cerr << "Begin scan at point " << hex_string<4>(scan_points[i]) << std::endl;

                std::uint32_t addr = scan_points[i];
                std::uint32_t max_len = range.size - (addr - range.start);

                while (addr < range.start + range.size)
                {
                    scanned.insert(addr);

                    std::optional<AddressBlock> opt_block = scan_code(anal, { addr, max_len });

                    if (!opt_block.has_value())
                        break;

                    result.push_back(opt_block.value());

                    bool at_end = false;

                    for_each_instr(anal.main_block.bytes(opt_block.value()), opt_block.value(), [&] ([[maybe_unused]] std::uint32_t addr, Instr const& instr)
                    {
                        OpInfo const* info = get_instr_info(instr, anal);

                        if ((info->flags & OpInfo::FLAG_JUMP) && (info->flags & OpInfo::FLAG_END))
                            at_end = true;
                    });

                    if (at_end)
                        break;

                    addr += opt_block.value().size;
                    max_len -= opt_block.value().size;
                }
            }

            i++;
        }
    }

    return result;
}

static std::vector<AddressBlock> find_code_blocks_using_symbols(AnalConfig const& anal, AddressBlock const& range)
{
    std::vector<AddressBlock> result;

    std::set<std::uint32_t> analysed_points;

    auto const get_new_points = [&] (std::vector<AddressBlock> const& analysed_blocks) -> std::vector<std::uint32_t>
    {
        std::vector<std::uint32_t> xrefs = list_code_xrefs(anal, analysed_blocks);

        xrefs.erase(std::remove_if(xrefs.begin(), xrefs.end(), [&] (std::uint32_t xref)
        {
            if (!range.contains(xref))
                return true;

            for (AddressBlock const& block : analysed_blocks)
                if (block.contains(xref))
                    return true;

            if (analysed_points.count(xref))
                return true;

            analysed_points.insert(xref);

            return false;
        }), xrefs.end());

        std::sort(xrefs.begin(), xrefs.end());

        return xrefs;
    };

    std::vector<std::uint32_t> current_points;

    for (Symbol const& symbol : anal.symbols)
    {
        if (!range.contains(symbol.value))
            continue;

        if (!(symbol.flags & Symbol::FLAG_EXEC))
            continue;

        current_points.push_back(symbol.value);
    }

    std::sort(current_points.begin(), current_points.end());

    do
    {
        std::vector<AddressBlock> new_blocks = scan_code_at_points(anal, inverted_blocks(range, result), current_points);
        result = merge_sorted_vectors(result, new_blocks);
    }
    while (!(current_points = get_new_points(result)).empty());

    return result;
}

static std::vector<AddressBlock> find_code_blocks_linearly(AnalConfig const& anal, AddressBlock const& range)
{
    std::vector<AddressBlock> result;

    std::cerr << "Begin linear scan at " << hex_string<4>(range.start) << std::endl;

    std::uint32_t current_offset = 0;

    while (current_offset < range.size)
    {
        std::uint32_t const start = range.start + current_offset;
        std::uint32_t const size = range.size - current_offset;

        std::optional<AddressBlock> const opt_block = scan_code(anal, { start, size });

        if (opt_block.has_value())
        {
            AddressBlock const& block = opt_block.value();

            result.push_back(block);
            current_offset = block.start - range.start + block.size;
        }
        else
        {
            current_offset = current_offset + 1;
        }
    }

    return result;
}

static std::vector<AddressBlock> find_code_blocks_linearly(AnalConfig const& anal, std::vector<AddressBlock> const& ranges)
{
    std::vector<AddressBlock> result;

    for (AddressBlock const& range : ranges)
    {
        std::vector<AddressBlock> const new_blocks = find_code_blocks_linearly(anal, range);

        result.reserve(result.size() + new_blocks.size());
        std::copy(new_blocks.begin(), new_blocks.end(), std::back_inserter(result));
    }

    return result;
}

static bool remove_bad_jump_blocks(AnalConfig const& anal, std::vector<AddressBlock>& blocks, std::vector<std::uint32_t> const& all_code_points)
{
    std::vector<AddressBlock> result;
    result.reserve(blocks.size());

    std::cerr << "Checking for bad jump blocks..." << std::endl;

    SpanScanner bytes = SpanScanner::from_vector(anal.main_block.data);

    for (AddressBlock block : blocks)
    {
        std::uint32_t start_offset = block.start - anal.main_block.address;

        bytes.seek(start_offset);

        while (bytes.tell() - start_offset < block.size)
        {
            std::uint32_t const addr = anal.main_block.address + bytes.tell();

            Instr const instr = decode_instruction(addr, bytes);
            OpInfo const* const info = get_instr_info(instr, anal);

            auto const remove = [&] ()
            {
                // invalidate block up to now

                std::uint32_t const new_start = anal.main_block.address + bytes.tell();

                block.size -= new_start - block.start;
                block.start = new_start;

                start_offset = new_start - anal.main_block.address;
            };

            if (info == nullptr)
            {
                remove();
                continue;
            }

            if (info->flags & OpInfo::FLAG_JUMP)
            {
                switch (info->addressing_mode)
                {

                case Am::REL:
                case Am::ABS:
                    if (anal.main_block.contains(instr.operand) && !in_sorted_vector(all_code_points, (std::uint32_t) instr.operand))
                    {
                        // invalidate block up to now
                        std::cerr << "Removed " << hex_string<4>(block.start) << ": " << hex_string<4>(instr.operand) << " is bad jump target." << std::endl;
                        remove();
                    }

                    break;

                default:
                    break;

                }
            }
        }

        if (block.size == 0)
            continue;

        result.push_back(block);
    }

    std::swap(blocks, result);
    return result.size() != blocks.size();
}

static bool remove_isolated_blocks(AnalConfig const& anal, std::vector<AddressBlock>& blocks)
{
    std::vector<AddressBlock> result;

    std::cerr << "Checking for isolated blocks..." << std::endl;

    for (std::size_t i = 0; i < blocks.size(); ++i)
    {
        std::uint32_t const size = blocks[i].size;

        if (size < 12) // TODO: this could be configurable
        {
            std::uint32_t const prev_addr = (i == 0) ? anal.main_block.address : blocks[i-1].start + blocks[i-1].size;
            std::uint32_t const next_addr = (i == blocks.size()-1) ? anal.main_block.address + anal.main_block.data.size() : blocks[i+1].start;

            std::uint32_t const lo_addr = blocks[i].start;
            std::uint32_t const hi_addr = blocks[i].start + blocks[i].size;

            if ((lo_addr - size > prev_addr) && (hi_addr + size < next_addr))
            {
                std::cerr << "Removed isolated block at " << hex_string<4>(lo_addr) << std::endl;
                continue;
            }
        }

        result.push_back(blocks[i]);
    }

    std::swap(blocks, result);
    return result.size() != blocks.size();
}

std::vector<AddressBlock> analyse_code_blocks(AnalConfig const& anal)
{
    std::vector<AddressBlock> result = find_code_blocks_using_symbols(anal, anal.main_block);

    std::vector<AddressBlock> linear_blocks = find_code_blocks_linearly(anal, inverted_blocks(anal.main_block, result));
    result = merge_sorted_vectors(result, linear_blocks);

    std::vector<std::uint32_t> code_points;

    do
    {
        code_points = list_code_points(anal, result);
    }
    while (remove_bad_jump_blocks(anal, result, code_points) || remove_isolated_blocks(anal, result));

    return result;
}

std::vector<Symbol> build_symbols(AnalConfig const& anal, std::vector<AddressBlock> const& blocks, bool extended_symbols)
{
    std::vector<Symbol> result;

    auto const sym_compare = [] (Symbol const& l, Symbol const& r)
    {
        return (l.value == r.value);
    };

    auto const add_symbol = [&] (std::string&& name, std::uint32_t val, std::uint8_t flags)
    {
        if (!extended_symbols && !anal.main_block.contains(val))
            return;

        Symbol symbol { std::move(name), val, flags };

        auto const anal_symbols = symbols_at(anal.symbols, val);

        for (auto it = anal_symbols.first; it != anal_symbols.second; ++it)
        {
            if (it->value == symbol.value && it->flags == symbol.flags)
                return;
        }

        result.push_back(std::move(symbol));
    };

    for (AddressBlock const& block : blocks)
    {
        for_each_instr(anal.main_block.bytes(block), block, [&] ([[maybe_unused]] std::uint32_t addr, Instr const& instr)
        {
            OpInfo const* const info = anal.get_opcode_info(instr.opcode);

            switch (info->addressing_mode)
            {

            case Am::ZRP:
            case Am::ZRX:
            case Am::ZRY:
            case Am::ABS:
            case Am::ABX:
            case Am::ABY:
            case Am::INX:
            case Am::INY:
            case Am::REL:
                if (info->flags & OpInfo::FLAG_JUMP)
                {
                    add_symbol(std::string("CODE_") + hex_string<4>(instr.operand), instr.operand, Symbol::FLAG_EXEC);
                    break;
                }

                [[fallthrough]];

            case Am::IAB:
            {
                using flags_t = decltype(Symbol::flags);

                flags_t flags = (info->flags & OpInfo::FLAG_WRITE)
                    ? Symbol::FLAG_WRITE : Symbol::FLAG_READ;

                for (Segment const& segment : anal.segments)
                {
                    if (!segment.contains(instr.operand))
                        continue;

                    if (segment.flags & Segment::FLAG_EXEC)
                        flags |= Symbol::FLAG_EXEC;

                    if (segment.flags & Segment::FLAG_READ)
                        flags |= Symbol::FLAG_READ;

                    if (segment.flags & Segment::FLAG_WRITE)
                        flags |= Symbol::FLAG_WRITE;

                    break;
                }

                add_symbol(std::string("DATA_") + hex_string<4>(instr.operand), instr.operand, flags);

                break;
            }

            default:
                break;

            }
        });
    }

    std::sort(result.begin(), result.end(), Symbol::Compare {});
    result.erase(std::unique(result.begin(), result.end(), sym_compare), result.end());

    return result;
}
