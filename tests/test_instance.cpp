#include "../src/instance.hpp"

#include <vector>

#include "../src/database.hpp"
#include "lib/doctest.h"
#include "temp_database.hpp"

namespace
{

    // Four cities in a square-ish layout, with a known set of real edges. All
    // coordinates and distances are made up so the expected results can be worked
    // out by hand.
    void populate(TempDatabase &db)
    {
        db.add_city(1, "A", "X", 100, 0.0, 0.0);
        db.add_city(2, "B", "X", 100, 0.0, 10.0);
        db.add_city(3, "C", "X", 100, 10.0, 10.0);
        db.add_city(4, "D", "X", 100, 10.0, 0.0);

        // Real edges: a ring 1-2-3-4, plus one diagonal 1-3.
        db.add_connection(1, 2, 100.0);
        db.add_connection(2, 3, 200.0);
        db.add_connection(3, 4, 300.0);
        db.add_connection(4, 1, 400.0);
        db.add_connection(1, 3, 500.0);
        // The diagonal 2-4 is deliberately missing.
    }

} // namespace

TEST_CASE("the instance reindexes ids to 0..k-1")
{
    TempDatabase temp("reindex");
    populate(temp);
    Database db(temp.path());

    // Ids given out of order: the index must follow the given order, not the
    // numeric order.
    Instance inst({3, 1, 4, 2}, db);

    REQUIRE(inst.size() == 4);
    CHECK(inst.id_at(0) == 3);
    CHECK(inst.id_at(1) == 1);
    CHECK(inst.id_at(2) == 4);
    CHECK(inst.id_at(3) == 2);
}

TEST_CASE("real edges keep their database distance")
{
    TempDatabase temp("real_edges");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // Indices match ids here since they were given in order.
    CHECK(inst.connected(0, 1));
    CHECK(inst.weight(0, 1) == doctest::Approx(100.0));

    // Symmetry: the edge is undirected.
    CHECK(inst.weight(1, 0) == doctest::Approx(100.0));

    CHECK(inst.weight(2, 3) == doctest::Approx(300.0));
    CHECK(inst.weight(0, 2) == doctest::Approx(500.0)); // the diagonal 1-3
}

TEST_CASE("a missing edge gets the augmented weight")
{
    TempDatabase temp("augmented");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // 2-4 has no real edge (indices 1 and 3).
    CHECK_FALSE(inst.connected(1, 3));

    // Its weight must be natural(2,4) * max_distance, far larger than any real
    // edge, so the penalty is doing its job.
    CHECK(inst.weight(1, 3) > inst.max_distance());
}

TEST_CASE("max_distance is the longest real edge")
{
    TempDatabase temp("max");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // The heaviest real edge is 1-3 at 500.
    CHECK(inst.max_distance() == doctest::Approx(500.0));
}

TEST_CASE("the normalizer sums the k-1 heaviest real edges")
{
    TempDatabase temp("normalizer");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // k = 4, so k-1 = 3 heaviest real edges: 500 + 400 + 300 = 1200.
    CHECK(inst.normalizer() == doctest::Approx(1200.0));
}

TEST_CASE("with fewer than k-1 real edges, all are summed")
{
    TempDatabase temp("few_edges");
    temp.add_city(1, "A", "X", 100, 0.0, 0.0);
    temp.add_city(2, "B", "X", 100, 0.0, 10.0);
    temp.add_city(3, "C", "X", 100, 10.0, 10.0);
    // Only one real edge among three cities; k-1 would be 2.
    temp.add_connection(1, 2, 700.0);

    Database db(temp.path());
    Instance inst({1, 2, 3}, db);

    // Only edge available, so the normalizer is just that one.
    CHECK(inst.normalizer() == doctest::Approx(700.0));
    CHECK(inst.max_distance() == doctest::Approx(700.0));
}

TEST_CASE("an instance with no connected pair throws")
{
    TempDatabase temp("disconnected");
    temp.add_city(1, "A", "X", 100, 0.0, 0.0);
    temp.add_city(2, "B", "X", 100, 0.0, 10.0);
    // No connections at all.

    Database db(temp.path());
    CHECK_THROWS_AS(Instance({1, 2}, db), std::runtime_error);
}

TEST_CASE("evaluate sums the tour weights over the normalizer")
{
    TempDatabase temp("evaluate");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // Normalizer is 500 + 400 + 300 = 1200 (checked above).
    // Tour 0-1-2-3 uses edges 1-2 (100), 2-3 (200), 3-4 (300) = 600.
    // Cost = 600 / 1200 = 0.5.
    std::vector<std::size_t> tour = {0, 1, 2, 3};
    CHECK(inst.evaluate(tour) == doctest::Approx(0.5));
}

TEST_CASE("a feasible tour scores in [0, 1]")
{
    TempDatabase temp("feasible_score");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // 0-1-2-3 uses only real edges (1-2, 2-3, 3-4).
    std::vector<std::size_t> tour = {0, 1, 2, 3};
    CHECK(inst.is_feasible(tour));
    CHECK(inst.evaluate(tour) <= 1.0);
}

TEST_CASE("a non-feasible tour scores above 1")
{
    TempDatabase temp("infeasible_score");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    // 0-1-3-2 uses edge 2-4 (indices 1-3), which is not a real edge. The
    // augmented penalty pushes the cost above 1.
    std::vector<std::size_t> tour = {0, 1, 3, 2};
    CHECK_FALSE(inst.is_feasible(tour));
    CHECK(inst.evaluate(tour) > 1.0);
}

TEST_CASE("is_feasible detects a single missing edge")
{
    TempDatabase temp("feasible_detail");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    SUBCASE("all edges real")
    {
        // 0-2-... uses 1-3 (real diagonal) then 3-4, 4-1: all real.
        CHECK(inst.is_feasible({1, 0, 2, 3})); // 2-1-3-4
    }

    SUBCASE("one edge missing")
    {
        // ...-1-3 is edge 2-4, missing.
        CHECK_FALSE(inst.is_feasible({0, 1, 3, 2}));
    }
}
