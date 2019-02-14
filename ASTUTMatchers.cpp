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

DeclarationMatcher CC1 =
	cxxConstructorDecl(
		unless(isImplicit())
	).bind("CC1");

/***************************************************
 ** EXPERIMENTAL
 ***************************************************/
/*DeclarationMatcher PD1 =
	callExpr(
		isExpansionInMainFile(),
		callee(expr()),
		hasAncestor(
			recordDecl().bind("caller")
		)
	).bind("callee");*/


/***************************************************
 ** MATCHERS FOR GENERATING DATA
 ***************************************************/
DeclarationMatcher DG1 =
	functionDecl(
		forEachDescendant(
			binaryOperator(
				anyOf(
					hasOperatorName("=="),
					hasOperatorName("!="),
					hasOperatorName(">"),
					hasOperatorName(">="),
					hasOperatorName("<"),
					hasOperatorName("<=")
				)
			).bind("DG1")
		),
		unless(isImplicit())
	).bind("DG1b");

//We will reunite and insert into the match map here
map<string, DeclarationMatcher> createMapMatchers()
{
	map<string, DeclarationMatcher> matchs;

	matchs.insert(pair<string, DeclarationMatcher>("FD1", FD1));
	matchs.insert(pair<string, DeclarationMatcher>("MD1", MD1));
	matchs.insert(pair<string, DeclarationMatcher>("CT1", CT1));
	matchs.insert(pair<string, DeclarationMatcher>("CC1", CC1));

	//matchs.insert(pair<string, DeclarationMatcher>("PD1", PD1));

	matchs.insert(pair<string, DeclarationMatcher>("DG1", DG1));

	return matchs;
}