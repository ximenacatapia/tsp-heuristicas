#include "reader.hpp"

#include <cctype>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>

std::vector<int> read_instance(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Couldn't open file:'" + path + "'.");
    }

    // Se lee todo el archivo a memoria.
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    std::vector<int> ids;
    std::set<int> seen;

    // Se toma cualquier secuencia de dígitos como un identificador y se ignora todo lo demás.
    // the reader works whether the file has comas, spaces or new lines.
    std::size_t i = 0;
    while (i < content.size())
    {
        if (!std::isdigit(static_cast<unsigned char>(content[i])))
        {
            i++;
            continue;
        }

        int value = 0;
        while (i < content.size() &&
               std::isdigit(static_cast<unsigned char>(content[i])))
        {
            value = value * 10 + (content[i] - '0');
            i++;
        }

        if (seen.count(value) > 0)
        {
            throw std::runtime_error("City" + std::to_string(value) +
                                     " reapeted in '" + path + "'.");
        }

        seen.insert(value);
        ids.push_back(value);
    }

    if (ids.size() < 2)
    {
        throw std::runtime_error("Instance '" + path +
                                 "' has less than two cities.");
    }

    return ids;
}