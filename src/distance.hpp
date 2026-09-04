#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include <cmath>

#include "city.hpp"

// Natural distance between two cities (definition 4.1.3).
//
// This is the great-circle distance: the length of the shortest arc over the
// surface of a sphere. It is what the problem uses when two cities are NOT
// directly connected in the map, so a distance still has to be assigned.
//
// The formula assumes the Earth is a perfect sphere. It is not, so the result
// is off by a few kilometres, which the problem statement deliberately
// ignores. On this database the `connections.distance` column was in fact
// computed with this same formula, so a real edge and its natural distance
// agree up to rounding.
//
// Kept as a free function, not a method: it depends only on its inputs, has no
// state, and is trivial to test on its own.
namespace distance
{

    // Earth radius in metres, as fixed by the problem statement.
    inline constexpr double kEarthRadius = 6373000.0;

    // Degrees to radians. The coordinates are stored in degrees but the
    // trigonometric functions work in radians.
    inline double to_radians(double degrees)
    {
        return degrees * M_PI / 180.0;
    }

    // Natural distance in metres between two points given by latitude/longitude.
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

        // atan2(sqrt(a), sqrt(1 - a)) is used instead of asin(sqrt(a)): the two
        // are equal in exact arithmetic, but atan2 is the form given in the
        // definition and is better behaved near the antipodes.
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        return kEarthRadius * c;
    }

    // Convenience overload taking two City objects.
    inline double natural(const City &u, const City &v)
    {
        return natural(u.latitude, u.longitude, v.latitude, v.longitude);
    }

} // namespace distance

#endif // DISTANCE_HPP