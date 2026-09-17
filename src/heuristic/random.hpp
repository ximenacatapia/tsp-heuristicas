#ifndef HEURISTIC_RANDOM_HPP
#define HEURISTIC_RANDOM_HPP

#include <cstddef>
#include <cstdint>
#include <random>

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

    // A random real in [0, 1). Used where a probability is needed.
    double real()
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(engine_);
    }

    // The underlying engine, for std::shuffle and the like.
    std::mt19937 &engine() { return engine_; }

private:
    std::mt19937 engine_;
};

#endif // HEURISTIC_RANDOM_HPP