//Example.cpp
#include <iostream>

int global;

void myPrint(int param) {
	if (param == 1)
		//DELETEprintf("param is 1");
	for (int i = 0; i < 10; i++) {
		global += 1;
	}
}

int main(int argc, const char **argv) {
	int param = 1;
	myPrint(param);
	return 0;
}
