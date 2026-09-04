#include "database.hpp"

#include <stdexcept>
#include <set>

Database::Database(const std::string &path)
{
    // SQLITE_OPEN_READONLY: nunca escribimos en la base.
    int codigo =
        sqlite3_open_v2(path.c_str(), &db_, SQLITE_OPEN_READONLY, nullptr);

    if (codigo != SQLITE_OK)
    {
        std::string message = "Couldn't open the db'" + path +
                              "': " +
                              (db_ ? sqlite3_errmsg(db_) : "unknown error");
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error(message);
    }
}

Database::~Database()
{
    if (db_ != nullptr)
        sqlite3_close(db_);
}

std::vector<City> Database::get_cities(
    const std::vector<int> &ids) const
{
    const char *sql = "SELECT id, name, country, population, latitude, longitude "
                      "FROM cities WHERE id = ?";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare query: " +
                                 std::string(sqlite3_errmsg(db_)));
    }

    std::vector<City> cities;
    cities.reserve(ids.size());

    // La consulta se prepara UNA vez y se reutiliza ligando un id distinto en
    // cada vuelta.
    //`?` placeholder evita problemas de inyección de SQL.
    for (int id : ids)
    {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            throw std::runtime_error("City with id " + std::to_string(id) +
                                     " doesn't exist in db.");
        }

        City c;
        c.id = sqlite3_column_int(stmt, 0);

        // sqlite3_column_text devuelve NULL si la columna es NULL; hay que
        // revisarlo antes de construir el std::string.
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        const unsigned char *country = sqlite3_column_text(stmt, 2);
        c.name = name ? reinterpret_cast<const char *>(name) : "";
        c.country = country ? reinterpret_cast<const char *>(country) : "";

        c.population = sqlite3_column_int64(stmt, 3);
        c.latitude = sqlite3_column_double(stmt, 4);
        c.longitude = sqlite3_column_double(stmt, 5);

        cities.push_back(c);
    }

    sqlite3_finalize(stmt);
    return cities;
}

// Toda la tabla connections se escanea solo una vez y se filtra en memoria
// we the edges of two enpoints that are both in the instance
std::vector<Connection> Database::get_connections(const std::vector<int> &ids) const
{
    std::set<int> wanted(ids.begin(), ids.end()); // set to ask if the city belong to ids.
    sqlite3_stmt *stmt = nullptr;

    const char *sql = "SELECT id_city_1, id_city_2, distance FROM connections";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare query: " +
                                 std::string(sqlite3_errmsg(db_)));
    }

    std::vector<Connection> connections;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Connection c;
        c.c1 = sqlite3_column_int(stmt, 0);
        c.c2 = sqlite3_column_int(stmt, 1);
        // An edge is kept only if both endpoints belong to the instance.
        if (wanted.count(c.c1) == 0 || wanted.count(c.c2) == 0)
            continue;

        c.distance = sqlite3_column_double(stmt, 2);
        connections.push_back(c);
    }
    sqlite3_finalize(stmt);
    return connections;
}

// Cuenta el total de ciudades en la base.
int Database::count_cities() const
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM cities", -1, &stmt,
                           nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("Error counting cities: " +
                                 std::string(sqlite3_errmsg(db_)));
    }

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return total;
}

// Cuenta el total de conexiones en la base.
int Database::count_connections() const
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