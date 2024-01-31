#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <utility>

typedef std::string formattedType;
typedef std::string originalType;

typedef std::pair<formattedType, originalType> Type;
typedef std::pair<std::string, Type> Parameter;

#endif /* TYPES_HPP */