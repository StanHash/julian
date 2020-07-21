
#include "args.hh"

Args parse_args(ArgsConfig const& config, int argc, char const* const* argv)
{
    Args result;

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg { argv[i] };

        if (arg.size() == 0)
            continue;

        if (arg[0] != '-' || arg.size() == 1)
        {
            result.positionals.push_back(arg);
            continue;
        }

        arg = arg.substr(1);

        if (config.aliases.count(arg))
            arg = config.aliases.at(arg);

        if (config.flag_keys.count(arg))
        {
            result.flags.insert(arg);
            continue;
        }

        if (config.option_keys.count(arg))
        {
            ++i;

            if (i >= argc)
            {
                std::string error_msg;

                error_msg.append("Reached end of arguments expecting parameter for \"");
                error_msg.append(arg);
                error_msg.append("\".");

                throw ArgError(std::move(error_msg));
            }

            std::string_view val { argv[i] };

            result.options[arg] = val;
            continue;
        }

        std::string error_msg;

        error_msg.append("Unknown flag/option \"");
        error_msg.append(arg);
        error_msg.append("\".");

        throw ArgError(error_msg);
    }

    return result;
}
