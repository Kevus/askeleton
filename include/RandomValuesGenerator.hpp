#pragma once

#include <map>
#include <random>
#include <string>
#include <vector>

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

enum class RandomProfile {
    Random,
    Boundary,
    Safe,
    Stress
};

class RandomValuesGenerator {
public:
    RandomValuesGenerator();

    std::string getRandomValue(std::string type, int nparams = 5);
    void setSeed(uint32_t seed);
    // Controls how values are selected for boundary, safe, or stress scenarios.
    void setProfile(RandomProfile profile);

private:
    std::random_device rd;
    std::mt19937 gen;
    RandomProfile profile = RandomProfile::Random;

    Options resolveOption(std::string type);

    static const std::map<std::string, Options> optionString;

    int pickContainerSize();
    int pickStringLength();
    int pickIntBoundary(int minVal, int maxVal);
    long long pickLongBoundary(long long minVal, long long maxVal);

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
