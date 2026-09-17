#ifndef HEURISTIC_THRESHOLD_ACCEPTING_HPP
#define HEURISTIC_THRESHOLD_ACCEPTING_HPP

#include <cstddef>
#include <vector>

#include "../instance.hpp"
#include "random.hpp"
#include "solution.hpp"

struct Parameters
{
    std::size_t batch_size = 4000;      // L, accepted solutions per batch
    double cooling = 0.995;             // phi
    double epsilon = 0.00001;           // virtual zero for the temperature
    std::size_t max_tries = 400000;     // cap on attempts per batch
    double accept_percentage = 0.9;     // P, target acceptance for the initial
    double temperature_epsilon = 0.001; // virtual zero for that search
    std::size_t percentage_samples = 0; // N, neighbors sampled to measure the
                                        // acceptance percentage; 0 -> from L
};

// The outcome of a run, kept together so nothing leaks through globals.
struct Result
{
    Solution solution;                    // the best solution found
    double cost = 0.0;                    // its cost (== solution.cost())
    bool feasible = false;                // whether it is feasible
    double initial_temperature = 114688.; // the T the search settled on
};

class ThresholdAccepting
{
public:
    ThresholdAccepting(const Instance &instance, Parameters params);

    // Runs the heuristic with the given seed and returns the best solution.
    Result run(std::uint64_t seed) const;

    // Computes an initial temperature by binary search: a T for
    // which about `accept_percentage` of neighbors are accepted.
    double initial_temperature(Solution &s, Random &rng) const;

private:
    // Fraction of `percentage_samples` neighbors accepted at temperature T,
    // starting from s. Leaves s where it ends up.
    double accepted_fraction(Solution &s, double t, Random &rng) const;

    // One batch (procedure 1): keep proposing neighbors until `batch_size` are accepted (or `max_tries` is hit)
    double compute_batch(Solution &s, double t, Solution &best,
                         Random &rng) const;

    const Instance &instance_;
    Parameters params_;
    std::size_t effective_max_tries_;
    std::size_t effective_samples_;
};

#endif // HEURISTIC_THRESHOLD_ACCEPTING_HPP