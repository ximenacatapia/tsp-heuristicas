#ifndef HEURISTIC_THRESHOLD_ACCEPTING_HPP
#define HEURISTIC_THRESHOLD_ACCEPTING_HPP

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

#include "../instance.hpp"
#include "random.hpp"
#include "solution.hpp"

/*
 * Free parameters of the heuristic. No closed formula exists for them; they
 * are tuned by experiment.
 */
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

    // Escape clause (reheating)
    std::size_t stall_batches = 0; // batches without improvement to escape
    std::size_t max_reheats = 20;  // cap on how many times to reheat
    double reheat_fraction = 0.3;  // reset T to this fraction of initial T
};

/* Why a run ended. */
enum class StopReason
{
    kTemperatureReached, // T dropped below epsilon (the normal end)
    kAttemptsExhausted,  // batches kept hitting max_tries without filling
};

/* The outcome of a run, kept together so nothing leaks through globals. */
struct Result
{
    Solution solution;                    // the best solution found
    double cost = 0.0;                    // its cost (== solution.cost())
    bool feasible = false;                // whether it is feasible
    double initial_temperature = 114688.; // the T the search settled on

    // Acceptance statistics over the whole run.
    std::uint64_t attempts = 0;       // neighbors proposed in total
    std::uint64_t accepted = 0;       // of those, how many passed the threshold
    std::uint64_t batches = 0;        // batches computed
    std::uint64_t batches_cut = 0;    // batches that hit max_tries early
    std::uint64_t reheats = 0;        // times the escape clause fired
    std::uint64_t accepted_total = 0; // accepted solutions counted for tracing
    StopReason stop_reason = StopReason::kTemperatureReached;
};

/*
 * Threshold Accepting: from a random start, repeatedly accept a
 * neighbor whose cost is within a shrinking threshold T, cooling T by batches
 * until it reaches epsilon. Returns the best solution seen. Deterministic per
 * seed.
 */
class ThresholdAccepting
{
public:
    ThresholdAccepting(const Instance &instance, Parameters params);

    Result run(std::uint64_t seed, std::ostream *trace = nullptr) const;

    double initial_temperature(Solution &s, Random &rng) const;

private:
        double accepted_fraction(Solution &s, double t, Random &rng) const;

    double compute_batch(Solution &s, double t, Solution &best, Random &rng,
                         Result &stats, std::ostream *trace = nullptr) const;

    static constexpr std::uint64_t kTraceEvery = 1000;

    const Instance &instance_;
    Parameters params_;
    std::size_t effective_max_tries_;
    std::size_t effective_samples_;
};

#endif // HEURISTIC_THRESHOLD_ACCEPTING_HPP