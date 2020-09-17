
#include "common.hh"
#include "csv.hh"
#include "anal.hh"
#include "print.hh"
#include "args.hh"

#include <fstream>
#include <cstring>

int main(int argc, char** argv)
{
    Args args = parse_args(argc, argv);

    AnalConfig anal {};

    // Read input data

    std::ifstream input(std::string { args.input_filename }, std::ios::in | std::ios::binary);

    if (!input.is_open())
    {
        std::cerr << "Couldn't open input file \"" << args.input_filename << "\"" << std::endl;
        std::cerr << std::endl;

        return 3;
    }

    if (args.input_size == 0)
    {
        input.seekg(0, std::ios::end);
        args.input_size = ((std::size_t) input.tellg()) - args.input_offset;
    }

    input.seekg(args.input_offset, std::ios::beg);

    anal.main_block.address = args.base_address;
    anal.main_block.data.resize(args.input_size);

    if (!input.read(reinterpret_cast<char*>(anal.main_block.data.data()), anal.main_block.data.size()))
    {
        std::cerr << "Failed to read data from input file \"" << args.input_filename << "\"" << std::endl;
        std::cerr << std::endl;

        return 3;
    }

    input.close();

    // Read segment table

    if (args.opt_segment_file)
    {
        std::string const file_name { *args.opt_segment_file };
        std::ifstream f(file_name);

        if (!f.is_open())
        {
            std::cerr << "Couldn't open file for read:" << std::endl;
            std::cerr << "  " << file_name << std::endl;
            std::cerr << std::endl;

            return 3;
        }

        try
        {
            Csv csv = Csv::from_stream(f);

            if (csv.field_names.size() != 4)
                throw CsvError("Bad CSV column count. (Expected 4)");

            for (Csv::Record const& record : csv.records)
            {
                Segment segment {};

                segment.name = record[0];
                segment.start = hex_decode<std::uint16_t>(record[1]);
                segment.size = hex_decode<std::uint16_t>(record[2]);

                for (char c : record[3])
                {
                    switch (c)
                    {

                    case 'w':
                        segment.flags |= Segment::FLAG_WRITE;
                        break;

                    case 'r':
                        segment.flags |= Segment::FLAG_READ;
                        break;

                    case 'x':
                        segment.flags |= Segment::FLAG_EXEC;
                        break;

                    default:
                        break;

                    }
                }

                anal.segments.push_back(std::move(segment));
            }
        }
        catch (CsvError const& e)
        {
            std::cerr << "Failed to parse segment table file \"" << file_name << "\":" << std::endl;
            std::cerr << "  " << e.what() << std::endl;
            std::cerr << std::endl;

            return 3;
        }
        catch (HexDecodeError const& e)
        {
            std::cerr << "Failed to parse segment table file \"" << file_name << "\":" << std::endl;
            std::cerr << "  Hex decode error: " << e.what() << std::endl;
            std::cerr << std::endl;

            return 3;
        }
    }

    // Read symbol table

    if (args.opt_symbol_file)
    {
        std::string const file_name { *args.opt_symbol_file };
        std::ifstream f(file_name);

        if (!f.is_open())
        {
            std::cerr << "Couldn't open file for read:" << std::endl;
            std::cerr << "  " << file_name << std::endl;
            std::cerr << std::endl;

            return 3;
        }

        try
        {
            Csv csv = Csv::from_stream(f);

            if (csv.field_names.size() != 3)
                throw CsvError("Bad CSV column count. (Expected 3)");

            for (Csv::Record const& record : csv.records)
            {
                Symbol symbol {};

                symbol.name = record[0];
                symbol.value = hex_decode<std::uint16_t>(record[1]);

                for (char c : record[2])
                {
                    switch (c)
                    {

                    case 'w':
                        symbol.flags |= Symbol::FLAG_WRITE;
                        break;

                    case 'r':
                        symbol.flags |= Symbol::FLAG_READ;
                        break;

                    case 'x':
                        symbol.flags |= Symbol::FLAG_EXEC;
                        break;

                    default:
                        break;

                    }
                }

                anal.symbols.push_back(std::move(symbol));
            }
        }
        catch (CsvError const& e)
        {
            std::cerr << "Failed to parse symbol table file \"" << file_name << "\":" << std::endl;
            std::cerr << "  " << e.what() << std::endl;
            std::cerr << std::endl;

            return 3;
        }
        catch (HexDecodeError const& e)
        {
            std::cerr << "Failed to parse symbol table file \"" << file_name << "\":" << std::endl;
            std::cerr << "  Hex decode error: " << e.what() << std::endl;
            std::cerr << std::endl;

            return 3;
        }
    }

    // Finish setting up anal

    anal.allow_brk = args.flag_brk;

    if (anal.segments.empty())
    {
        anal.segments.push_back({ { 0x0000, 0x8000 }, "ALL1", Segment::FLAG_READ | Segment::FLAG_WRITE | Segment::FLAG_EXEC });
        anal.segments.push_back({ { 0x8000, 0x8000 }, "ALL2", Segment::FLAG_READ | Segment::FLAG_WRITE | Segment::FLAG_EXEC });
    }

    if (anal.main_block.contains(0xFFFA) && anal.main_block.contains(0xFFFF))
    {
        char const* const vector_value_names[]
        {
            "ENTRY_NMI",
            "ENTRY_RESET",
            "ENTRY_IRQ",
        };

        for (std::size_t i = 0; i < 3; ++i)
        {
            std::uint8_t const lo = anal.main_block.data[0xFFFA - anal.main_block.address + 2*i + 0];
            std::uint8_t const hi = anal.main_block.data[0xFFFA - anal.main_block.address + 2*i + 1];

            std::uint16_t const val = lo | (hi << 8);

            anal.symbols.push_back({ vector_value_names[i], val, Symbol::FLAG_EXEC });
        }
    }

    std::sort(anal.symbols.begin(), anal.symbols.end(), Symbol::Compare {});

    std::vector<AddressBlock> const blocks = analyse_code_blocks(anal);

    std::vector<Symbol> const new_symbols = build_symbols(anal, blocks, args.flag_auto_symbols);
    std::vector<Symbol> const symbols = merge_sorted_vectors(anal.symbols, new_symbols);

    std::vector<PrintItem> const print = gen_print_items(anal.main_block, blocks, symbols);

    const auto do_print = [&] (std::ostream& output)
    {
        print_symbols(anal.main_block, args.flag_print_input_symbols ? symbols : new_symbols, output);
        print_items(anal.main_block, print, symbols, output);
    };

    if (args.opt_output_file)
    {
        std::string const file_name { *args.opt_output_file };
        std::ofstream output(file_name);

        if (!output.is_open())
        {
            std::cerr << "Couldn't open file for write:" << std::endl;
            std::cerr << "  " << file_name << std::endl;
            std::cerr << std::endl;

            return 3;
        }

        do_print(output);
    }
    else
    {
        do_print(std::cout);
    }

    return 0;
}
