#include<iostream>
#include<list>
#include<typeinfo>
#include<cstdlib>
#include<stdio.h>

using namespace std;

// define DISPLAY to watch the action of the garbage collector
#define DISPLAY

class OutOfRangeExc {
	// Add action
};

template<class T>
class Iter {
	T *ptr;
	T *end;
	T *begin;
	unsigned length;
public:
	Iter() {
		ptr = end = begin = NULL;
		length = 0;
	}
	Iter(T *p, T *first, T *last) {
		ptr = p;
		begin = first;
		end = last;
		length = last - first;
	}

	unsigned size() const { return length; }

	T &operator*() {
		if ((ptr < begin) || (ptr > end)) {
			throw OutOfRangeExc();
		}
		return *ptr;
	}
	T *operator->() {
		if ((ptr < begin) || (ptr > end)) {
			throw OutOfRangeExc();
		}
		return ptr;
	}
	T &operator[](int i) {
		if ((i < 0) || (i >= length)) {
			throw OutOfRangeExc();
		}
		return ptr[i];
	}

	Iter operator++() {
		ptr++;
		return *this;
	}
	Iter operator--() {
		ptr--;
		return *this;
	}
	Iter operator++(int notused) {
		T *tmp = ptr;
		ptr++;
		return Iter(tmp, begin, end);
	}
	Iter operator--(int notused) {
		T *tmp = ptr;
		ptr--;
		return Iter(tmp, begin, end);
	}
	bool operator==(Iter itr) { return ptr == itr.ptr; }
	bool operator!=(Iter itr) { return ptr != itr.ptr; }
	bool operator<(Iter itr) { return ptr < itr.ptr; }
	bool operator<=(Iter itr) { return ptr <= itr.ptr; }
	bool operator>(Iter itr) { return ptr > itr.ptr; }
	bool operator>=(Iter itr) { return ptr >= itr.ptr; }

	Iter operator+(int n) {
		ptr += n;
		return *this;
	}
	Iter operator-(int n) {
		ptr -= n;
		return *this;
	}
	int operator-(Iter &itr) { return ptr - itr.ptr; }
};

template<class T>
class GCInfo {
public:
	unsigned refcount;
	T *memPtr;
	bool isArray;
	unsigned arraySize;

	GCInfo(T *mPtr, unsigned size = 0) {
		refcount = 1;
		memPtr = mPtr;
		if (size != 0) {
			isArray = true;
		}
		else {
			isArray = false;
		}
		arraySize = size;
	}

};

template<class T>
bool operator==(const GCInfo<T> &ob1, const GCInfo<T> &ob2) {
	return ob1.memPtr == ob2.memPtr;
}

template<class T, int size = 0>
class GCPtr {
	static list<GCInfo<T> > gclist;
	static bool init;

	T *addr;
	bool isArray;
	unsigned arraySize;

	typename list<GCInfo<T> >::iterator findPtrInfo(T *ptr) {
		list<GCInfo<T> >::iterator p;

		for (p = gclist.begin(); p != gclist.end(); p++) {
			if (p->memPtr == ptr) {
				return p;
			}
		}

		return p;
	}
public:
	typedef Iter<T> GCiterator;

	GCPtr(T *t = NULL) {
		if (init) atexit(shutdown);
		init = false;

		list<GCInfo<T> >::iterator p;
		p = findPtrInfo(t);

		if (p != gclist.end()) {
			p->refcount++;
		}
		else {
			GCInfo<T> gcObj(t, size);
			gclist.push_front(gcObj);
		}

		addr = t;
		arraySize = size;
		if (arraySize > 0) isArray = true;
		else isArray = false;

#ifdef DISPLAY
		cout << "Constructing GCPtr. ";
		if (isArray) {
			cout << "Size is " << arraySize;
		}
		cout << endl;
#endif // DISPLAY
	}
	GCPtr(const GCPtr &ob) {
		list<GCInfo<T> >::iterator p;

		p = findPtrInfo(ob.addr);
		p->refcount++;
		addr = ob.addr;
		arraySize = ob.arraySize;
		if (arraySize > 0) isArray = true;
		else isArray = false;

#ifdef DISPLAY
		cout << "Constructing copy. ";
		if (isArray) {
			cout << "Size is " << arraySize;
		}
		cout << endl;
#endif // DISPLAY
	}
	~GCPtr() {
		list<GCInfo<T> >::iterator p;

		p = findPtrInfo(addr);
		if(p->refcount) p->refcount--;

#ifdef DISPLAY
		cout << "GCPtr going out of scope" << endl;
#endif // DISPLAY
		collect();
	}

	T *operator=(T *t) {
		list<GCInfo<T> >::iterator p;

		p = findPtrInfo(addr);
		p->refcount--;

		p = findPtrInfo(t);
		if (p != gclist.end()) {
			p->refcount++;
		}
		else {
			GCInfo<T> gcObj(t, size);
			gclist.push_front(gcObj);
		}

		addr = t;
		return t;
	}
	GCPtr &operator=(GCPtr &rv) {
		list<GCInfo<T> >::iterator p;

		p = findPtrInfo(addr);
		p->refcount--;

		p = findPtrInfo(rv.addr);
		p->refcount++;

		addr = rv.addr;
		return rv;
	}
	T &operator*() { return *addr; }
	T *operator->() { return addr; }
	T &operator[](int i) { return addr[i]; }
	
	Iter<T> begin() {
		int size;

		if (isArray) size = arraySize;
		else size = 1;

		return Iter<T>(addr, addr, addr + size);
	}
	Iter<T> end() {
		int size;

		if (isArray) size = arraySize;
		else size = 1;

		return Iter<T>(addr + size, addr, addr + size);
	}

	static bool collect() {
		bool memfreed = false;
#ifdef DISPLAY
		cout << "Before garbage collection for ";
		showList();
#endif // DISPLAY

		list<GCInfo<T> >::iterator p;
		do {
			for (p = gclist.begin(); p != gclist.end(); p++) {
				if (p->refcount > 0) continue;

				memfreed = true;
				
				if (p->memPtr) {
					if (p->isArray) {
#ifdef DISPLAY
						cout << "Deleting array of size " << p->arraySize << endl;
#endif // DISPLAY

						delete[] p->memPtr;
					}
					else {
#ifdef DISPLAY
						cout << "Deleting:" << *(T *) p->memPtr << endl;
#endif // DISPLAY
						delete p->memPtr;
					}
				}
				p = gclist.erase(p);
				break;
			}
		} while (p != gclist.end());
#ifdef DISPLAY
		cout << "After garbage collection for ";
		showList();
#endif // DISPLAY
		return memfreed;
	}
	static int gclistSize() { return gclist.size(); }
	static void showList() {
		list<GCInfo<T> >::iterator p;

		cout << "gclist<" << typeid(T).name() << ", " << size << ">:" << endl;
		cout << "memPtr refcount value" << endl;

		if (gclist.begin() == gclist.end()) {
			cout << " -- Empty -- \n\n";
			return;
		}

		for (p = gclist.begin(); p != gclist.end(); p++) {
			cout << "[" << (void *)p->memPtr << "]"
				<< " " << p->refcount << " ";
			if (p->memPtr) cout << " " << *p->memPtr;
			else cout << " ---";
			cout << endl;
		}
		cout << endl;
	}
	static void shutdown() {
		if (gclistSize() == 0) return;

		list<GCInfo<T> >::iterator p;

		for (p = gclist.begin(); p != gclist.end(); p++) {
			p->refcount = 0;
		}

#ifdef DISPLAY
		cout << "Before collecting for shutdown() for "
			<< typeid(T).name() << endl;
#endif // DISPLAY
		collect();

#ifdef DISPLAY
		cout << "After collecting for shutdown() for "
			<< typeid(T).name() << endl;
#endif // DISPLAY
	}
};

template<class T, int size>
list<GCInfo<T> > GCPtr<T, size>::gclist;

template<class T, int size>
bool GCPtr<T, size>::init = true;