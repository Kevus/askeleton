#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "test_functions.h"
#include "tests.h"
#include <map>
#include <iostream>

int main(){
	try{
		beforeTests();
		cout << "Ejecución de las pruebas:" << endl;
		TestFunctions tf(false,8,true);
		
		std::map<unsigned int, TPrueba> myMap;
		myMap[1] = test1;
		myMap[2] = test2;
		myMap[3] = test3;
		myMap[4] = test4;
		myMap[5] = test5;
		myMap[6] = test6;
		myMap[7] = test7;
		myMap[8] = test8;
		myMap[9] = test9;
		myMap[10] = test10;
		myMap[11] = test11;
		myMap[12] = test12;
		myMap[13] = test13;
		myMap[14] = test14;
		myMap[15] = test15;
		myMap[16] = test16;
		myMap[17] = test17;
		myMap[18] = test18;
		myMap[19] = test19;
		myMap[20] = test20;
		myMap[21] = test21;
		myMap[22] = test22;
		myMap[23] = test23;
		myMap[24] = test24;
		myMap[25] = test25;
		myMap[26] = test26;
		myMap[27] = test27;
		myMap[28] = test28;
		myMap[29] = test29;
		myMap[30] = test30;
		myMap[31] = test31;
		myMap[32] = test32;
		myMap[33] = test33;
		myMap[34] = test34;
		myMap[35] = test35;
		myMap[36] = test36;
		myMap[37] = test37;
		myMap[38] = test38;
		myMap[39] = test39;
		myMap[40] = test40;
		myMap[41] = test41;
		myMap[42] = test42;
		myMap[43] = test43;
		myMap[44] = test44;
		myMap[45] = test45;
		myMap[46] = test46;
		myMap[47] = test47;
		myMap[48] = test48;
		myMap[49] = test49;
		myMap[50] = test50;
		myMap[51] = test51;
		myMap[52] = test52;
		myMap[53] = test53;
		myMap[54] = test54;
		myMap[55] = test55;
		myMap[56] = test56;
		myMap[57] = test57;	
	
		for(int i=1; i<=myMap.size(); i++){
			tf.test_case(myMap[i]);
			if(tf.has_failed() && !tf.continue_execution()){
                                //cout << "Test fallado: " << i << endl;
                                break;
                        }
		}
		
		afterTests();
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
