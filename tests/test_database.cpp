#include <string>
#include <vector>

#include "../src/database.hpp"
#include "../src/city.hpp"
#include "temp_database.hpp"
#include "lib/doctest.h"

namespace
{

    // Fills a temporary database with a known set of cities.
    /*
    The values are made up on purpose: this way the tests verify that column
    reading works.
    The coordinates cover cases worth exercising: the origin, negative latitude
    and longitude, and the extreme values of the range.*/
    void populate(TempDatabase &db)
    {
        db.add_city(1, "Alfa", "One", 1000, 0.0, 0.0);
        db.add_city(2, "Beta", "Two", 2000, 10.5, 20.25);
        db.add_city(3, "Gamma South", "Three Words", 3000, -15.75, -30.5);
        db.add_city(7, "Delta", "Four", 4000, 90.0, 180.0);
        db.add_city(42, "Épsilon", "Five", 5000, -90.0, -180.0);
    }

} // namespace

TEST_CASE("opening the database")
{
    SUBCASE("a valid database opens without error")
    {
        TempDatabase temp("open_ok");
        populate(temp);

        CHECK_NOTHROW(Database db(temp.path()));
    }

    SUBCASE("a nonexistent database throws")
    {
        CHECK_THROWS_AS(Database db("/tmp/does_not_exist_12345.db"),
                        std::runtime_error);
    }
}

TEST_CASE("reading a single city")
{
    TempDatabase temp("one_city");
    populate(temp);
    Database db(temp.path());

    std::vector<City> cities = db.get_cities({2});
    REQUIRE(cities.size() == 1);

    // Each field is checked separately: if the columns were read in the wrong
    // order, the failure would point at exactly which one is off.
    CHECK(cities[0].id == 2);
    CHECK(cities[0].name == "Beta");
    CHECK(cities[0].country == "Two");
    CHECK(cities[0].population == 2000);
    CHECK(cities[0].latitude == doctest::Approx(10.5));
    CHECK(cities[0].longitude == doctest::Approx(20.25));
}

TEST_CASE("coordinates keep their sign")
{
    // If the sign were lost, a southern-hemisphere city would end up in the
    // north and every distance would come out wrong with no warning.
    TempDatabase temp("signs");
    populate(temp);
    Database db(temp.path());

    std::vector<City> cities = db.get_cities({3});
    REQUIRE(cities.size() == 1);

    CHECK(cities[0].latitude == doctest::Approx(-15.75));
    CHECK(cities[0].longitude == doctest::Approx(-30.5));
}

TEST_CASE("coordinates at the extremes of the range")
{
    TempDatabase temp("extremes");
    populate(temp);
    Database db(temp.path());

    SUBCASE("maxima")
    {
        std::vector<City> cities = db.get_cities({7});
        CHECK(cities[0].latitude == doctest::Approx(90.0));
        CHECK(cities[0].longitude == doctest::Approx(180.0));
    }

    SUBCASE("minima")
    {
        std::vector<City> cities = db.get_cities({42});
        CHECK(cities[0].latitude == doctest::Approx(-90.0));
        CHECK(cities[0].longitude == doctest::Approx(-180.0));
    }
}

TEST_CASE("names with spaces and non-ASCII characters")
{
    TempDatabase temp("names");
    populate(temp);
    Database db(temp.path());

    SUBCASE("name and country with spaces")
    {
        std::vector<City> cities = db.get_cities({3});
        CHECK(cities[0].name == "Gamma South");
        CHECK(cities[0].country == "Three Words");
    }

    SUBCASE("accents")
    {
        std::vector<City> cities = db.get_cities({42});
        CHECK(cities[0].name == "Épsilon");
    }
}

TEST_CASE("the returned order is the requested order")
{
    // here is what lets cities[i] correspond to ids[i].
    TempDatabase temp("order");
    populate(temp);
    Database db(temp.path());

    std::vector<int> ids = {42, 1, 7, 2};
    std::vector<City> cities = db.get_cities(ids);

    REQUIRE(cities.size() == ids.size());
    for (std::size_t i = 0; i < ids.size(); i++)
    {
        CHECK(cities[i].id == ids[i]);
    }
}

TEST_CASE("identifiers need not be consecutive")
{
    // Real instances use sparse identifiers: 1, 2, 75, 163...
    TempDatabase temp("sparse");
    populate(temp);
    Database db(temp.path());

    std::vector<City> cities = db.get_cities({1, 7, 42});

    REQUIRE(cities.size() == 3);
    CHECK(cities[0].name == "Alfa");
    CHECK(cities[1].name == "Delta");
    CHECK(cities[2].name == "Épsilon");
}

TEST_CASE("a nonexistent city throws")
{
    TempDatabase temp("nonexistent");
    populate(temp);
    Database db(temp.path());

    CHECK_THROWS_AS(db.get_cities({999}), std::runtime_error);
}

TEST_CASE("requesting zero cities returns an empty list")
{
    TempDatabase temp("empty");
    populate(temp);
    Database db(temp.path());

    CHECK(db.get_cities({}).empty());
}

TEST_CASE("the counts reflect what is in the database")
{
    // Not fixed number, the test counts what it inserted itself, so it can return something else depending on the db
    TempDatabase temp("counts");
    populate(temp);
    temp.add_connection(1, 2, 100.0);
    temp.add_connection(2, 3, 200.0);

    Database db(temp.path());

    CHECK(db.count_cities() == 5);
    CHECK(db.count_connections() == 2);
}

TEST_CASE("get_connections returns edges among instance cities")
{
    TempDatabase temp("connections");
    populate(temp);
    // A small graph over cities {1, 2, 3, 7}. City 42 is left unconnected on purpose, to check it never shows up.
    temp.add_connection(1, 2, 100.0);
    temp.add_connection(2, 3, 200.0);
    temp.add_connection(1, 7, 300.0);
    temp.add_connection(3, 7, 400.0);

    Database db(temp.path());

    SUBCASE("all endpoints in the instance")
    {
        std::vector<Connection> edges = db.get_connections({1, 2, 3, 7});
        CHECK(edges.size() == 4);
    }

    SUBCASE("an edge with one endpoint outside is excluded")
    {
        // Instance {1, 2}: only the edge 1-2 qualifies. The edges 2-3, 1-7 and
        // 3-7 each have an endpoint outside the instance and must be dropped.
        std::vector<Connection> edges = db.get_connections({1, 2});
        REQUIRE(edges.size() == 1);
        CHECK(edges[0].c1 == 1);
        CHECK(edges[0].c2 == 2);
        CHECK(edges[0].distance == doctest::Approx(100.0));
    }

    SUBCASE("a city with no edges yields none")
    {
        // City 42 has no connections at all.
        std::vector<Connection> edges = db.get_connections({42});
        CHECK(edges.empty());
    }

    SUBCASE("distances come back intact")
    {
        std::vector<Connection> edges = db.get_connections({3, 7});
        REQUIRE(edges.size() == 1);
        CHECK(edges[0].distance == doctest::Approx(400.0));
    }
}

TEST_CASE("an empty database is handled without breaking")
{
    TempDatabase temp("no_data");
    Database db(temp.path());

    CHECK(db.count_cities() == 0);
    CHECK(db.count_connections() == 0);
    CHECK_THROWS_AS(db.get_cities({1}), std::runtime_error);
}