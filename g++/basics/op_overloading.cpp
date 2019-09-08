
#include <iostream>

using namespace std;

class Int {
	private:
		int _val;
	public:
		Int( int val = 0) : _val(val){}

		void print() { cout << _val << endl; }

		Int& operator ++ (){         // pre increment
			_val++;
			return *this;
		}
		Int operator ++ (int dummy){ // post increment
			Int tmp = *this;
			++(*this);
			return tmp;
		}
		Int operator + (const Int & i) {
			cout << "operator + " << endl;
			return Int( this->_val + i._val);
			//return 0;
		}
};


int main() {
	Int i(100);
	Int j(200);

	i.print();
	(++i).print();
	(i++).print();
	i.print();

	Int k = i + j;
	k.print();

	return 0;
}
