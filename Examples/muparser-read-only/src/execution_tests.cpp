#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "test_functions.h"
#include "muParserTest.h"
#include <map>
#include <iostream>

int main(){
	try{
		//beforeTests();
		cout << "Ejecución de las pruebas:" << endl;
		TestFunctions tf(false,2,true);
		
		std::map<unsigned int, TPrueba> myMap;
		myMap[1] = &mu::Test::ParserTester::TestNames;
		myMap[2] = &mu::Test::ParserTester::TestSyntax;
		myMap[3] = &mu::Test::ParserTester::TestMultiArg;
		myMap[4] = &mu::Test::ParserTester::TestPostFix;
		myMap[5] = &mu::Test::ParserTester::TestExpression;
		myMap[6] = &mu::Test::ParserTester::TestInfixOprt;
		myMap[7] = &mu::Test::ParserTester::TestBinOprt;
		myMap[8] = &mu::Test::ParserTester::TestVarConst;
		myMap[9] = &mu::Test::ParserTester::TestInterface;
		myMap[10] = &mu::Test::ParserTester::TestException;
		myMap[11] = &mu::Test::ParserTester::TestStrArg;
		myMap[12] = &mu::Test::ParserTester::TestIfThenElse;
		myMap[13] = &mu::Test::ParserTester::TestBulkMode;
		
		for(int i=1; i<=myMap.size(); i++){
			tf.test_case(myMap[i]);
			if(tf.has_failed() && !tf.continue_execution()){
				//cout << "Test fallado: " << i << endl;
				 break;			
			}
		}
		
		//afterTests();
		cout << "Fin de las pruebas." << endl;
	} 
	catch(char *s){                 
		cout << "Excepción " << s << endl;
	}
	catch(...){
		cout << "Excepción en las pruebas" << endl;
	}
	return 0;
}
