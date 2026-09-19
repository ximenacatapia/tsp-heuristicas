#ifndef HEURISTIC_RANDOM_HPP
#define HEURISTIC_RANDOM_HPP

#include <cstddef>
#include <cstdint>
#include <random>

/*
 * Seeded random-number generator. Wraps std::mt19937 so nothing uses a global
 * rand(); the same seed reproduces the same run, which is what makes the
 * experiments reproducible and comparable.
 */
class Random
{
public:
    explicit Random(std::uint64_t seed) : engine_(seed) {}

    // A random integer in [0, n). Used to pick a position in the tour.
    std::size_t index(std::size_t n)
    {
        std::uniform_int_distribution<std::size_t> dist(0, n - 1);
        return dist(engine_);
    }
    double real()
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(engine_);
    }

    std::mt19937 &engine() { return engine_; }

private:
    std::mt19937 engine_;
};

#endif // HEURISTIC_RANDOM_HPP