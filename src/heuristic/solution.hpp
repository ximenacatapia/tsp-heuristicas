#ifndef HEURISTIC_SOLUTION_HPP
#define HEURISTIC_SOLUTION_HPP

#include <cstddef>
#include <vector>

#include "../instance.hpp"
#include "random.hpp"

/*
 * A candidate solution: a tour (permutation of 0..k-1) and its cached cost.
 * Neighbors are made by swapping two positions; the swap updates the cost
 * incrementally and can be undone, which is what the heuristic relies on.
 */
class Solution
{
public:
    // A random permutation of 0..k-1, evaluated once. This is the initial
    // tour the heuristic starts from.
    Solution(const Instance &instance, Random &rng);

    // A given tour, must be a permutation of 0..k-1.
    Solution(const Instance &instance, std::vector<std::size_t> tour);

    double cost() const { return cost_; }
    bool is_feasible() const { return instance_->is_feasible(tour_); }
    const std::vector<std::size_t> &tour() const { return tour_; }
    std::size_t size() const { return tour_.size(); }

    void swap_positions(std::size_t i, std::size_t j);

    void random_neighbor(Random &rng);

    void undo();

private:
    // Sum of the tour edges touching position p (used by the incremental swap).
    double edges_around(std::size_t p) const;

    const Instance *instance_;
    std::vector<std::size_t> tour_;
    double cost_ = 0.0;

    // State for undo: the positions of the last swap and the cost before it.
    std::size_t last_i_ = 0;
    std::size_t last_j_ = 0;
    double cost_before_ = 0.0;
    bool can_undo_ = false;

    static constexpr std::size_t kResyncEvery = 10000;
    std::size_t swaps_since_resync_ = 0;
};

#endif // HEURISTIC_SOLUTION_HPP