#ifndef HEURISTIC_RANDOM_HPP
#define HEURISTIC_RANDOM_HPP

#include <cstddef>
#include <cstdint>
#include <random>

// A seeded random-number generator.
//
// Wraps std::mt19937 so the rest of the code never touches a global rand():
// the generator is an object passed where it is needed. The seed is the whole
// point of the experiments: the same seed reproduces the same run exactly, so
// a good result found with seed N can be recreated, and thousands of seeds can
// be compared on equal footing.
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