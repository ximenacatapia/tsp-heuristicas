#include "instance.hpp"

#include <algorithm>
#include <functional>
#include <stdexcept>

#include "distance.hpp"

/* Loads the cities, builds the index and the matrix, computes the constants. */
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

/*
 * Fills the k x k weight matrix. First the real edges (their natural distance,
 * marked in real_), then max_distance, then the missing cells with the
 * augmented weight natural * max_distance. The diagonal stays 0.
 */
void Instance::build_matrix(const std::vector<Connection> &connections)
{
    const std::size_t k = cities_.size();
    matrix_.assign(k * k, 0.0);
    real_.assign(k * k, 0);

    //  the real edges from the database (with city1 < city2).
    for (const Connection &e : connections)
    {
        std::size_t i = index_of_.at(e.c1);
        std::size_t j = index_of_.at(e.c2);
        double nat = distance::natural(cities_[i], cities_[j]);
        matrix_[i * k + j] = nat;
        matrix_[j * k + i] = nat;
        real_[i * k + j] = 1;
        real_[j * k + i] = 1;
    }

    // max_distance over the real edges only
    max_distance_ = 0.0;
    for (std::size_t x = 0; x < matrix_.size(); ++x)
    {
        if (real_[x] && matrix_[x] > max_distance_)
            max_distance_ = matrix_[x];
    }

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

/*
 * Computes max_distance and the normalizer N(S): the sum of the k-1 heaviest
 * real edges (all of them if fewer than k-1). Throws if no edge exists.
 */
void Instance::compute_max_and_normalizer()
{
    const std::size_t k = cities_.size();

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

    std::size_t take = std::min<std::size_t>(k - 1, weights.size());
    std::partial_sort(weights.begin(), weights.begin() + take, weights.end(),
                      std::greater<double>());

    // Summed from heaviest to lightest, the order the list is in.
    normalizer_ = 0.0;
    for (std::size_t i = 0; i < take; ++i)
        normalizer_ += weights[i];
}

/* Cost of a tour: sum of augmented weights of its edges, over the normalizer. */
double Instance::evaluate(const std::vector<std::size_t> &tour) const
{
    double sum = 0.0;
    for (std::size_t i = 1; i < tour.size(); ++i)
    {
        sum += weight(tour[i - 1], tour[i]);
    }
    return sum / normalizer_;
}

/* Feasible iff every consecutive pair in the tour is a real edge of E. */
bool Instance::is_feasible(const std::vector<std::size_t> &tour) const
{
    for (std::size_t i = 1; i < tour.size(); ++i)
    {
        if (!connected(tour[i - 1], tour[i]))
            return false;
    }
    return true;
}