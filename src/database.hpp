#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>

#include <string>
#include <vector>

#include "ciudad.hpp"

// Conexión a la base de datos de ciudades.
//
// La conexión se abre en el constructor y se cierra en el destructor.
// La clase no se puede copiar porque el handle de SQLite es un recurso único:
// si se copiara, dos objetos intentarían cerrar la misma conexión.
class DataBase
{
public:
    // Lanza std::runtime_error si el archivo no existe o no es una base
    // válida.
    explicit DataBase(const std::string &ruta);
    ~DataBase();

    DataBase(const DataBase &) = delete;
    DataBase &operator=(const DataBase &) = delete;

    // Devuelve las ciudades con los identificadores dados en el mismo orden en que se piden. 
    //Lanza std::runtime_error si alguna no existe.
    std::vector<Ciudad> obtener_ciudades(const std::vector<int> &ids) const;

    // Cuántas ciudades hay en total en la base. Sirve para verificar que la
    // base se cargó completa.
    int contar_ciudades() const;

    // Cuántas conexiones hay en total en la base.
    int contar_conexiones() const;

private:
    sqlite3 *db_ = nullptr;
};

#endif // DATABASE_HPP