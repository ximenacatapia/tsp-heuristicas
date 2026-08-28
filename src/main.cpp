// lee una instancia de TSP y muestra las ciudades que la componen, tomadas de la base de datos.
// Por ahora sólo funciona la lectura.
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "database.hpp"
#include "ciudad.hpp"
#include "lector.hpp"

int main(int argc, char *argv[])
{
    //validamos argc, esperamos la ruta del .tsp y opcionalmente de la base
    //si no se da la base, usamos tsp.db del directorio actual
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Uso: " << argv[0] << " <archivo.tsp> [base.db]\n";
        return 1;
    }

    const std::string ruta_tsp = argv[1];
    const std::string ruta_db = (argc == 3) ? argv[2] : "tsp.db";

    try
    {
        // leemos la instancia (Lector.cpp)
        std::vector<int> ids = leer_instancia(ruta_tsp);

        // abrimos la base 
        DataBase db(ruta_db);
        //consultamos cada id de la base para ciudades
        std::vector<Ciudad> ciudades = db.obtener_ciudades(ids);

        // Nombre del archivo sin la ruta.
        std::string nombre = ruta_tsp;
        std::size_t diagonal = nombre.find_last_of("/\\");
        if (diagonal != std::string::npos)
            nombre = nombre.substr(diagonal + 1);

        std::printf("  Archivo: %s\n", nombre.c_str());
        std::printf(" Ciudades: %zu\n\n", ciudades.size());

        std::printf("%6s  %-22s %-22s %12s %12s\n", "id", "nombre", "país",
                    "latitud", "longitud");
        std::printf(
            "--------------------------------------------------"
            "----------------------------------\n");

        for (const Ciudad &c : ciudades)
        {
            std::printf("%6d  %-22s %-22s %12.4f %12.4f\n", c.id,
                        c.nombre.c_str(), c.pais.c_str(), c.latitud,
                        c.longitud);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}