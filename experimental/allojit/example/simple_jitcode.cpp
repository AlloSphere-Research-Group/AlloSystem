/*
Code to be compiled by JIT engine:

TODO: why do statics cause EXC_BAD_ACCESS at shutdown?
 */


#include <iostream>

class Foo {
public:
	Foo(int argc = 0, char * const argv[] = NULL) {
		for (int i=0; i<argc; i++) {
			printf("%d %s\n", i, argv[i]);
		} 
	}
	virtual ~Foo() { printf("death of Foo\n"); }
};

class Bar : public Foo {
public:

	Bar(int argc, char * const argv[]) : Foo(argc, argv) {
		printf("Created a Bar! %p\n", this);
	}
	virtual ~Bar() { printf("death of Bar\n"); }
};

Bar bar(0, NULL);

int main (int argc, char * const argv[]) { 
    std::cout << "Hello, Jitted World!\n";
    Bar b(argc, argv);
	return 0; 
}