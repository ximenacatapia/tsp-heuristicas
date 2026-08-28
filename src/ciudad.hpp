#ifndef CIUDAD_HPP
#define CIUDAD_HPP

#include <string>

// Los campos corresponden uno a uno con las columnas de la tabla `cities`.
// Las coordenadas se guardan en grados (como en la base)
struct Ciudad
{
    int id = 0;
    std::string nombre;
    std::string pais;
    long long poblacion = 0;
    double latitud = 0.0;
    double longitud = 0.0;
};

#endif // CIUDAD_HPP