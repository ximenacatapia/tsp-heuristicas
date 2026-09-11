#include "solution.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

Solution::Solution(const Instance &instance, Random &rng)
    : instance_(&instance), tour_(instance.size())
{
    // Fill with 0, 1, ..., k-1, then shuffle into a random permutation. This
    // is the initial tour.
    std::iota(tour_.begin(), tour_.end(), 0);
    std::shuffle(tour_.begin(), tour_.end(), rng.engine());
    cost_ = instance_->evaluate(tour_);
}

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

void Solution::swap_positions(std::size_t i, std::size_t j)
{
    // Remember enough to undo: the two positions and the cost before the move.
    last_i_ = i;
    last_j_ = j;
    cost_before_ = cost_;
    can_undo_ = true;

    std::swap(tour_[i], tour_[j]);
    cost_ = instance_->evaluate(tour_);
}

void Solution::random_neighbor(Random &rng)
{
    // Pick two distinct positions. With k >= 2 this loop ends quickly; the
    // number of possible neighbors is the number of pairs, k*(k-1)/2.
    std::size_t i = rng.index(tour_.size());
    std::size_t j = rng.index(tour_.size());
    while (j == i)
        j = rng.index(tour_.size());

    swap_positions(i, j);
}

void Solution::undo()
{
    if (!can_undo_)
        return;

    // Undoing a swap is the same swap again; the cached cost is restored
    // directly rather than recomputed.
    std::swap(tour_[last_i_], tour_[last_j_]);
    cost_ = cost_before_;
    can_undo_ = false;
}