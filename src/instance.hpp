#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "city.hpp"
#include "database.hpp"

/* An instance S of the TSP, ready to be evaluated.

 The city ids are sparse (1, 2, 75, 163, ...), so they cannot index an array
 directly. Cities are reindexed to 0..k-1: position i in every internal
 array is the city whose original id is ids_[i]. That original id is kept so
 results can be printed back later. */
class Instance
{
public:
    /* Builds the instance from the city ids and the database. Throws if a
       city is missing or if no two cities are connected. */
    Instance(const std::vector<int> &ids, const Database &db);

    // Number of cities, |S|.
    std::size_t size() const { return cities_.size(); }

    // Original id at index i (0-based).
    int id_at(std::size_t i) const { return cities_[i].id; }

    /* Augmented weight w_S between indices i and j (definition 4.1.1): the
       real distance if the edge exists in the map, or natural * max_distance
    .  if it does not.*/
    double weight(std::size_t i, std::size_t j) const
    {
        return matrix_[i * cities_.size() + j];
    }

    // Whether the edge between indices i and j exists in the real map E.
    bool connected(std::size_t i, std::size_t j) const
    {
        return real_[i * cities_.size() + j];
    }

    // max_distance(S), (definition 4.1.2) the longest real edge among the cities of the instance.
    double max_distance() const { return max_distance_; }

    // N(S), the sum of the k-1 heaviest real edges (definition 4.3.1).
    double normalizer() const { return normalizer_; }

    // cost of a tour:the sum of the augmented weights along it, divided by the normalizer.
    double evaluate(const std::vector<std::size_t> &tour) const;

    // Whether every consecutive pair in the tour is a real edge of E. A tour
    // is feasible exactly when its cost is <= 1.
    bool is_feasible(const std::vector<std::size_t> &tour) const;

private:
    void build_matrix(const std::vector<Connection> &connections);
    void compute_max_and_normalizer();

    std::vector<City> cities_;                      // index -> city
    std::unordered_map<int, std::size_t> index_of_; // original id -> index
    std::vector<double> matrix_;                    // k x k augmented weights
    std::vector<char> real_;                        // k x k, veer si arista en E
    double max_distance_ = 0.0;
    double normalizer_ = 0.0;
};

#endif // INSTANCE_HPP