#ifndef CITY_HPP
#define CITY_HPP

#include <string>

/*
 * A city: one row of the `cities` table. Fields map one-to-one to its columns.
 * Coordinates are stored in degrees, as in the database.
 */
struct City
{
    int id = 0;
    std::string name;
    std::string country;
    long long population = 0;
    double latitude = 0.0;
    double longitude = 0.0;
};

#endif // CITY_HPP