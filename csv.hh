
#pragma once

#include <vector>
#include <string>
#include <ios>
#include <stdexcept>

struct CsvError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct Csv
{
    using Record = std::vector<std::string>;

    static Csv from_stream(std::istream& input);

    Record field_names;
    std::vector<Record> records;
};
