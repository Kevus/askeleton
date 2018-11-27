#include <string>
#include <iostream>

using namespace std;

enum DaysOfTheWeek
{
	Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday
};

class FirstClass
{
public:
  FirstClass() {}
  FirstClass(int i_param, string s_param)  {}

  void invisibleForASTUT(int p)
  {
    //DELETEprintf("You shouldn't see me!");
  }

  int rareMethod(unsigned int ui_param, float a_param, double d_param)
  {
    double variableDefinedByKevin = privateMethod("hola");
    return 240294;
  }

  char whoReturnsAChar()
  {
    return 'i';
  }

    char iObtainAString(string s_param)
  {
    return 'i';
  }

int public_param;

private:
  double privateMethod(string s_param2)
  {
    return 3.0;
  }

int private_param;

protected:
  int protectedMethod()
  {
    return 1;
  }

int protected_param;
};

class SecondClass : FirstClass
{
  //No tags
  int newMethod()
  {
    return 1;
  }

public:
  int * testPointer1(int * a)
  {
    return a;
  }

  int* testPointer2(int *b)
  {
    return b;
  } 

  int* testPointer3(int* c)
  {
    return c;
  }
};

int outsideMethod() {
	return 1;
}

int outsideMethod2() {
  return 1;
}