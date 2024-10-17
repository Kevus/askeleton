#pragma once

#include <map>
#include <random>
#include <string>

enum Options {
    Invalid_Type,
    Char,
    Short,
    UnsignedShort,
    Int,
    Unsigned,
    Long,
    UnsignedLong,
    LongLong,
    UnsignedLongLong,
    Double,
    Float,
    Bool,
    String
};

class RandomValuesGenerator {
public:
    RandomValuesGenerator() {}

    std::string getRandomValue(std::string type, int nparams = 5);

private:
    std::random_device rd;

    Options resolveOption(std::string type);

    static const std::map<std::string, Options> optionString;

    std::string getRandomList(std::string inner_type);
    std::string getRandomMap(std::string key_type, std::string value_type);

    std::string getRandomChar();
    std::string getRandomShort();
    std::string getRandomUnsignedShort();
    std::string getRandomInt();
    std::string getRandomUnsigned();
    std::string getRandomLong();
    std::string getRandomUnsignedLong();
    std::string getRandomLongLong();
    std::string getRandomUnsignedLongLong();
    std::string getRandomDouble();
    std::string getRandomFloat();
    std::string getRandomBool();
    std::string getRandomString();
};