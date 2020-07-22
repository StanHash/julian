
#include "common.hh"
#include "csv.hh"
#include "anal.hh"
#include "print.hh"
#include "args.hh"

#include <fstream>
#include <cstring>

constexpr char const* const KEY_OPTION_OUTPUT = "-output";
constexpr char const* const KEY_OPTION_SEGMENTS = "-segments";
constexpr char const* const KEY_OPTION_SYMBOLS = "-symbols";

constexpr char const* const KEY_OPTION_OUTPUT_SHORT = "o";
constexpr char const* const KEY_OPTION_SEGMENTS_SHORT = "m";
constexpr char const* const KEY_OPTION_SYMBOLS_SHORT = "s";

constexpr char const* const KEY_FLAG_BRK = "fbrk";
constexpr char const* const KEY_FLAG_AUTO_SYMBOLS = "fauto-symbols";
constexpr char const* const KEY_FLAG_PRINT_INPUT_SYMBOLS = "fprint-input-symbols";

void print_usage(std::ostream& out, std::string_view program_name)
{
    out << "Usage: " << program_name << " <binary> <address> [options]" << std::endl;
    out << std::endl;

    out << "Options:" << std::endl;
    out << "  -o, --output <output>" << std::endl;
    out << "      sets main output file. [default: stdout]" << std::endl;
    out << "  -m, --segments <segment.csv>" << std::endl;
    out << "      defines the input segment table." << std::endl;
    out << "  -s, --symbols <symbols.csv>" << std::endl;
    out << "      defines the input symbol table." << std::endl;
    out << std::endl;

    out << "Flags:" << std::endl;
    out << "  -fbrk" << std::endl;
    out << "      allow BRK instructions to be analysed." << std::endl;
    out << "  -fauto-symbols" << std::endl;
    out << "      generate symbols for all addresses." << std::endl;
    out << "  -fprint-input-symbols" << std::endl;
    out << "      print input symbols alongside analysed ones." << std::endl;
    out << std::endl;
}

int main(int argc, char** argv)
{
    if (argc == 1 || std::any_of(argv + 1, argv + argc, [] (char const* arg) { return std::strcmp(arg, "--help") == 0; }))
    {
        print_usage(std::cout, argv[0]);
        return 0;
    }

    AnalConfig anal {};

    std::optional<std::string> opt_output_file;

    bool flag_auto_symbols = false;
    bool flag_print_input_symbols = false;

    try
    {
        ArgsConfig const args_config
        {
            {
                // Options

                KEY_OPTION_OUTPUT,
                KEY_OPTION_SEGMENTS,
                KEY_OPTION_SYMBOLS,
            },
            {
                // Flags

                KEY_FLAG_BRK,
                KEY_FLAG_AUTO_SYMBOLS,
                KEY_FLAG_PRINT_INPUT_SYMBOLS,
            },
            {
                // Aliases

                { KEY_OPTION_OUTPUT_SHORT, KEY_OPTION_OUTPUT },
                { KEY_OPTION_SEGMENTS_SHORT, KEY_OPTION_SEGMENTS },
                { KEY_OPTION_SYMBOLS_SHORT, KEY_OPTION_SYMBOLS },
            },
        };

        Args const args = parse_args(args_config, argc, argv);

        if (args.positionals.size() != 2)
            throw ArgError("Wrong number of positionals. (Expected 2)");

        // Parse input

        std::size_t const colon_pos = args.positionals[0].find_first_of(':');

        std::string_view input_file = (colon_pos != std::string_view::npos)
            ? args.positionals[0].substr(0, colon_pos)
            : args.positionals[0];

        std::size_t input_offset = 0;
        std::size_t input_size = 0;

        if (colon_pos != std::string_view::npos)
        {
            std::string_view input_range = args.positionals[0].substr(colon_pos+1);
            std::size_t const range_colon_pos = input_range.find_first_of(':');

            if (range_colon_pos == std::string_view::npos)
            {
                std::cerr << "Error: input range (\"" << input_range << "\") requires 2 components (offset:size)" << std::endl;
                std::cerr << std::endl;

                print_usage(std::cerr, argv[0]);
                return 3;
            }

            std::string_view offset_str = input_range.substr(0, range_colon_pos);
            std::string_view size_str = input_range.substr(range_colon_pos + 1);

            try { input_offset = hex_decode<std::size_t>(offset_str); }
            catch (HexDecodeError const& e)
            {
                std::cerr << "An error occured during parsing of range offset string (" << offset_str << "):" << std::endl;
                std::cerr << "  " << e.what() << std::endl;
                std::cerr << std::endl;

                print_usage(std::cerr, argv[0]);
                return 4;
            }

            try { input_size = hex_decode<std::size_t>(size_str); }
            catch (HexDecodeError const& e)
            {
                std::cerr << "An error occured during parsing of range size string (" << size_str << "):" << std::endl;
                std::cerr << "  " << e.what() << std::endl;
                std::cerr << std::endl;

                print_usage(std::cerr, argv[0]);
                return 5;
            }
        }

        // Parse address

        std::uint16_t addr = 0;

        try { addr = hex_decode(args.positionals[1]); }
        catch (HexDecodeError const& e)
        {
            std::cerr << "An error occured during parsing of address string (" << args.positionals[1] << "):" << std::endl;
            std::cerr << "  " << e.what() << std::endl;
            std::cerr << std::endl;

            print_usage(std::cerr, argv[0]);
            return 2;
        }

        // Open input

        std::ifstream input(std::string { input_file }, std::ios::in | std::ios::binary);

        if (!input.is_open())
        {
            std::cerr << "Couldn't open input file \"" << input_file << "\"" << std::endl;
            std::cerr << std::endl;

            return 12;
        }

        if (input_size == 0)
        {
            input.seekg(0, std::ios::end);
            input_size = ((std::size_t) input.tellg()) - input_offset;
        }

        input.seekg(input_offset, std::ios::beg);

        anal.main_block.address = addr;

        anal.main_block.data.resize(input_size);

        if (!input.read(reinterpret_cast<char*>(anal.main_block.data.data()), anal.main_block.data.size()))
        {
            std::cerr << "Failed to read data from input file \"" << input_file << "\"" << std::endl;
            std::cerr << std::endl;

            return 13;
        }

        input.close();

        // Parse options

        if (args.options.count(KEY_OPTION_OUTPUT))
            opt_output_file = std::string { args.options.at(KEY_OPTION_OUTPUT) };

        if (args.options.count(KEY_OPTION_SEGMENTS))
        {
            std::string file_name { args.options.at(KEY_OPTION_SEGMENTS) };

            std::ifstream f(file_name);

            if (!f.is_open())
            {
                std::cerr << "Couldn't open file for read:" << std::endl;
                std::cerr << "  " << file_name << std::endl;
                std::cerr << std::endl;

                return 6;
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

                return 6;
            }
            catch (HexDecodeError const& e)
            {
                std::cerr << "Failed to parse segment table file \"" << file_name << "\":" << std::endl;
                std::cerr << "  Hex decode error: " << e.what() << std::endl;
                std::cerr << std::endl;

                return 7;
            }
        }

        if (args.options.count(KEY_OPTION_SYMBOLS))
        {
            std::string file_name { args.options.at(KEY_OPTION_SYMBOLS) };

            std::ifstream f(file_name);

            if (!f.is_open())
            {
                std::cerr << "Couldn't open file for read:" << std::endl;
                std::cerr << "  " << file_name << std::endl;
                std::cerr << std::endl;

                return 8;
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

                return 9;
            }
            catch (HexDecodeError const& e)
            {
                std::cerr << "Failed to parse symbol table file \"" << file_name << "\":" << std::endl;
                std::cerr << "  Hex decode error: " << e.what() << std::endl;
                std::cerr << std::endl;

                return 10;
            }
        }

        // Parse flags

        if (args.flags.count(KEY_FLAG_BRK))
            anal.allow_brk = true;

        if (args.flags.count(KEY_FLAG_AUTO_SYMBOLS))
            flag_auto_symbols = true;

        if (args.flags.count(KEY_FLAG_PRINT_INPUT_SYMBOLS))
            flag_print_input_symbols = true;
    }
    catch (ArgError const& e)
    {
        std::cerr << "An error occured during parsing of arguments:" << std::endl;
        std::cerr << "  " << e.what() << std::endl;
        std::cerr << std::endl;

        print_usage(std::cerr, argv[0]);
        return 1;
    }

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

    std::vector<Symbol> const new_symbols = build_symbols(anal, blocks, flag_auto_symbols);
    std::vector<Symbol> const symbols = merge_sorted_vectors(anal.symbols, new_symbols);

    std::vector<PrintItem> const print = gen_print_items(anal.main_block, blocks, symbols);

    const auto do_print = [&] (std::ostream& output)
    {
        print_symbols(anal.main_block, flag_print_input_symbols ? symbols : new_symbols, output);
        print_items(anal.main_block, print, symbols, output);
    };

    if (opt_output_file.has_value())
    {
        std::ofstream output(opt_output_file.value());

        if (!output.is_open())
        {
            std::cerr << "Couldn't open file for write:" << std::endl;
            std::cerr << "  " << opt_output_file.value() << std::endl;
            std::cerr << std::endl;

            return 11;
        }

        do_print(output);
    }
    else
    {
        do_print(std::cout);
    }

    return 0;
}
