#include <cstdio>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "instance.hpp"
#include "reader.hpp"
#include "database.hpp"
#include "heuristic/threshold_accepting.hpp"

namespace
{

    const char *kDefaultDatabase = "tsp.db";
    const char *kEnvVariable = "TSP_DB";

    struct Options
    {
        std::string tsp_path;
        std::string db_path;
        bool run = false;
        bool verbose = false;
        bool csv = false;
        std::uint64_t seed = 0;
        Parameters params;
    };

    void uso(const char *program)
    {
        std::cerr << "Usage: " << program << " [options] file.tsp\n"
                  << "\n"
                  << "  -r           run the heuristic (default: just evaluate the "
                     "file's tour)\n"
                  << "  -v           verbose output\n"
                  << "  --csv        with -r, print one parseable line\n"
                  << "  --csv-header print the CSV header and exit\n"
                  << "  -s N         seed (default 0)\n"
                  << "  -L N         batch size (default 4000)\n"
                  << "  -p F         cooling factor phi (default 0.95)\n"
                  << "  -e F         epsilon (default 0.0001)\n"
                  << "  -P F         target acceptance P (default 0.9)\n"
                  << "  -d FILE      database (default " << kDefaultDatabase
                  << ", or TSP_DB)\n";
    }

    // returns the path without its directory
    std::string base_name(const std::string &path)
    {
        std::size_t slash = path.find_last_of("/\\");
        return (slash == std::string::npos) ? path : path.substr(slash + 1);
    }

    // The CSV columns stay in one column, (may change later)
    const char *kCsvHeader =
        "instance,seed,batch_size,cooling,epsilon,accept_percentage,"
        "initial_temperature,cost,feasible";

    // Parses argv into Options. Returns false if the arguments are invalid; sets
    // `done` to true if the program should exit successfully without running
    bool parse(int argc, char *argv[], Options &opt, bool &done)
    {
        done = false;
        std::vector<std::string> positional;

        for (int i = 1; i < argc; ++i)
        {
            std::string a = argv[i];
            auto next = [&](const char *flag) -> std::string
            {
                if (i + 1 >= argc)
                {
                    std::cerr << "Error: " << flag << " needs a value.\n";
                    return std::string();
                }
                return argv[++i];
            };

            if (a == "-r")
            {
                opt.run = true;
            }
            else if (a == "-v")
            {
                opt.verbose = true;
            }
            else if (a == "--csv")
            {
                opt.csv = true;
            }
            else if (a == "--csv-header")
            {
                std::printf("%s\n", kCsvHeader);
                done = true;
                return true;
            }
            else if (a == "-s")
            {
                opt.seed = std::strtoull(next("-s").c_str(), nullptr, 10);
            }
            else if (a == "-L")
            {
                opt.params.batch_size =
                    std::strtoull(next("-L").c_str(), nullptr, 10);
            }
            else if (a == "-p")
            {
                opt.params.cooling = std::strtod(next("-p").c_str(), nullptr);
            }
            else if (a == "-e")
            {
                opt.params.epsilon = std::strtod(next("-e").c_str(), nullptr);
            }
            else if (a == "-P")
            {
                opt.params.accept_percentage =
                    std::strtod(next("-P").c_str(), nullptr);
            }
            else if (a == "-d")
            {
                opt.db_path = next("-d");
            }
            else if (!a.empty() && a[0] == '-')
            {
                std::cerr << "Error: unknown option '" << a << "'.\n";
                return false;
            }
            else
            {
                positional.push_back(a);
            }
        }

        if (positional.size() != 1)
        {
            return false;
        }
        opt.tsp_path = positional[0];

        if (opt.db_path.empty())
        {
            const char *from_env = std::getenv(kEnvVariable);
            opt.db_path = (from_env != nullptr) ? from_env : kDefaultDatabase;
        }
        return true;
    }

    // Default mode: evaluate the tour as given in the file.
    void report_file_tour(const Instance &instance, const std::string &tsp_path,
                          const std::vector<int> &ids, bool verbose)
    {
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
        std::printf("  Feasible: %s\n", instance.is_feasible(tour) ? "YES" : "NO");
        (void)verbose; // breakdown omitted here for brevity
    }

    // -r mode: run the heuristic and report the best solution.
    void report_run(const Instance &instance, const Options &opt,
                    const std::vector<int> &ids)
    {
        ThresholdAccepting ta(instance, opt.params);
        Result r = ta.run(opt.seed);

        if (opt.csv)
        {
            // instance,seed,batch_size,cooling,epsilon,accept_percentage,
            // initial_temperature,cost,feasible
            std::printf("%s,%llu,%zu,%g,%g,%g,%.9f,%.9f,%d\n",
                        base_name(opt.tsp_path).c_str(),
                        (unsigned long long)opt.seed, opt.params.batch_size,
                        opt.params.cooling, opt.params.epsilon,
                        opt.params.accept_percentage, r.initial_temperature,
                        r.cost, r.feasible ? 1 : 0);
            return;
        }

        std::printf("  Filename: %s\n", base_name(opt.tsp_path).c_str());
        std::printf("      Seed: %llu\n", (unsigned long long)opt.seed);
        std::printf("   Initial T: %.9f\n", r.initial_temperature);
        std::printf("        Cost: %.9f\n", r.cost);
        std::printf("    Feasible: %s\n", r.feasible ? "YES" : "NO");

        if (opt.verbose)
        {
            std::string path;
            for (std::size_t i = 0; i < r.solution.tour().size(); ++i)
            {
                if (i > 0)
                    path += ',';
                path += std::to_string(ids[r.solution.tour()[i]]);
            }
            std::printf("        Tour: %s\n", path.c_str());
        }
    }
}

//
int main(int argc, char *argv[])
{
    Options opt;
    bool done = false;
    if (!parse(argc, argv, opt, done))
    {
        uso(argv[0]);
        return 1;
    }
    if (done)
        return 0;

    try
    {
        std::vector<int> ids = read_instance(opt.tsp_path);
        Database db(opt.db_path);
        Instance instance(ids, db);

        if (opt.run)
        {
            report_run(instance, opt, ids);
        }
        else
        {
            report_file_tour(instance, opt.tsp_path, ids, opt.verbose);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}