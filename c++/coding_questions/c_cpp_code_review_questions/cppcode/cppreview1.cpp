#include <iostream>
#include <cstring>

using namespace std;

class A
{
public:
    void printA()
    {
        cout << endl << "Printing A";
    }

    void printX()
    {
        cout << endl << "Printing X" << x;
    }

private:
    int x;
};


class B
{
public:
    void printB()
    {
        cout << endl << "Printing B";
    }

    virtual void printY()
    {
        cout << endl << "Printing Y" << y;
    }

private:
    int y;
};


int main (void)
{
	A* aObj = NULL;
    B* bObj = NULL;
    
    aObj->printA();
    aObj->printX();

    bObj->printB();
    bObj->printY();

    return 0;
}
