#include "threshold_accepting.hpp"

#include <algorithm>
#include <limits>

/* Precomputes the effective caps, filling in defaults derived from L. */
ThresholdAccepting::ThresholdAccepting(const Instance &instance,
                                       Parameters params)
    : instance_(instance), params_(params)
{
    effective_max_tries_ =
        params_.max_tries != 0 ? params_.max_tries : params_.batch_size * 100;

    effective_samples_ = params_.percentage_samples != 0
                             ? params_.percentage_samples
                             : params_.batch_size;
}

/* Samples neighbors at temperature t and returns the fraction accepted. */
double ThresholdAccepting::accepted_fraction(Solution &s, double t,
                                             Random &rng) const
{
    std::size_t accepted = 0;
    for (std::size_t i = 0; i < effective_samples_; ++i)
    {
        double before = s.cost();
        s.random_neighbor(rng);
        if (s.cost() <= before + t)
        {
            accepted++; // keep the neighbor, s already moved
        }
        else
        {
            s.undo(); // reject, go back
        }
    }
    return static_cast<double>(accepted) / effective_samples_;
}

/*
 * Binary-searches a starting temperature that accepts about accept_percentage:
 * bracket the target by doubling/halving, then bisect.
 */
double ThresholdAccepting::initial_temperature(Solution &s, Random &rng) const
{

    double t = 1.0;
    double p = accepted_fraction(s, t, rng);
    const double target = params_.accept_percentage;
    const double eps = params_.temperature_epsilon;

    if (std::abs(target - p) <= eps)
        return t;

    double t1, t2;
    if (p < target)
    {
        while (p < target)
        {
            t *= 2.0;
            p = accepted_fraction(s, t, rng);
        }
        t1 = t / 2.0;
        t2 = t;
    }
    else
    {
        while (p > target)
        {
            t /= 2.0;
            p = accepted_fraction(s, t, rng);
        }
        t1 = t;
        t2 = t * 2.0;
    }

    while (t2 - t1 > eps)
    {
        double mid = (t1 + t2) / 2.0;
        p = accepted_fraction(s, mid, rng);
        if (std::abs(target - p) < eps)
            return mid;
        if (p > target)
        {
            t2 = mid;
        }
        else
        {
            t1 = mid;
        }
    }
    return (t1 + t2) / 2.0;
}

/*
 * Runs one batch: accepts neighbors under the threshold rule until batch_size
 * pass or max_tries is hit. Tracks the best seen, accumulates statistics, and
 * traces accepted costs. Returns the average cost of the accepted solutions.
 */
double ThresholdAccepting::compute_batch(Solution &s, double t, Solution &best,
                                         Random &rng, Result &stats,
                                         std::ostream *trace) const
{
    std::size_t accepted = 0;
    std::size_t tries = 0;
    double sum = 0.0;

    while (accepted < params_.batch_size && tries < effective_max_tries_)
    {
        tries++;
        double before = s.cost();
        s.random_neighbor(rng);

        if (s.cost() <= before + t)
        {
            accepted++;
            sum += s.cost();

            if (s.cost() < best.cost())
                best = s;

            if (trace)
            {
                stats.accepted_total++;
                if (stats.accepted_total % kTraceEvery == 0)
                    *trace << stats.accepted_total << ',' << s.cost() << '\n';
            }
        }
        else
        {
            s.undo();
        }
    }

    // Accumulate the run-wide statistics.
    stats.attempts += tries;
    stats.accepted += accepted;
    stats.batches += 1;
    if (accepted < params_.batch_size)
        stats.batches_cut += 1;

    // Average of the accepted solutions
    return accepted > 0 ? sum / accepted : s.cost();
}

/*
 * Main loop: run batches at each temperature until they stop improving, cool
 * by phi, and reheat if the escape clause is on and the best has stalled.
 * Returns the best solution found.
 */
Result ThresholdAccepting::run(std::uint64_t seed, std::ostream *trace) const
{
    Random rng(seed);

    Solution current(instance_, rng);
    Solution best = current;

    double t = initial_temperature(current, rng);
    double settled_t = t;

    Result result{best, best.cost(), best.is_feasible(), settled_t};

    if (trace)
        *trace << "accepted,cost\n";

    // Escape clause , track how long best has gone without improving.
    double best_before = best.cost();
    std::size_t stalled = 0;

    while (t > params_.epsilon)
    {
        double previous = std::numeric_limits<double>::infinity();
        double average = compute_batch(current, t, best, rng, result, trace);

        while (average < previous)
        {
            previous = average;
            average = compute_batch(current, t, best, rng, result);
        }

        // Escape clause: did best improve during this temperature step?
        if (params_.stall_batches != 0)
        {
            if (best.cost() < best_before)
            {
                best_before = best.cost(); // progress, reset the stall count
                stalled = 0;
            }
            else
            {
                stalled++;
            }

            if (stalled >= params_.stall_batches &&
                result.reheats < params_.max_reheats)
            {
                t = settled_t * params_.reheat_fraction;
                stalled = 0;
                result.reheats++;
                continue; // skip the cooling step for this iteration
            }
        }

        t *= params_.cooling;
    }

    // Mostly-cut batches mean attempts, not temperature, ended the run.
    if (result.batches_cut > result.batches / 2)
    {
        result.stop_reason = StopReason::kAttemptsExhausted;
    }

    result.solution = best;
    result.cost = best.cost();
    result.feasible = best.is_feasible();
    return result;
}
