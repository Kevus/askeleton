#include <string>
#include <iostream>

#include <vector>
#include <list>
#include <map>

using namespace std;

enum DaysOfTheWeek
{
	Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday
};

struct customType
{
  int number;
  std::string name;
  double other_num;
};

class FirstClass
{
public:
  FirstClass() {}
  FirstClass(int i_param, string s_param)  {public_param = i_param;}

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

  list<int> allLists(list<int> l_param)
  {
    return l_param;
  }

  vector<int> allVector(vector<int> v_param)
  {
    return v_param;
  }

  map<int,int> allMap(map<int,int> m_param)
  {
    return m_param;
  }

    char iObtainAString(std::string s_param)
  {
    return 'i';
  }

int public_param;

private:
  double privateMethod(std::string s_param2)
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

int conditionalMethod(int a, float f, bool c, char x) {
  if(a == 2)
    a = 1;

  if(a < 5)
    if(a != 8)
    {}

  if(c == true){}
  if(x == 'd'){}
  if(f > 8.5){}



	return a;
}

int outsideMethod2(int b) {
  int a;
  if(3 == b){}
  if(b != 8){}
  if(a == b){}
  return 1;
}



customType ctypetest() {
  customType a;
  a.number = 1;
  a.name = "kevin";
  a.other_num = 2.0;
  return a;
}

int& ihavepointers(int& a)
{
  return a;
}

string methodwithswitch(int a)
{
  switch(a) {
    case 1: return "hola";
    case 2: return "adios";
    default: return "kevin";
  }
}

/*const char* TestConstChar(const char* c)
{
  return c;
}*/

const int a(const int b)
{
  return b;
}

/*int functionwithconstchar(const char* c)
{
  return 0;
}

int* KevinValle(int* a)
{
  return a;
}

int Gomez(const int* a)
{
  return 0;
}*/