#ifndef RANDOMVALUESGENERATOR_HPP
#define RANDOMVALUESGENERATOR_HPP

#include "../auxiliary_functions.hpp"

#include <random>
#include <map>

enum Options
{
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


class RandomValuesGenerator
{
public:
	RandomValuesGenerator(){}

	string getRandomValue(string type, int nparams = 5);
private:
	random_device rd;

	Options resolveOption(string type);

	static const map<string, Options> optionString;

	string getRandomList(string inner_type);
	string getRandomMap(string key_type, string value_type);

	string getRandomChar();
	string getRandomShort();
	string getRandomUnsignedShort();
	string getRandomInt();
	string getRandomUnsigned();
	string getRandomLong();
	string getRandomUnsignedLong();
	string getRandomLongLong();
	string getRandomUnsignedLongLong();
	string getRandomDouble();
	string getRandomFloat();
	string getRandomBool();
	string getRandomString();
};

#endif