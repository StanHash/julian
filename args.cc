
#include "args.hh"

#include "common.hh"

#include <argp.h>

char const* /* const */ argp_program_version     = "julian " JULIAN_VERSION_STRING;
char const* /* const */ argp_program_bug_address = "https://github.com/StanHash/julian/issues";

static char const julian_argp_arg[] = "INPUT[:OFFSET:SIZE] ADDRESS";
static char const julian_argp_doc[] = "Disassemble 6502 from pure data";

static argp_option julian_argp_options[] =
{
    { "output",   'o', "<output>",       0, "output file [default: stdout]", 0 },
    { "segments", 'm', "<segments.csv>", 0, "input segment table", 0 },
    { "symbols",  's', "<symbols.csv>",  0, "input symbol table", 0 },

    { nullptr,    'f', "<flag>",         0, "set a flag. flags:", 2 },
    { "  brk",                 0, nullptr, OPTION_DOC, "allow BRK instructions to be analysed", 2 },
    { "  auto-symbols",        0, nullptr, OPTION_DOC, "generate symbols for all addresses", 2 },
    { "  print-input-symbols", 0, nullptr, OPTION_DOC, "print input symbols alongside analysed ones", 2 },

    {},
};

static void julian_argp_parse_positional(Args& args, int num, std::string_view const& arg, argp_state* st)
{
    switch (num)
    {

    case 0:
    {
        // Input positional

        std::size_t const colon_pos = arg.find_first_of(':');

        args.input_filename = (colon_pos != std::string_view::npos)
            ? arg.substr(0, colon_pos) : arg;

        args.input_offset = 0;
        args.input_size = 0;

        if (colon_pos != std::string_view::npos)
        {
            std::string_view input_range = arg.substr(colon_pos+1);
            std::size_t const range_colon_pos = input_range.find_first_of(':');

            if (range_colon_pos == std::string_view::npos)
            {
                std::string const input_range_str { input_range };
                argp_error(st, "Input range (\"%s\") requires 2 components (offset:size)", input_range_str.c_str());
                return;
            }

            std::string_view offset_view = input_range.substr(0, range_colon_pos);
            std::string_view size_view = input_range.substr(range_colon_pos + 1);

            try { args.input_offset = hex_decode<std::size_t>(offset_view); }
            catch (HexDecodeError const& e)
            {
                std::string const offset_str { offset_view };
                argp_failure(st, 2, 0, "Couldn't parse range offset (%s): %s", offset_str.c_str(), e.what());
                return;
            }

            try { args.input_size = hex_decode<std::size_t>(size_view); }
            catch (HexDecodeError const& e)
            {
                std::string const size_str { size_view };
                argp_failure(st, 2, 0, "Couldn't parse range size (%s): %s", size_str.c_str(), e.what());
                return;
            }
        }

        break;
    }

    case 1:
    {
        try { args.base_address = hex_decode(arg); }
        catch (HexDecodeError const& e)
        {
            std::string const arg_str { arg };
            argp_failure(st, 2, 0, "Couldn't parse address (%s): %s", arg_str.c_str(), e.what());
            return;
        }

        break;
    }

    default:
        argp_usage(st);
        break;

    }
}

static error_t julian_argp_parser(int key, char* arg, argp_state* st)
{
    Args& args = *reinterpret_cast<Args*>(st->input);
    std::string_view const arg_view { arg ? arg : "" };

    switch (key)
    {

    case 'o':
        args.opt_output_file = arg_view;

        if (arg_view == "-")
            args.opt_output_file = std::nullopt;

        break;

    case 'm':
        args.opt_segment_file = arg_view;
        break;

    case 's':
        args.opt_symbol_file = arg_view;
        break;

    case 'f':
        if (arg_view == "brk")
            args.flag_brk = true;

        else if (arg_view == "auto-symbols")
            args.flag_auto_symbols = true;

        else if (arg_view == "print-input-symbols")
            args.flag_print_input_symbols = true;

        else
        {
            std::string const arg_str { arg_view };
            argp_error(st, "Unreckognized flag: %s", arg_str.c_str());
        }

        break;

    case ARGP_KEY_ARG:
        julian_argp_parse_positional(args, st->arg_num, arg_view, st);
        break;

    case ARGP_KEY_END:
        if (st->arg_num != 2)
            argp_usage(st);

        break;

    }

    return 0;
}

static argp const julian_argp =
{
    julian_argp_options,
    julian_argp_parser,
    julian_argp_arg,
    julian_argp_doc,
    nullptr,
    nullptr,
    nullptr,
};

Args parse_args(int argc, char** argv)
{
    Args result {};
    argp_parse(&julian_argp, argc, argv, 0, 0, &result);

    return result;
}
