#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>

#include <string>
#include <vector>

#include "city.hpp"

/* An edge from the `connections` table: two city ids and the distance. */
struct Connection
{
    int c1 = 0; // city 1
    int c2 = 0; // citty 2
    double distance = 0.0;
};
/*
 * Connection to the cities database. Opens in the constructor, closes in the
 * destructor. Non-copyable: the SQLite handle is a unique resource, so copying
 * it would let two objects close the same connection.
 */
class Database
{
public:
    // Throws std::runtime_error if the file is missing or not a valid database.
    explicit Database(const std::string &path);
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    std::vector<City> get_cities(const std::vector<int> &ids) const;

    std::vector<Connection> get_connections(const std::vector<int> &ids) const;

    int count_cities() const;

    int count_connections() const;

private:
    sqlite3 *db_ = nullptr;
};

#endif // DATABASE_HPP