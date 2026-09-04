#ifndef TEMP_DATABASE_HPP
#define TEMP_DATABASE_HPP

#include <sqlite3.h>

#include <cstdio>
#include <stdexcept>
#include <string>

// Crea una base de datos temporal con el mismo esquema que la real, la puebla
// con los datos que se le den, y la borra al destruirse.
//
//
class TempDatabase
{
public:
    // El nombre se usa para el archivo.
    explicit TempDatabase(const std::string &name)
    {
        path_ = "/tmp/tsp_test_" + name + ".db";
        std::remove(path_.c_str());

        if (sqlite3_open(path_.c_str(), &db_) != SQLITE_OK)
        {
            throw std::runtime_error("Couldn't create temporal database");
        }

        // Same schema as the real database.Si cambiamos l base esto también
        exec(
            "CREATE TABLE cities ("
            "  id INTEGER PRIMARY KEY,"
            "  name TEXT,"
            "  country TEXT,"
            "  population INTEGER,"
            "  latitude DOUBLE,"
            "  longitude DOUBLE);");

        exec(
            "CREATE TABLE connections ("
            "  id_city_1 INTEGER,"
            "  id_city_2 INTEGER,"
            "  distance DOUBLE,"
            "  PRIMARY KEY (id_city_1, id_city_2));");
    }

    ~TempDatabase()
    {
        if (db_ != nullptr)
            sqlite3_close(db_);
        std::remove(path_.c_str());
    }

    TempDatabase(const TempDatabase &) = delete;
    TempDatabase &operator=(const TempDatabase &) = delete;

    void add_city(int id, const std::string &name,
                  const std::string &country, long long population,
                  double latitude, double longitude)
    {
        std::string sql = "INSERT INTO cities VALUES (" + std::to_string(id) +
                          ", '" + escape(name) + "', '" + escape(country) +
                          "', " + std::to_string(population) + ", " +
                          std::to_string(latitude) + ", " +
                          std::to_string(longitude) + ");";
        exec(sql);
    }

    // La tabla real guarda cada arista una sola vez, siempre con
    // id_city_1 < id_city_2. Aquí se respeta esa convención para que las
    // pruebas ejerciten el mismo caso que se dará en producción.
    void add_connection(int c1, int c2, double distance)
    {
        if (c1 > c2)
            std::swap(c1, c2);
        std::string sql = "INSERT INTO connections VALUES (" +
                          std::to_string(c1) + ", " +
                          std::to_string(c2) + ", " +
                          std::to_string(distance) + ");";
        exec(sql);
    }

    const std::string &path() const { return path_; }

private:
    void exec(const std::string &sql)
    {
        char *error = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) !=
            SQLITE_OK)
        {
            std::string message = error ? error : "unknown error";
            sqlite3_free(error);
            throw std::runtime_error("Database error: " + message);
        }
    }

    // Duplica las comillas simples para que un nombre como "O'Higgins" no
    // rompa el INSERT.
    static std::string escape(const std::string &text)
    {
        std::string out;
        for (char c : text)
        {
            out += c;
            if (c == '\'')
                out += '\'';
        }
        return out;
    }

    sqlite3 *db_ = nullptr;
    std::string path_;
};

#endif // BASE_TEMPORAL_HPP