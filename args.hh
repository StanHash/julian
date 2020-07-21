
#pragma once

#include "common.hh"

#include <stdexcept>

#include <unordered_set>
#include <unordered_map>

struct ArgError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct ArgsConfig
{
    std::unordered_set<std::string_view> option_keys;
    std::unordered_set<std::string_view> flag_keys;
    std::unordered_map<std::string_view, std::string_view> aliases;
};

struct Args
{
    std::vector<std::string_view> positionals;
    std::unordered_map<std::string_view, std::string_view> options;
    std::unordered_set<std::string_view> flags;
};

Args parse_args(ArgsConfig const& config, int argc, char const* const* argv);
