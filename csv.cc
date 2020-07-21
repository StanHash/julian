
#include "csv.hh"

#include <sstream>

Csv Csv::from_stream(std::istream& input)
{
    Csv result;

    auto const read_record = [&] () -> Record
    {
        Record result;

        std::string line;
        std::getline(input, line, '\n');

        if (line.empty())
            return {};

        std::stringstream ss(line);
        std::string field;

        while (std::getline(ss, field, ','))
            result.push_back(std::move(field));

        return result;
    };

    result.field_names = read_record();

    while (input.good())
    {
        Record record = read_record();

        if (record.size())
        {
            if (record.size() != result.field_names.size())
                throw CsvError("A record has an incorrect number of elements!");

            result.records.push_back(std::move(record));
        }
    }

    return result;
}
