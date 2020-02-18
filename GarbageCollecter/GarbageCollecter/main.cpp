#include "gc.h"

int main() {
	GCPtr<int> p = new int(1);
	GCPtr<int> q;
	//GCPtr<int> t = p;

	p = new int(2);
	q = p;

	GCPtr<int>::collect();
	return 0;
}