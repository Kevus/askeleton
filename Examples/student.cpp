#include <string>

using namespace std;

enum DeclaredEnum
{
	Lunes, Martes, Miercoles
};

class Example{

    public:
       Example(): a(0) { }
       void mainMethod(int p){
          a = a * 2 + p;
       }
       int referencedMethod(unsigned int param1, float param2){
          return a * 2;	
       }
       int notAConstructor(double param3, string param4){
        return 1;
       }
    private:
       int privateMethod() {
	  return a;
       }

       int a;
};

int outsideMethod() {
	return 1;
}
