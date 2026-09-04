#ifndef CITY_HPP
#define CITY_HPP

#include <string>

// Los campos corresponden uno a uno con las columnas de la tabla `cities`.
// Las coordenadas se guardan en grados (como en la base)
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