#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>

#include <string>
#include <vector>

#include "city.hpp"

// tabla de conexiones, tenemos 2 ids de 2 ciudades y la dintancia entre ambas
struct Connection
{
    int c1 = 0; // city 1
    int c2 = 0; // citty 2
    double distance = 0.0;
};
/*Conexión a la base de datos de ciudades.
La conexión se abre en el constructor y se cierra en el destructor.
La clase no se puede copiar porque el handle de SQLite es un recurso único:
si se copiara, dos objetos intentarían cerrar la misma conexión.*/
class Database
{
public:
    // Lanza std::runtime_error si el archivo no existe o no es una base
    // válida.
    explicit Database(const std::string &path);
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    // Devuelve las ciudades con los identificadores dados en el mismo orden en que se piden.
    // Lanza std::runtime_error si alguna no existe.
    std::vector<City> get_cities(const std::vector<int> &ids) const;

    // Devuelve cada arista de la tabala en la cual ambos endpoints deben estar en ids
    std::vector<Connection> get_connections(const std::vector<int> &ids) const;

    // Cuántas ciudades hay en total en la base. Sirve para verificar que la
    // base se cargó completa.
    int count_cities() const;

    // Cuántas conexiones hay en total en la base.
    int count_connections() const;

private:
    sqlite3 *db_ = nullptr;
};

#endif // DATABASE_HPP