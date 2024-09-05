#include "RandomValuesGenerator.hpp"

const map<string, Options> RandomValuesGenerator::optionString{
    {"char", Char},
    {"signed_char", Char},
    {"unsigned_char", Char},
    //--
    {"short", Short},
    {"short_int", Short},
    {"signed_short", Short},
    {"signed_short_int", Short},
    //--
    {"unsigned_short", UnsignedShort},
    {"unsigned_short_int", UnsignedShort},
    //--
    {"int", Int},
    {"signed", Int},
    {"signed_int", Int},
    //--
    {"unsigned", Unsigned},
    {"unsigned_int", Unsigned},
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
    //--
    {"unsigned_long_long", UnsignedLongLong},
    {"unsigned_long_long_int", UnsignedLongLong},
    //--
    {"double", Double},
    {"long_double", Double},
    //--
    {"float", Float},
    {"bool", Bool},
    {"string", String}};

Options RandomValuesGenerator::resolveOption(string type) {
    auto it = optionString.find(type);
    if (it != optionString.end())
        return it->second;
    else
        return Invalid_Type;
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
        if (type.find("list") != string::npos ||
            type.find("vector") != string::npos) {
            unsigned first = type.find("<") + 1;
            unsigned last = type.find_last_of(">");

            string newType = type.substr(first, last - first);
            // newType = newType.substr(first + 1);

            replaceAll(newType, " ", "_");

            stringstream ss;
            ss << "{";

            // int for_iterator = 5;
            for (int i = 0; i < nparams; i++) {
                ss << getRandomValue(newType);
                if (i < nparams - 1)
                    ss << ",";
            }

            ss << "}";

            return ss.str();
        } else if (type.find("map") != string::npos) {
            unsigned first = type.find("<") + 1;
            unsigned last = type.find_last_of(">");

            string newType = type.substr(first, last - first);
            replaceAll(newType, ", ", ",");
            replaceAll(newType, " ", "_");

            unsigned splitter = newType.find(",");
            string firstType = newType.substr(0, splitter);
            string secondType = newType.substr(splitter + 1, last - splitter);

            // cout << "first: " << firstType << "\nsecond: " << secondType <<
            // "\n";
            stringstream ss;
            ss << "{";

            // int for_iterator = 5;
            for (int i = 0; i < nparams; i++) {
                ss << "(" << getRandomValue(firstType) << ","
                   << getRandomValue(secondType) << ")";

                if (i < nparams - 1)
                    ss << ",";
            }

            ss << "}";

            // Do something
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
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 9);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomShort() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedShort() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomInt() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsigned() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomLong() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedLong() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomLongLong() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedLongLong() {
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-100, 100);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomDouble() {
    mt19937 gen(rd());
    exponential_distribution<> dis(1);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomFloat() {
    mt19937 gen(rd());
    exponential_distribution<> dis(1);
    return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomBool() {
    mt19937 gen(rd());
    bernoulli_distribution dis(0.5);
    return dis(gen) ? "true" : "false";
}

string RandomValuesGenerator::getRandomString() { return "randomString"; }
