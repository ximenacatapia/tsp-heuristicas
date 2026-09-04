#include "instance.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>

#include "distance.hpp"

Instance::Instance(const std::vector<int> &ids, const Database &db)
{
    // Cities in the order given, index_of_ maps each original id back to its position.
    cities_ = db.get_cities(ids);
    for (std::size_t i = 0; i < cities_.size(); ++i)
    {
        index_of_[cities_[i].id] = i;
    }

    build_matrix(db.get_connections(ids));
    compute_max_and_normalizer();
}

void Instance::build_matrix(const std::vector<Connection> &connections)
{
    const std::size_t k = cities_.size();
    matrix_.assign(k * k, 0.0);
    real_.assign(k * k, 0);

    // First round. the real edges from the database (with city1 < city2).
    for (const Connection &e : connections)
    {
        std::size_t i = index_of_.at(e.c1);
        std::size_t j = index_of_.at(e.c2);
        matrix_[i * k + j] = e.distance;
        matrix_[j * k + i] = e.distance;
        real_[i * k + j] = 1;
        real_[j * k + i] = 1;
    }

    // max_distance is needed to fill the non-real cells, so it is found here,
    // over the real edges only (definition 4.1.2).
    max_distance_ = 0.0;
    for (std::size_t x = 0; x < matrix_.size(); ++x)
    {
        if (real_[x] && matrix_[x] > max_distance_)
            max_distance_ = matrix_[x];
    }

    // Second round: the cells with no real edge get the augmented weight natural(u, v) *
    // max_distance (definition 4.1.1), a deliberately large, the diagonal stays 0.
    for (std::size_t i = 0; i < k; ++i)
    {
        for (std::size_t j = 0; j < k; ++j)
        {
            if (i == j || real_[i * k + j])
                continue;
            double nat = distance::natural(cities_[i], cities_[j]);
            matrix_[i * k + j] = nat * max_distance_;
        }
    }
}

void Instance::compute_max_and_normalizer()
{
    const std::size_t k = cities_.size();

    // Collect the real edge weights, upper triangle only so each edge counts
    // once (the normalizer sums edges, not matrix cells).
    std::vector<double> weights;
    weights.reserve(k * (k - 1) / 2);
    for (std::size_t i = 0; i < k; ++i)
    {
        for (std::size_t j = i + 1; j < k; ++j)
        {
            if (real_[i * k + j])
                weights.push_back(matrix_[i * k + j]);
        }
    }

    if (weights.empty())
    {
        throw std::runtime_error(
            "No two cities in the instance are connected: max_distance and "
            "the normalizer are undefined.");
    }

    // max_distance was already found in build_matrix
    max_distance_ = *std::max_element(weights.begin(), weights.end());

    // N(S): sum of the k-1 heaviest real edges. If there are fewer than k-1,
    // take them all
    std::size_t take = std::min<std::size_t>(k - 1, weights.size());
    std::partial_sort(weights.begin(), weights.begin() + take, weights.end(),
                      std::greater<double>());

    // Summed from heaviest to lightest, the order the list is in.
    normalizer_ = 0.0;
    for (std::size_t i = 0; i < take; ++i)
        normalizer_ += weights[i];
}

double Instance::evaluate(const std::vector<std::size_t> &tour) const
{
    // Sum the augmented weight of each consecutive pair. The tour is a path,
    // not a cycle: it does not return from the last city to the first, so the
    // loop runs from the second element (definition 4.3.2).
    double sum = 0.0;
    for (std::size_t i = 1; i < tour.size(); ++i)
    {
        sum += weight(tour[i - 1], tour[i]);
    }
    return sum / normalizer_;
}

bool Instance::is_feasible(const std::vector<std::size_t> &tour) const
{
    // Feasible iff every consecutive pair is a real edge of E.
    for (std::size_t i = 1; i < tour.size(); ++i)
    {
        if (!connected(tour[i - 1], tour[i]))
            return false;
    }
    return true;
}