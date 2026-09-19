#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include <cmath>

#include "city.hpp"

/*
 * Natural (great-circle) distance between two cities, definition 4.1.3. Used
 * for pairs not directly connected in the map. Assumes a spherical Earth.
 */
namespace distance
{
    // Earth radius in metres, as fixed by the problem statement.
    inline constexpr double kEarthRadius = 6373000.0;

    // Degrees to radians, for the trigonometric function to work
    inline double to_radians(double degrees)
    {
        return degrees * M_PI / 180.0;
    }

    // Natural distance in metres between two latitude/longitude points.
    inline double natural(double lat1, double lon1, double lat2, double lon2)
    {
        double rlat1 = to_radians(lat1);
        double rlat2 = to_radians(lat2);
        double dlat = to_radians(lat2 - lat1);
        double dlon = to_radians(lon2 - lon1);

        // A is the square of half the chord length between the two points.
        double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
                   std::cos(rlat1) * std::cos(rlat2) * std::sin(dlon / 2) *
                       std::sin(dlon / 2);

        // atan2(sqrt(a), sqrt(1 - a)) is used instead of asin(sqrt(a))
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        return kEarthRadius * c;
    }

    // Overload taking two City objects.
    inline double natural(const City &u, const City &v)
    {
        return natural(u.latitude, u.longitude, v.latitude, v.longitude);
    }

}

#endif // DISTANCE_HPP