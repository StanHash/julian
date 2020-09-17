
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

struct Args
{
    std::string_view input_filename;
    std::size_t input_offset;
    std::size_t input_size;

    std::uint32_t base_address;

    std::optional<std::string_view> opt_output_file;
    std::optional<std::string_view> opt_segment_file;
    std::optional<std::string_view> opt_symbol_file;

    bool flag_brk : 1;
    bool flag_auto_symbols : 1;
    bool flag_print_input_symbols : 1;
};

Args parse_args(int argc, char** argv);
