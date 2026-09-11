#include "../src/heuristic/solution.hpp"

#include <algorithm>
#include <vector>

#include "../src/database.hpp"
#include "../src/heuristic/random.hpp"
#include "../src/instance.hpp"
#include "lib/doctest.h"
#include "temp_database.hpp"

namespace
{

    // Same toy square as the instance tests, cities 1-4, a ring plus one
    // diagonal, so tours can be reasoned about by hand.
    void populate(TempDatabase &db)
    {
        db.add_city(1, "A", "X", 100, 0.0, 0.0);
        db.add_city(2, "B", "X", 100, 0.0, 10.0);
        db.add_city(3, "C", "X", 100, 10.0, 10.0);
        db.add_city(4, "D", "X", 100, 10.0, 0.0);
        db.add_connection(1, 2, 1.0);
        db.add_connection(2, 3, 1.0);
        db.add_connection(3, 4, 1.0);
        db.add_connection(4, 1, 1.0);
        db.add_connection(1, 3, 1.0);
    }

}

TEST_CASE("a random solution is a valid permutation")
{
    TempDatabase temp("perm");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Random rng(12345);
    Solution s(inst, rng);

    // It must contain exactly the indices 0..k-1, each once.
    std::vector<std::size_t> sorted = s.tour();
    std::sort(sorted.begin(), sorted.end());
    CHECK(sorted == std::vector<std::size_t>{0, 1, 2, 3});
}

TEST_CASE("the same seed gives the same solution")
{
    // This is what makes experiments reproducible.
    TempDatabase temp("seed");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Random rng_a(999);
    Random rng_b(999);
    Solution a(inst, rng_a);
    Solution b(inst, rng_b);

    CHECK(a.tour() == b.tour());
    CHECK(a.cost() == doctest::Approx(b.cost()));
}

TEST_CASE("a swap changes the tour and updates the cost")
{
    TempDatabase temp("swap");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Solution s(inst, {0, 1, 2, 3});
    double before = s.cost();

    s.swap_positions(1, 2);

    // Positions 1 and 2 exchanged.
    CHECK(s.tour() == std::vector<std::size_t>{0, 2, 1, 3});

    // The cost must now match a fresh evaluation of the new tour.
    CHECK(s.cost() == doctest::Approx(inst.evaluate({0, 2, 1, 3})));
    // And it must have actually changed (these two tours differ in cost).
    CHECK(s.cost() != doctest::Approx(before));
}

TEST_CASE("undo restores the tour and the cost exactly")
{
    // The heuristic relies on, to propose a neighbor, and if it is not
    // accepted, go back to exactly where it was.
    TempDatabase temp("undo");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Solution s(inst, {0, 1, 2, 3});
    std::vector<std::size_t> tour_before = s.tour();
    double cost_before = s.cost();

    s.swap_positions(0, 3);
    s.undo();

    CHECK(s.tour() == tour_before);
    // Restored, not recomputed: must be bit-for-bit the same value.
    CHECK(s.cost() == cost_before);
}

TEST_CASE("undo reverts only the most recent move")
{
    TempDatabase temp("undo_recent");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Solution s(inst, {0, 1, 2, 3});

    s.swap_positions(0, 1); // -> 1 0 2 3
    std::vector<std::size_t> mid = s.tour();
    s.swap_positions(2, 3); // -> 1 0 3 2
    s.undo();               // back to 1 0 2 3, not to 0 1 2 3

    CHECK(s.tour() == mid);
}

TEST_CASE("a random neighbor differs in exactly two positions")
{
    TempDatabase temp("neighbor");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Solution s(inst, {0, 1, 2, 3});
    std::vector<std::size_t> before = s.tour();

    Random rng(7);
    s.random_neighbor(rng);

    // Count how many positions changed: a single swap changes exactly two.
    int changed = 0;
    for (std::size_t i = 0; i < before.size(); ++i)
    {
        if (s.tour()[i] != before[i])
            changed++;
    }
    CHECK(changed == 2);

    // It is still a valid permutation.
    std::vector<std::size_t> sorted = s.tour();
    std::sort(sorted.begin(), sorted.end());
    CHECK(sorted == std::vector<std::size_t>{0, 1, 2, 3});
}

TEST_CASE("a neighbor can be undone back to the original")
{
    TempDatabase temp("neighbor_undo");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Solution s(inst, {0, 1, 2, 3});
    std::vector<std::size_t> before = s.tour();
    double cost_before = s.cost();

    Random rng(3);
    s.random_neighbor(rng);
    s.undo();

    CHECK(s.tour() == before);
    CHECK(s.cost() == cost_before);
}