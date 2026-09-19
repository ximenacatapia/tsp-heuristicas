#include "solution.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

/* Builds a random initial tour and evaluates it once. */
Solution::Solution(const Instance &instance, Random &rng)
    : instance_(&instance), tour_(instance.size())
{
    std::iota(tour_.begin(), tour_.end(), 0);
    std::shuffle(tour_.begin(), tour_.end(), rng.engine());
    cost_ = instance_->evaluate(tour_);
}

/* Builds a solution from a given tour, checking its length. */
Solution::Solution(const Instance &instance, std::vector<std::size_t> tour)
    : instance_(&instance), tour_(std::move(tour))
{
    if (tour_.size() != instance.size())
    {
        throw std::runtime_error(
            "The tour does not have the same number of cities as the "
            "instance.");
    }
    cost_ = instance_->evaluate(tour_);
}

/* Sum of the augmented weights of the tour edges touching position p. */
double Solution::edges_around(std::size_t p) const
{
    double s = 0.0;
    if (p > 0)
        s += instance_->weight(tour_[p - 1], tour_[p]);
    if (p + 1 < tour_.size())
        s += instance_->weight(tour_[p], tour_[p + 1]);
    return s;
}

/*
 * Swaps positions i and j and updates the cost incrementally: only the edges
 * touching i and j change, so subtract the old ones and add the new. Every
 * kResyncEvery swaps the cost is recomputed in full to bound floating-point
 * drift.
 */
void Solution::swap_positions(std::size_t i, std::size_t j)
{
    last_i_ = i;
    last_j_ = j;
    cost_before_ = cost_;
    can_undo_ = true;

    const double N = instance_->normalizer();

    if (i > j)
        std::swap(i, j); // ensure i < j to handle adjacency cleanly

    // old contribution of the edges around both positions.
    double removed = edges_around(i) + edges_around(j);

    if (j == i + 1)
        removed -= instance_->weight(tour_[i], tour_[j]);

    std::swap(tour_[i], tour_[j]);

    double added = edges_around(i) + edges_around(j);
    if (j == i + 1)
        added -= instance_->weight(tour_[i], tour_[j]);

    cost_ = (cost_ * N - removed + added) / N;

    if (++swaps_since_resync_ >= kResyncEvery)
    {
        cost_ = instance_->evaluate(tour_);
        swaps_since_resync_ = 0;
    }
}

/* Swaps two random distinct positions. */
void Solution::random_neighbor(Random &rng)
{

    std::size_t i = rng.index(tour_.size());
    std::size_t j = rng.index(tour_.size());
    while (j == i)
        j = rng.index(tour_.size());

    swap_positions(i, j);
}

/* Reverts the last swap */
void Solution::undo()
{
    if (!can_undo_)
        return;

    std::swap(tour_[last_i_], tour_[last_j_]);
    cost_ = cost_before_;
    can_undo_ = false;
}