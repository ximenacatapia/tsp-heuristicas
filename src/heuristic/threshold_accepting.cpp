#include "threshold_accepting.hpp"

#include <algorithm>
#include <limits>

ThresholdAccepting::ThresholdAccepting(const Instance &instance,
                                       Parameters params)
    : instance_(instance), params_(params)
{
    effective_max_tries_ =
        params_.max_tries != 0 ? params_.max_tries : params_.batch_size * 100;

    // Neighbors sampled to measure the acceptance percentage. Defaults to the
    // batch size if not set.
    effective_samples_ = params_.percentage_samples != 0
                             ? params_.percentage_samples
                             : params_.batch_size;
}

double ThresholdAccepting::accepted_fraction(Solution &s, double t,
                                             Random &rng) const
{
    // sample N neighbors and count how many satisfy the accept rule
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

double ThresholdAccepting::compute_batch(Solution &s, double t, Solution &best,
                                         Random &rng, Result &stats) const
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
            // Track the best solution ever seen (the chapter omits this from
            // the pseudocode but it is what the run must return).
            if (s.cost() < best.cost())
                best = s;
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

    // Average of the accepted solutions; if none were accepted, report the
    // current cost so the caller sees no improvement and stops.
    return accepted > 0 ? sum / accepted : s.cost();
}

Result ThresholdAccepting::run(std::uint64_t seed, std::ostream *trace) const
{
    Random rng(seed);

    Solution current(instance_, rng);
    Solution best = current;

    double t = initial_temperature(current, rng);
    double settled_t = t;

    Result result{best, best.cost(), best.is_feasible(), settled_t};

    if (trace)
        *trace << "batch,temperature,current_cost,best_cost\n";
    std::size_t batch_index = 0;

    // Escape clause , track how long best has gone without improving.
    double best_before = best.cost();
    std::size_t stalled = 0;

    while (t > params_.epsilon)
    {
        double previous = std::numeric_limits<double>::infinity();
        double average = compute_batch(current, t, best, rng, result);
        if (trace)
        {
            *trace << batch_index++ << ',' << t << ',' << current.cost() << ','
                   << best.cost() << '\n';
        }

        // Keep running batches at this temperature while the average cost of
        // the accepted solutions is still decreasing.
        while (average < previous)
        {
            previous = average;
            average = compute_batch(current, t, best, rng, result);
            if (trace)
            {
                *trace << batch_index++ << ',' << t << ',' << current.cost()
                       << ',' << best.cost() << '\n';
            }
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

            // Stalled too long and still allowed to reheat: reset T high again
            // so the search can escape the local minimum it is stuck in.
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

    // The run ended because the temperature reached epsilon.
    if (result.batches_cut > result.batches / 2)
    {
        result.stop_reason = StopReason::kAttemptsExhausted;
    }

    // Fill in the final best solution and its cost/feasibility.
    result.solution = best;
    result.cost = best.cost();
    result.feasible = best.is_feasible();
    return result;
}
