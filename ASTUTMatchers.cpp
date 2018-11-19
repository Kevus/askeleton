#include "ASTUTMatchers.hpp"

/***************************************************
 ** DECLARATION OF MATCHERS
 ***************************************************/

DeclarationMatcher FD1 =
	functionDecl(
		unless(returns(voidType()))
	).bind("FD1");

DeclarationMatcher MD1 =
	cxxMethodDecl(
		isPublic(),
		unless(returns(voidType()))
	).bind("MD1");


//We will reunite and insert into the match map here
map<string, DeclarationMatcher> createMapMatchers()
{
	map<string, DeclarationMatcher> matchs;

	matchs.insert(pair<string, DeclarationMatcher>("FD1", FD1));
	matchs.insert(pair<string, DeclarationMatcher>("MD1", MD1));

	return matchs;
}