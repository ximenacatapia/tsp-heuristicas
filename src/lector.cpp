#include "lector.hpp"

#include <cctype>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>

std::vector<int> leer_instancia(const std::string &ruta)
{
    std::ifstream archivo(ruta);
    if (!archivo)
    {
        throw std::runtime_error("No se pudo abrir el archivo '" + ruta + "'.");
    }

    // Se lee todo el archivo a memoria.
    std::string contenido((std::istreambuf_iterator<char>(archivo)),
                          std::istreambuf_iterator<char>());

    std::vector<int> ids;
    std::set<int> vistos;

    // Se toma cualquier secuencia de dígitos como un identificador y se ignora todo lo demás. 
    std::size_t i = 0;
    while (i < contenido.size())
    {
        if (!std::isdigit(static_cast<unsigned char>(contenido[i])))
        {
            i++;
            continue;
        }

        int valor = 0;
        while (i < contenido.size() &&
               std::isdigit(static_cast<unsigned char>(contenido[i])))
        {
            valor = valor * 10 + (contenido[i] - '0');
            i++;
        }

        if (vistos.count(valor) > 0)
        {
            throw std::runtime_error("La ciudad " + std::to_string(valor) +
                                     " aparece repetida en '" + ruta + "'.");
        }

        vistos.insert(valor);
        ids.push_back(valor);
    }

    if (ids.size() < 2)
    {
        throw std::runtime_error("La instancia '" + ruta +
                                 "' tiene menos de dos ciudades.");
    }

    return ids;
}