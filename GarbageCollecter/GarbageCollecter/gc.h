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

template<class T> class Iter {
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

	unsigned size() const{
		return length;
	}

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
	bool operator==(Iter itr) {
		return ptr == itr.ptr;
	}
	bool operator!=(Iter itr) {
		return ptr != itr.ptr;
	}
	bool operator<(Iter itr) {
		return ptr < itr.ptr;
	}
	bool operator<=(Iter itr) {
		return ptr <= itr.ptr;
	}
	bool operator>(Iter itr) {
		return ptr > itr.ptr;
	}
	bool operator>=(Iter itr) {
		return ptr >= itr.ptr;
	}

	
	Iter operator+(int n) {
		ptr += n;
		return *this;
	}
	Iter operator-(int n) {
		ptr -= n;
		return *this;
	}
	int operator-(Iter &itr) {
		return ptr - itr.ptr;
	}
};

template<class T> class GCInfo {
public:
	unsigned refCount;
	T *memPtr;
	bool isArray;
	unsigned arraySize;

	GCInfo(T *mPtr, unsigned size = 0) {
		refCount = 1;
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

template<class T> bool operator==(GCInfo<T> &ob1, GCInfo<T> &ob2) {
	return ob1.memPtr == ob2.memPtr;
}

template<class T, int size = 0> class GCPtr {

};