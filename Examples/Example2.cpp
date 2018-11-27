//Example2.cpp
#include <iostream>
#include <string>
#define DEFINITION 10
#define OTHERDEFINITION 20

using namespace std;

class A
{
	public:
		void NothingToDo() {}

		int B() {
			return 1;
		}

		int C(int par) {
			return par;
		}

		static void staticFunction(int parameter) 
		{}

		const int constFunction() {
			return 1;
		}

	private:
		void Priv() {}
		int a;
		char b;
};

class B : A {
public:	
	string imfromB()
	{
		return "hello";
	}

};

int OutsideFunction(int par);
