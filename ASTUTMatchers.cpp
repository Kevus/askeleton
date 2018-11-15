#include "ASTUTMatchers.hpp"

/***************************************************
 ** DECLARATION OF MATCHERS
 ***************************************************/

DeclarationMatcher FunctionExample =
recordDecl(
	forEachDescendant(
		functionDecl(
				hasAnyParameter(anything())
			).bind("FunctionExample")
	)
);

//We will reunite and insert into the match map here
map<string, DeclarationMatcher> createMapMatchers()
{
	map<string, DeclarationMatcher> matchs;

	matchs.insert(pair<string, DeclarationMatcher>("FunctionExample", FunctionExample));

	return matchs;
}