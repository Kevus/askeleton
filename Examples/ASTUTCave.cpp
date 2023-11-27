#include "ASTUTCave.hpp"

FirstClass::FirstClass()
{
    // Default constructor
}

FirstClass::FirstClass(int i_param, std::string s_param)
{
    public_param = i_param;
}

void FirstClass::invisibleForASTUT(int p)
{
    //DELETEprintf("You shouldn't see me!");
}

int FirstClass::rareMethod(unsigned int ui_param, float a_param, double d_param)
{
    double variableDefinedByKevin = privateMethod("hola");
    return 240294;
}

char FirstClass::whoReturnsAChar()
{
    return 'i';
}

std::list<int> FirstClass::allLists(std::list<int> l_param)
{
    return l_param;
}

std::vector<int> FirstClass::allVector(std::vector<int> v_param)
{
    return v_param;
}

std::map<int,int> FirstClass::allMap(std::map<int,int> m_param)
{
    return m_param;
}

char FirstClass::iObtainAString(std::string s_param)
{
    return 'i';
}

double FirstClass::privateMethod(std::string s_param2)
{
    return 3.0;
}

int FirstClass::protectedMethod()
{
    return 1;
}

int SecondClass::newMethod()
{
    return 1;
}

int* SecondClass::testPointer1(int* a)
{
    return a;
}

int* SecondClass::testPointer2(int* b)
{
    return b;
}

int* SecondClass::testPointer3(int* c)
{
    return c;
}

int conditionalMethod(int a, float f, bool c, char x)
{
    if (a == 2)
        a = 1;

    if (a < 5)
        if (a != 8)
        {}

    if (c == true){}
    if (x == 'd'){}
    if (f > 8.5){}

    return a;
}

int outsideMethod2(int b)
{
    int a;
    if (3 == b){}
    if (b != 8){}
    if (a == b){}
    return 1;
}

customType ctypetest()
{
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

 int funcion_puntero(unsigned long *valor) { *valor = 4; return *valor; }

std::string methodwithswitch(int a)
{
    switch (a)
    {
        case 1: return "hola";
        case 2: return "adios";
        default: return "kevin";
    }
}

const int a(const int b)
{
    return b;
}

int b(char* c)
{
    return 0;
}
