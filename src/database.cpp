#include "database.hpp"

#include <stdexcept>

DataBase::DataBase(const std::string &ruta)
{
    // SQLITE_OPEN_READONLY: nunca escribimos en la base.
    int codigo =
        sqlite3_open_v2(ruta.c_str(), &db_, SQLITE_OPEN_READONLY, nullptr);

    if (codigo != SQLITE_OK)
    {
        std::string mensaje = "No se pudo abrir la base de datos '" + ruta +
                              "': " +
                              (db_ ? sqlite3_errmsg(db_) : "error desconocido");
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error(mensaje);
    }
}

DataBase::~DataBase()
{
    if (db_ != nullptr)
        sqlite3_close(db_);
}

std::vector<Ciudad> DataBase::obtener_ciudades(
    const std::vector<int> &ids) const
{
    const char *sql =
        "SELECT id, name, country, population, latitude, longitude "
        "FROM cities WHERE id = ?";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Error al preparar la consulta: " +
                                 std::string(sqlite3_errmsg(db_)));
    }

    std::vector<Ciudad> ciudades;
    ciudades.reserve(ids.size());

    // La consulta se prepara UNA vez y se reutiliza ligando un id distinto en
    // cada vuelta. El parámetro `?` además evita cualquier problema de
    // inyección de SQL, aunque aquí los valores sean enteros.
    for (int id : ids)
    {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("La ciudad con id " + std::to_string(id) +
                                     " no existe en la base de datos.");
        }

        Ciudad c;
        c.id = sqlite3_column_int(stmt, 0);

        // sqlite3_column_text devuelve NULL si la columna es NULL; hay que
        // revisarlo antes de construir el std::string.
        const unsigned char *nombre = sqlite3_column_text(stmt, 1);
        const unsigned char *pais = sqlite3_column_text(stmt, 2);
        c.nombre = nombre ? reinterpret_cast<const char *>(nombre) : "";
        c.pais = pais ? reinterpret_cast<const char *>(pais) : "";

        c.poblacion = sqlite3_column_int64(stmt, 3);
        c.latitud = sqlite3_column_double(stmt, 4);
        c.longitud = sqlite3_column_double(stmt, 5);

        ciudades.push_back(c);
    }

    sqlite3_finalize(stmt);
    return ciudades;
}

int DataBase::contar_ciudades() const
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM cities", -1, &stmt,
                           nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Error al contar ciudades: " +
                                 std::string(sqlite3_errmsg(db_)));
    }

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return total;
}

int DataBase::contar_conexiones() const
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM connections", -1, &stmt,
                           nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Error al contar conexiones: " +
                                 std::string(sqlite3_errmsg(db_)));
    }

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return total;
}