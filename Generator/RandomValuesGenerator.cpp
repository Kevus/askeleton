#include "RandomValuesGenerator.hpp"


const map<string, Options> RandomValuesGenerator::optionString
{
	{ "char", Char },
	{ "signed_char", Char },
	{ "unsigned_char", Char },
	//--
	{ "short", Short },
	{ "short_int", Short },
	{ "signed_short", Short },
	{ "signed_short_int", Short },
	//--
	{ "unsigned_short", UnsignedShort },
	{ "unsigned_short_int", UnsignedShort },
	//--
	{ "int", Int },
	{ "signed", Int },
	{ "signed_int", Int },
	//--
	{ "unsigned", Unsigned },
	{ "unsigned_int", Unsigned },
	//--
	{ "long", Long },
	{ "long_int", Long },
	{ "signed_long", Long },
	{ "signed_long_int", Long },
	//--
	{ "unsigned_long", UnsignedLong },
	{ "unsigned_long_int", UnsignedLong },
	//--
	{ "long_long", LongLong },
	{ "long_long_int", LongLong },
	{ "signed_long_long", LongLong },
	{ "signed_long_long_int", LongLong },
	//--
	{ "unsigned_long_long", UnsignedLongLong },
	{ "unsigned_long_long_int", UnsignedLongLong },
	//--
	{ "double", Double },
	{ "long_double", Double },
	//--
	{ "float", Float },
	{ "bool", Bool },
	{ "string", String }
};

Options RandomValuesGenerator::resolveOption(string type)
{
	auto it = optionString.find(type);
	if ( it != optionString.end() )
		return it->second;
	else
		return Invalid_Type;
}

string RandomValuesGenerator::getRandomValue(string type)
{
	switch (resolveOption(type))
	{
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

		default: {
			return "0";
			break;
		}
	}
}

string RandomValuesGenerator::getRandomChar()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(1, 9);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomShort()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedShort()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(0, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomInt()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsigned()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomLong()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedLong()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomLongLong()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomUnsignedLongLong()
{
	mt19937 gen(rd());
	uniform_int_distribution<> dis(-100, 100);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomDouble()
{
	mt19937 gen(rd());
	exponential_distribution<> dis(1);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomFloat()
{
	mt19937 gen(rd());
	exponential_distribution<> dis(1);
	return to_string(dis(gen));
}

string RandomValuesGenerator::getRandomBool()
{
	mt19937 gen(rd());
	bernoulli_distribution dis(0.5);
	return dis(gen) ? "true" : "false";
}

string RandomValuesGenerator::getRandomString()
{
	return "randomString";
}
