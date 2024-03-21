#ifndef FIRSTCLASS_HPP
#define FIRSTCLASS_HPP

#include <string>
#include <list>
#include <vector>
#include <map>

class FirstClass {
public:
    FirstClass();
    FirstClass(int i_param, std::string s_param);
    void invisibleForASTUT(int p);
    int rareMethod(unsigned int ui_param, float a_param, double d_param);
    char whoReturnsAChar();
    std::list<int> allLists(std::list<int> l_param);
    std::vector<int> allVector(std::vector<int> v_param);
    std::map<int,int> allMap(std::map<int,int> m_param);
    int funcion_puntero(unsigned long *valor);
    char iObtainAString(std::string s_param);
    
    std::string public_param;

protected:
    int protectedMethod();

private:
    double privateMethod(std::string s_param2);
};

class SecondClass {
public:
    int newMethod();
    int* testPointer1(int* a);
    int* testPointer2(int* b);
    int* testPointer3(int* c);
};

int conditionalMethod(int a, float f, bool c, char x);
int outsideMethod2(int b);
struct customType {
    int number;
    std::string name;
    double other_num;
};
customType ctypetest();
int& ihavepointers(int& a);

std::string methodwithswitch(int a);
const int a(const int b);
int b(char* c);

#endif // FIRSTCLASS_HPP

