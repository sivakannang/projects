#include <iostream>
#include <cstring>

using namespace std;

class A
{
public:
    virtual void printA()
    {
        cout << endl << "Printing A";
    }
};

class B
{
public:
    virtual void printB()
    {
        cout << endl << "Printing B";
    }
};


class C : public A, public B
{
public:
    virtual void printC()
    {
        cout << endl << "Printing C";
    }
};

class D : public B, public A
{
public:
    virtual void printD()
    {
        cout << endl << "Printing D";
    }
};

void callPrints1(A* obj)
{
    obj->printA();
    B* bobj = (B*)(obj);
    bobj->printB();
}

void callPrints2(B* obj)
{
    obj->printB();
    A* bobj = (A*)(obj);
    bobj->printA();
}

int main (void)
{
	A* cObj = new C();
    A* dObj = new D();
    B* eObj = new C();
    B* fObj = new D();

    callPrints1(cObj);
    callPrints1(dObj);

    callPrints2(eObj);
    callPrints2(fObj);

    //delete cObj;
    //delete dObj;

    return 0;
}
