#include "gc.h"
template<class T> class a {
public:
	static void print() {
		cout << typeid(T).name() << endl;
	}
};

int main() {
	a<int> x;
	x.print();
	system("PAUSE");
}