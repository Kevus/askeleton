#include "ASTUTMatchers.hpp"

/***************************************************
 ** DECLARATION OF MATCHERS
 ***************************************************/

DeclarationMatcher FD1 =
	functionDecl(
		unless(returns(voidType()))
	).bind("FD1");

//We will reunite and insert into the match map here
map<string, DeclarationMatcher> createMapMatchers()
{
	map<string, DeclarationMatcher> matchs;

	matchs.insert(pair<string, DeclarationMatcher>("FD1", FD1));

	return matchs;
}