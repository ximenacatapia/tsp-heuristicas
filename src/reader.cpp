#include "reader.hpp"

#include <cctype>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>

/*
 * Reads the file whole and pulls out every run of digits as a city id, in
 * order. Any non-digit is a separator, so commas, spaces or newlines all work.
 * Rejects repeats, and instances with fewer than two cities.
 */
std::vector<int> read_instance(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Couldn't open file:'" + path + "'.");
    }

    // Read the whole file into memory.
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    std::vector<int> ids;
    std::set<int> seen;

    std::size_t i = 0;
    while (i < content.size())
    {
        if (!std::isdigit(static_cast<unsigned char>(content[i])))
        {
            i++;
            continue;
        }

        // Consume all consecutive digits so multi-digit ids stay whole.
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