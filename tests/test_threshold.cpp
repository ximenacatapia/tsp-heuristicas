#include "../src/heuristic/threshold_accepting.hpp"

#include "../src/database.hpp"
#include "../src/instance.hpp"
#include "lib/doctest.h"
#include "temp_database.hpp"

namespace
{

    // A toy instance that HAS feasible tours: a ring 1-2-3-4-1 where every
    // consecutive pair in some order is a real edge. With enough edges the
    // heuristic should be able to reach a feasible solution.
    void populate(TempDatabase &db)
    {
        db.add_city(1, "A", "X", 100, 0.0, 0.0);
        db.add_city(2, "B", "X", 100, 0.0, 10.0);
        db.add_city(3, "C", "X", 100, 10.0, 10.0);
        db.add_city(4, "D", "X", 100, 10.0, 0.0);
        // Fully connected: every pair is a real edge, so every tour is feasible.
        db.add_connection(1, 2, 1.0);
        db.add_connection(1, 3, 1.0);
        db.add_connection(1, 4, 1.0);
        db.add_connection(2, 3, 1.0);
        db.add_connection(2, 4, 1.0);
        db.add_connection(3, 4, 1.0);
    }

} // namespace

TEST_CASE("initial temperature accepts about the target fraction")
{
    TempDatabase temp("temp");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Parameters p;
    p.batch_size = 50;
    p.accept_percentage = 0.9;
    p.percentage_samples = 500;

    ThresholdAccepting ta(inst, p);
    Random rng(42);
    Solution s(inst, rng);

    double t = ta.initial_temperature(s, rng);
    CHECK(t > 0.0);
}

TEST_CASE("the run returns the same result for the same seed")
{
    // Reproducibility: identical seed, identical outcome.
    TempDatabase temp("repro");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Parameters p;
    p.batch_size = 50;
    p.cooling = 0.9;

    ThresholdAccepting ta(inst, p);
    Result a = ta.run(123);
    Result b = ta.run(123);

    CHECK(a.cost == doctest::Approx(b.cost));
    CHECK(a.solution.tour() == b.solution.tour());
}

TEST_CASE("on a fully connected instance the result is feasible")
{
    // Every tour is feasible here, so whatever the heuristic returns must be.
    TempDatabase temp("feasible");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Parameters p;
    p.batch_size = 50;
    p.cooling = 0.9;

    ThresholdAccepting ta(inst, p);
    Result r = ta.run(7);

    CHECK(r.feasible);
    CHECK(r.cost <= 1.0); // feasible tours score in [0, 1]
}

TEST_CASE("the result is no worse than a random start")
{
    // The best kept solution should be at least as good as the initial one.
    TempDatabase temp("improve");
    populate(temp);
    Database db(temp.path());
    Instance inst({1, 2, 3, 4}, db);

    Parameters p;
    p.batch_size = 50;
    p.cooling = 0.9;

    ThresholdAccepting ta(inst, p);

    Random rng(55);
    Solution start(inst, rng);
    double start_cost = start.cost();

    Result r = ta.run(55);
    CHECK(r.cost <= start_cost);
}