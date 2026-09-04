#include "../src/distance.hpp"

#include "lib/doctest.h"

TEST_CASE("a point to itself is zero")
{
    // Same coordinates, no distance
    CHECK(distance::natural(19.4, -99.1, 19.4, -99.1) == doctest::Approx(0.0));
}

TEST_CASE("the distance is symmetric")
{
    // d(u, v) must equal d(v, u): the map is undirected.
    double a = distance::natural(0.0, 0.0, 10.0, 20.0);
    double b = distance::natural(10.0, 20.0, 0.0, 0.0);
    CHECK(a == doctest::Approx(b));
}

TEST_CASE("one degree along the equator")
{
    // Along the equator, one degree of longitude spans a known arc
    double expected = distance::kEarthRadius * distance::to_radians(1.0);
    double got = distance::natural(0.0, 0.0, 0.0, 1.0);
    CHECK(got == doctest::Approx(expected));
}

TEST_CASE("one degree along a meridian")
{
    // The same as the equator holds for one degree of latitude at any longitude.
    double expected = distance::kEarthRadius * distance::to_radians(1.0);
    double got = distance::natural(0.0, 0.0, 1.0, 0.0);
    CHECK(got == doctest::Approx(expected));
}

TEST_CASE("degrees to radians")
{
    CHECK(distance::to_radians(0.0) == doctest::Approx(0.0));
    CHECK(distance::to_radians(180.0) == doctest::Approx(M_PI));
    CHECK(distance::to_radians(360.0) == doctest::Approx(2 * M_PI));
}

TEST_CASE("matches a known great-circle distance")
{
    /* Tokyo to Shanghai, coordinates as stored in this database. The expected
     value is the great-circle distance between them, ~1766 km. This is not
     hardcoded.*/
    double tokyo_lat = 35.685, tokyo_lon = 139.751;
    double shanghai_lat = 31.0456, shanghai_lon = 121.4;

    double d = distance::natural(tokyo_lat, tokyo_lon, shanghai_lat,
                                 shanghai_lon);

    // Checked against an independent great-circle calculator
    CHECK(d == doctest::Approx(1766000.0).epsilon(0.01));
}