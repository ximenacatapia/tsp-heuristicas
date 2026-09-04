#ifndef READER_HPP
#define READER_HPP

#include <string>
#include <vector>

// Lee un archivo .tsp y devuelve los identificadores de ciudades en el orden
// en que aparecen.
// Lanzamos std::runtime_error si el archivo no se puede abrir, si no tiene al
// menos dos ciudades, o si algún identificador aparece repetido (una
// instancia S es un conjunto de ciudades, así que no admite repeticiones).
std::vector<int> read_instance(const std::string &ruta);

#endif // LECTOR_HPP