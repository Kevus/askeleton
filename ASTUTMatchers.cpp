#include "ASTUTMatchers.hpp"

/***************************************************
 ** DECLARATION OF MATCHERS
 ***************************************************/

DeclarationMatcher FD1 =
	functionDecl(
		unless(isImplicit()),
		unless(returns(voidType()))
	).bind("FD1");

DeclarationMatcher MD1 =
	cxxMethodDecl(
		unless(isImplicit()),
		isPublic(),
		unless(returns(voidType()))
	).bind("MD1");

DeclarationMatcher CT1 =
	cxxRecordDecl(
		unless(isImplicit()),
		isStruct()
		).bind("CT1");


//We will reunite and insert into the match map here
map<string, DeclarationMatcher> createMapMatchers()
{
	map<string, DeclarationMatcher> matchs;

	matchs.insert(pair<string, DeclarationMatcher>("FD1", FD1));
	matchs.insert(pair<string, DeclarationMatcher>("MD1", MD1));
	matchs.insert(pair<string, DeclarationMatcher>("CT1", CT1));

	return matchs;
}