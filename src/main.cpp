// lee una instancia de TSP y muestra las ciudades que la componen, tomadas de la base de datos.
// Por ahora sólo funciona la lectura.
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "database.hpp"
#include "city.hpp"
#include "reader.hpp"

namespace
{

    const char *kDefaultDatabase = "tsp.db";
    const char *kEnvVariable = "TSP_DB";

    void uso(const char *program)
    {
        std::cerr << "Usage: " << program << " file.tsp [database.db]\n"
                  << "\n"
                  << "  file.tsp     instance to process\n"
                  << "  database.db  path to the database\n"
                  << "               (or the " << kEnvVariable
                  << " environment variable; default: " << kDefaultDatabase
                  << ")\n";
    }

    // returns the path without its directory
    std::string base_name(const std::string &path)
    {
        std::size_t slash = path.find_last_of("/\\");
        return (slash == std::string::npos) ? path : path.substr(slash + 1);
    }
}

// whattt
int main(int argc, char *argv[])
{

    // whatttmain
    if (argc < 2 || argc > 3)
    {
        uso(argv[0]);
        return 1;
    }

    const std::string tsp_path = argv[1];

    std::string db_path;
    if (argc == 3)
    {
        db_path = argv[2];
    }
    else
    {
        const char *from_env = std::getenv(kEnvVariable);
        db_path = (from_env != nullptr) ? from_env : kDefaultDatabase;
    }

    try
    {
        // The order of the ids is the tour: it must not be sorted.
        std::vector<int> ids = read_instance(tsp_path);
        // The destructor closes the connection when the block is left.
        Database db(db_path);

        // cities[i] corresponds to ids[i].
        std::vector<City> cities = db.get_cities(ids);

        std::printf("  Filename: %s\n", base_name(tsp_path).c_str());
        std::printf("    Cities: %zu\n\n", cities.size());

        std::printf("%6s  %-22s %-22s %12s %12s\n", "id", "name", "country",
                    "latitude", "longitude");
        std::printf(
            "--------------------------------------------------"
            "----------------------------------\n");

        for (const City &c : cities)
        {
            std::printf("%6d  %-22s %-22s %12.4f %12.4f\n", c.id,
                        c.name.c_str(), c.country.c_str(), c.latitude,
                        c.longitude);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}