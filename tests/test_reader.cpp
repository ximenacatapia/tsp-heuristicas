#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "../src/reader.hpp"
#include "lib/doctest.h"

namespace
{

    // Escribe un archivo temporal y lo borra al destruirse, para que las pruebas
    // no dependan de archivos del repositorio ni del directorio desde el que se
    // corran.
    class TempFile
    {
    public:
        TempFile(const std::string &name, const std::string &content)
        {
            path_ = "/tmp/tsp_test_" + name + ".tsp";
            std::ofstream file(path_);
            file << content;
        }

        ~TempFile() { std::remove(path_.c_str()); }

        const std::string &path() const { return path_; }

    private:
        std::string path_;
    };

}

//----
TEST_CASE("the reader extracts the identifiers")
{
    TempFile file("basic", "1,2,3,4,5\n");
    std::vector<int> ids = read_instance(file.path());

    CHECK(ids.size() == 5);
    CHECK(ids[0] == 1);
    CHECK(ids[4] == 5);
}

TEST_CASE("the reader preserves the file order")
{
    // El orden es la trayectoria propuesta: si el lector ordenara los
    // identificadores, se perdería la solución que se quiere evaluar.
    TempFile file("order", "5,3,1,4,2\n");
    std::vector<int> ids = read_instance(file.path());

    CHECK(ids == std::vector<int>{5, 3, 1, 4, 2});
}

TEST_CASE("the reader preserves the file order")
{
    SUBCASE("commas")
    {
        TempFile file("sep_commas", "10,20,30");
        CHECK(read_instance(file.path()) == std::vector<int>{10, 20, 30});
    }

    SUBCASE("spaces")
    {
        TempFile file("sep_spaces", "10 20 30");
        CHECK(read_instance(file.path()) == std::vector<int>{10, 20, 30});
    }

    SUBCASE("newline")
    {
        TempFile file("sep_newline", "10\n20\n30\n");
        CHECK(read_instance(file.path()) == std::vector<int>{10, 20, 30});
    }

    SUBCASE("mixed and extra whitespace")
    {
        TempFile file("sep_mixto", "  10, 20\n\t30  \n");
        CHECK(read_instance(file.path()) == std::vector<int>{10, 20, 30});
    }
}

TEST_CASE("the reader does not split multi-digit number")
{
    // Un bucle mal escrito leería 1092 como 1, 0, 9, 2.
    TempFile file("digits", "1,1092,500");
    std::vector<int> ids = read_instance(file.path());

    CHECK(ids == std::vector<int>{1, 1092, 500});
}

TEST_CASE("the reader rejects invalid instances")
{
    SUBCASE("nonexistent file")
    {
        CHECK_THROWS_AS(read_instance("/tmp/does_not_exist_12345.tsp"),
                        std::runtime_error);
    }

    SUBCASE("one single city")
    {
        // Con una ciudad no hay trayectoria que evaluar.
        TempFile file("one", "42\n");
        CHECK_THROWS_AS(read_instance(file.path()), std::runtime_error);
    }

    SUBCASE("empty file")
    {
        TempFile file("empty", "");
        CHECK_THROWS_AS(read_instance(file.path()), std::runtime_error);
    }

    SUBCASE("repeated identifier")
    {
        // Una instancia S es un conjunto de ciudades: no admite repeticiones.
        TempFile file("repeated", "1,2,3,2\n");
        CHECK_THROWS_AS(read_instance(file.path()), std::runtime_error);
    }
}

TEST_CASE("the reader works with instances of any size")
{
    // No hay ningún tamaño privilegiado: el programa debe aceptar la
    // instancia que se le dé, no sólo las de 40 o 150 ciudades.
    std::string content;
    for (int i = 1; i <= 300; i++)
    {
        if (i > 1)
            content += ",";
        content += std::to_string(i);
    }

    TempFile file("large", content);
    std::vector<int> ids = read_instance(file.path());

    CHECK(ids.size() == 300);
    CHECK(ids.front() == 1);
    CHECK(ids.back() == 300);
}