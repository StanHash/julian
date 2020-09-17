
#include "print.hh"

std::string instr_to_string(Instr const& instr, std::vector<Symbol> const& symbols)
{
    // Step 1. find opcode info

    OpInfo const* const info = find_opcode_info(instr.opcode);

    if (info == nullptr)
    {
        using namespace std::string_literals;
        return ".db $"s + hex_string<2>(instr.opcode);
    }

    // Step 2. get operand string

    std::string operand_str = {};

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
    case Am::IAB:
    case Am::REL:
    {
        const auto syms = symbols_at(symbols, instr.operand);

        for (auto it = syms.first; it != syms.second; ++it)
        {
            if (info->flags & OpInfo::FLAG_JUMP)
            {
                if (it->flags & Symbol::FLAG_EXEC)
                {
                    operand_str = it->name;
                    break;
                }
            }
            else if (info->flags & OpInfo::FLAG_WRITE)
            {
                if (it->flags & Symbol::FLAG_WRITE)
                {
                    operand_str = it->name;
                    break;
                }
            }
            else
            {
                if (it->flags & Symbol::FLAG_READ)
                {
                    operand_str = it->name;
                    break;
                }
            }
        }

        break;
    }

    default:
        break;

    }

    if (operand_str.empty())
    {
        switch (get_addressing_mode_operand_size(info->addressing_mode))
        {

        case 1:
            operand_str.push_back('$');
            operand_str.append(hex_string<2>(instr.operand));
            break;

        case 2:
            operand_str.push_back('$');
            operand_str.append(hex_string<4>(instr.operand));
            break;

        default:
            break;

        }
    }

    // Step 3. print operand and decoration

    std::string result(info->name);

    switch (info->addressing_mode)
    {

    case Am::IMP:
        break;

    case Am::ACC:
    {
        result += " A";

        break;
    }

    case Am::IMM:
    {
        result += " #";
        result += operand_str;

        break;
    }

    case Am::ZRP:
    {
        result += " ";
        result += operand_str;

        break;
    }

    case Am::ZRX:
    {
        result += " ";
        result += operand_str;
        result += ", X";

        break;
    }

    case Am::ZRY:
    {
        result += " ";
        result += operand_str;
        result += ", Y";

        break;
    }

    case Am::ABS:
    {
        result += " ";
        result += operand_str;

        break;
    }

    case Am::ABX:
    {
        result += " ";
        result += operand_str;
        result += ", X";

        break;
    }

    case Am::ABY:
    {
        result += " ";
        result += operand_str;
        result += ", Y";

        break;
    }

    case Am::IAB:
    {
        result += " (";
        result += operand_str;
        result += ")";

        break;
    }

    case Am::INX:
    {
        result += " (";
        result += operand_str;
        result += ", X)";

        break;
    }

    case Am::INY:
    {
        result += " (";
        result += operand_str;
        result += "), Y";

        break;
    }

    case Am::REL:
    {
        result += " ";
        result += operand_str;

        break;
    }

    }

    return result;
}

std::vector<PrintItem> gen_print_items(AddressBlock const& range, std::vector<AddressBlock> const& code_blocks, std::vector<Symbol> const& symbols)
{
    std::vector<AddressBlock> const& data_blocks = inverted_blocks(range, code_blocks);

    enum struct Kind
    {
        Code, Data, Name,
    };

    struct Item { Kind kind; std::string name; };

    std::vector<std::pair<std::uint32_t, Item>> map;

    for (AddressBlock const& code_block : code_blocks)
        map.emplace_back(code_block.start, Item { Kind::Code, {} });

    for (AddressBlock const& data_block : data_blocks)
        map.emplace_back(data_block.start, Item { Kind::Data, {} });

    for (Symbol const& symbol : symbols)
        if (range.contains(symbol.value) && (symbol.flags & (Symbol::FLAG_READ | Symbol::FLAG_EXEC)))
            map.emplace_back(symbol.value, Item { Kind::Name, symbol.name });

    std::sort(map.begin(), map.end(), [&] (auto& left, auto& right) -> bool
    {
        if (left.first == right.first)
        {
            if (left.second.kind == Kind::Name)
                return true;

            return false;
        }

        return left.first < right.first;
    });

    std::vector<PrintItem> result;

    Kind prev_kind = Kind::Name;

    for (std::size_t i = 0; i < map.size(); ++i)
    {
        std::uint32_t const addr = map[i].first;
        std::uint32_t const size = [&] ()
        {
            if (i+1 == map.size())
                return range.start + range.size - addr;

            return map[i+1].first - addr;
        } ();

        Kind kind = map[i].second.kind;

        if (kind == Kind::Name)
        {
            result.emplace_back(PrintName(map[i].second.name));
            kind = prev_kind;
        }

        if (size != 0)
        {
            switch (kind)
            {

            case Kind::Code:
                result.emplace_back(PrintCode({ addr, size }));
                break;

            case Kind::Data:
                result.emplace_back(PrintData({ addr, size }));
                break;

            default:
                break;

            }
        }

        prev_kind = kind;
    }

    return result;
}

void print_items(DataBlock const& main_block, std::vector<PrintItem> const& items, std::vector<Symbol> const& symbols, std::ostream& output)
{
    for (PrintItem const& item : items)
    {
        std::visit([&] (auto& item)
        {
            using T = std::decay_t<decltype(item)>;

            if constexpr (std::is_same_v<T, PrintCode>)
            {
                for_each_instr(main_block.bytes(item), item, [&] (std::uint32_t addr, Instr const& instr)
                {
                    auto const first = main_block.data.begin() + (addr - main_block.address);
                    auto const last  = main_block.data.begin() + (addr + get_instr_size(instr) - main_block.address);

                    output << "    /* " << hex_string<4>(addr) << " " << hex_string<8>(first, last) << " */ " << instr_to_string(instr, symbols) << std::endl;
                });

                output << std::endl;
            }

            if constexpr (std::is_same_v<T, PrintData>)
            {
                constexpr std::size_t BYTES_PER_LINE = 8;

                for (std::size_t i = 0; i < item.size; i += BYTES_PER_LINE)
                {
                    std::size_t count = std::min(BYTES_PER_LINE, item.size - i);

                    output << "    /* " << hex_string<4>(item.start + i) << " ...      */ .db ";

                    for (std::size_t j = 0; j < count; ++j)
                    {
                        byte_type const byte = main_block.data[item.start - main_block.address + i + j];

                        if (j != 0)
                            output << ", ";

                        output << "$" << hex_string<2>(byte);
                    }

                    output << std::endl;
                }

                output << std::endl;
            }

            if constexpr (std::is_same_v<T, PrintName>)
            {
                output << item << ":" << std::endl;
            }
        }, item);
    }
}

void print_symbols(AddressBlock const& main_block, std::vector<Symbol> const& symbols, std::ostream& output)
{
    for (Symbol const& symbol : symbols)
    {
        if (main_block.contains(symbol.value))
            continue;

        output << "    " << symbol.name << " = $" << hex_string<4>(symbol.value) << std::endl;
    }

    output << std::endl;
}
