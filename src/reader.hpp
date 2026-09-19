#ifndef READER_HPP
#define READER_HPP

#include <string>
#include <vector>

/*
 * Reads a .tsp file and returns the city ids in the order they appear. Throws
 * std::runtime_error if the file cannot be opened, holds fewer than two cities,
 * or repeats an id (an instance S is a set, so it allows no repetitions).
 */
std::vector<int> read_instance(const std::string &ruta);

#endif // LECTOR_HPP