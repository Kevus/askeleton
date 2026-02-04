#include "RandomValuesGenerator.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "utils/strings.hpp"

using namespace std;

RandomValuesGenerator::RandomValuesGenerator() : gen(rd()) {}

const map<string, Options> RandomValuesGenerator::optionString{
    {"char", Char},
    {"signed_char", Char},
    {"unsigned_char", Char},
    {"int8_t", Char},
    {"uint8_t", Char},
    //--
    {"short", Short},
    {"short_int", Short},
    {"signed_short", Short},
    {"signed_short_int", Short},
    {"int16_t", Short},
    //--
    {"unsigned_short", UnsignedShort},
    {"unsigned_short_int", UnsignedShort},
    {"uint16_t", UnsignedShort},
    //--
    {"int", Int},
    {"signed", Int},
    {"signed_int", Int},
    {"int32_t", Int},
    //--
    {"unsigned", Unsigned},
    {"unsigned_int", Unsigned},
    {"uint32_t", Unsigned},
    {"size_t", Unsigned},
    //--
    {"long", Long},
    {"long_int", Long},
    {"signed_long", Long},
    {"signed_long_int", Long},
    //--
    {"unsigned_long", UnsignedLong},
    {"unsigned_long_int", UnsignedLong},
    //--
    {"long_long", LongLong},
    {"long_long_int", LongLong},
    {"signed_long_long", LongLong},
    {"signed_long_long_int", LongLong},
    {"int64_t", LongLong},
    //--
    {"unsigned_long_long", UnsignedLongLong},
    {"unsigned_long_long_int", UnsignedLongLong},
    {"uint64_t", UnsignedLongLong},
    //--
    {"double", Double},
    {"long_double", Double},
    //--
    {"float", Float},
    {"bool", Bool},
    {"string", String},
    {"std::string", String},
    {"basic_string<char>", String},
    {"std::basic_string<char>", String}};

Options RandomValuesGenerator::resolveOption(string type) {
    auto it = optionString.find(type);
    if (it != optionString.end())
        return it->second;
    else
        return Invalid_Type;
}

void RandomValuesGenerator::setSeed(uint32_t seed) { gen.seed(seed); }

void RandomValuesGenerator::setProfile(RandomProfile newProfile) {
    profile = newProfile;
}

int RandomValuesGenerator::pickContainerSize() {
    vector<int> sizes;
    switch (profile) {
    case RandomProfile::Boundary:
        sizes = {0, 1};
        break;
    case RandomProfile::Safe:
        sizes = {1, 3};
        break;
    case RandomProfile::Stress:
        sizes = {8, 16};
        break;
    default:
        sizes = {0, 1, 3, 5, 8};
        break;
    }
    uniform_int_distribution<int> dis(0, static_cast<int>(sizes.size()) - 1);
    return sizes[dis(gen)];
}

int RandomValuesGenerator::pickStringLength() {
    vector<int> lengths;
    switch (profile) {
    case RandomProfile::Boundary:
        lengths = {0, 1};
        break;
    case RandomProfile::Safe:
        lengths = {1, 4, 8};
        break;
    case RandomProfile::Stress:
        lengths = {32, 64, 128};
        break;
    default:
        lengths = {1, 3, 8, 16};
        break;
    }
    uniform_int_distribution<int> dis(0, static_cast<int>(lengths.size()) - 1);
    return lengths[dis(gen)];
}

int RandomValuesGenerator::pickIntBoundary(int minVal, int maxVal) {
    vector<int> candidates = {minVal, minVal + 1, -1, 0, 1, maxVal - 1, maxVal};
    uniform_int_distribution<int> dis(0, static_cast<int>(candidates.size()) - 1);
    return candidates[dis(gen)];
}

long long RandomValuesGenerator::pickLongBoundary(long long minVal, long long maxVal) {
    vector<long long> candidates = {minVal, minVal + 1, -1, 0, 1, maxVal - 1, maxVal};
    uniform_int_distribution<int> dis(0, static_cast<int>(candidates.size()) - 1);
    return candidates[dis(gen)];
}

string RandomValuesGenerator::getRandomValue(string type, int nparams) {
    switch (resolveOption(type)) {
    case Char: {
        return getRandomChar();
        break;
    }

    case Short: {
        return getRandomShort();
        break;
    }

    case UnsignedShort: {
        return getRandomUnsignedShort();
        break;
    }

    case Int: {
        return getRandomInt();
        break;
    }

    case Unsigned: {
        return getRandomUnsigned();
        break;
    }

    case Long: {
        return getRandomLong();
        break;
    }

    case UnsignedLong: {
        return getRandomUnsignedLong();
        break;
    }

    case LongLong: {
        return getRandomLongLong();
        break;
    }

    case UnsignedLongLong: {
        return getRandomUnsignedLongLong();
        break;
    }

    case Double: {
        return getRandomDouble();
        break;
    }

    case Float: {
        return getRandomFloat();
        break;
    }

    case Bool: {
        return getRandomBool();
        break;
    }

    case String: {
        return getRandomString();
        break;
    }

    case Invalid_Type: {
        if (type.find("map") != string::npos) {
            size_t left = type.find("<");
            size_t right = type.find_last_of(">");
            if (left == string::npos || right == string::npos || right <= left) {
                return "0";
            }
            unsigned first = left + 1;
            unsigned last = static_cast<unsigned>(right);

            string newType = type.substr(first, last - first);
            replaceAll(newType, ", ", ",");

            size_t splitter = newType.find(",");
            if (splitter == string::npos) {
                return "0";
            }
            string firstType = newType.substr(0, splitter);
            string rest = newType.substr(splitter + 1);
            size_t second_split = rest.find(",");
            string secondType = (second_split == string::npos)
                                    ? rest
                                    : rest.substr(0, second_split);

            replaceAll(firstType, " ", "_");
            replaceAll(secondType, " ", "_");

            stringstream ss;
            ss << "{";

            const int mapSize = pickContainerSize();
            for (int i = 0; i < mapSize; i++) {
                ss << "(" << getRandomValue(firstType) << ","
                   << getRandomValue(secondType) << ")";

                if (i < mapSize - 1)
                    ss << ",";
            }

            ss << "}";

            // Do something
            return ss.str();
        } else if (type.find("list") != string::npos ||
                   type.find("vector") != string::npos) {
            size_t left = type.find("<");
            size_t right = type.find_last_of(">");
            if (left == string::npos || right == string::npos || right <= left) {
                return "0";
            }
            unsigned first = left + 1;
            unsigned last = static_cast<unsigned>(right);

            string newType = type.substr(first, last - first);
            size_t comma = newType.find(",");
            if (comma != string::npos) {
                newType = newType.substr(0, comma);
            }
            // newType = newType.substr(first + 1);

            replaceAll(newType, " ", "_");

            stringstream ss;
            ss << "{";

            const int listSize = pickContainerSize();
            for (int i = 0; i < listSize; i++) {
                ss << getRandomValue(newType);
                if (i < listSize - 1)
                    ss << ",";
            }

            ss << "}";

            return ss.str();
        } else if (type.find("struct") != string::npos) {
            stringstream ss;
            for (int i = 0; i < nparams; i++) {
                ss << getRandomValue("int");

                if (i < nparams - 1)
                    ss << ",";
            }

            return ss.str();
        } else {
            return "0";
        }

        break;
    }

    default: {
        return "0";
        break;
    }
    }
}

string RandomValuesGenerator::getRandomChar() {
    char value = 'a';
    switch (profile) {
    case RandomProfile::Boundary: {
        const vector<char> candidates = {'\0', 'a', 'z'};
        uniform_int_distribution<int> dis(0, static_cast<int>(candidates.size()) - 1);
        value = candidates[dis(gen)];
        break;
    }
    case RandomProfile::Safe:
        value = 'a';
        break;
    case RandomProfile::Stress:
    case RandomProfile::Random:
    default: {
        uniform_int_distribution<> dis(97, 122);
        value = static_cast<char>(dis(gen));
        break;
    }
    }
    return string(1, value);
}

string RandomValuesGenerator::getRandomShort() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickIntBoundary(-100, 100));
    }
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedShort() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickIntBoundary(0, 100));
    }
    uniform_int_distribution<> dis(0, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomInt() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickIntBoundary(-100, 100));
    }
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsigned() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickIntBoundary(0, 100));
    }
    uniform_int_distribution<> dis(0, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomLong() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickLongBoundary(-100, 100));
    }
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedLong() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickLongBoundary(0, 100));
    }
    uniform_int_distribution<> dis(0, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomLongLong() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickLongBoundary(-100, 100));
    }
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedLongLong() {
    if (profile == RandomProfile::Boundary) {
        return to_string(pickLongBoundary(0, 100));
    }
    uniform_int_distribution<> dis(0, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomDouble() {
    double value = 0.0;
    switch (profile) {
    case RandomProfile::Boundary: {
        const vector<double> candidates = {-100.0, -1.0, 0.0, 1.0, 100.0};
        uniform_int_distribution<int> dis(0, static_cast<int>(candidates.size()) - 1);
        value = candidates[dis(gen)];
        break;
    }
    case RandomProfile::Safe:
        value = 1.0;
        break;
    case RandomProfile::Stress:
    case RandomProfile::Random:
    default: {
        uniform_real_distribution<double> dis(-100.0, 100.0);
        value = dis(gen);
        break;
    }
    }
    ostringstream oss;
    oss << fixed << setprecision(3) << value;
    return oss.str();
}

string RandomValuesGenerator::getRandomFloat() {
    float value = 0.0f;
    switch (profile) {
    case RandomProfile::Boundary: {
        const vector<float> candidates = {-100.0f, -1.0f, 0.0f, 1.0f, 100.0f};
        uniform_int_distribution<int> dis(0, static_cast<int>(candidates.size()) - 1);
        value = candidates[dis(gen)];
        break;
    }
    case RandomProfile::Safe:
        value = 1.0f;
        break;
    case RandomProfile::Stress:
    case RandomProfile::Random:
    default: {
        uniform_real_distribution<float> dis(-100.0f, 100.0f);
        value = dis(gen);
        break;
    }
    }
    ostringstream oss;
    oss << fixed << setprecision(3) << value;
    return oss.str();
}

string RandomValuesGenerator::getRandomBool() {
    if (profile == RandomProfile::Safe)
        return "true";
    bernoulli_distribution dis(0.5);
    return dis(gen) ? "true" : "false";
}

string RandomValuesGenerator::getRandomString() {
    uniform_int_distribution<int> chDis(97, 122);
    const int length = pickStringLength();
    string result;
    result.reserve(static_cast<size_t>(length));
    for (int i = 0; i < length; ++i) {
        result.push_back(static_cast<char>(chDis(gen)));
    }
    return result;
}
