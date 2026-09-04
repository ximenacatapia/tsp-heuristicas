// lee una instancia de TSP y muestra las ciudades que la componen, tomadas de la base de datos.
// Por ahora sólo funciona la lectura.
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "instance.hpp"
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

//
int main(int argc, char *argv[])
{

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

        // construimos la fmatriz de costo, max_distance y el normalizador solo una vez
        Instance instance(ids, db);

        // The reference tour is the instance in the given order, i.e. indices
        // 0..k-1. Later this comes from the heuristic; for now it lets the
        // report reproduce the same numbers as the reference analyzer.
        std::vector<std::size_t> tour(ids.size());
        for (std::size_t i = 0; i < ids.size(); ++i)
            tour[i] = i;

        std::string path;
        for (std::size_t i = 0; i < ids.size(); ++i)
        {
            if (i > 0)
                path += ',';
            path += std::to_string(ids[i]);
        }
        std::printf("  Filename: %s\n", base_name(tsp_path).c_str());
        std::printf("      Path: %s\n", path.c_str());
        std::printf("   Maximum: %.9f\n", instance.max_distance());
        std::printf("Normalizer: %.9f\n", instance.normalizer());
        std::printf("Evaluation: %.9f\n", instance.evaluate(tour));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}