#include "allocore/al_Allocore.hpp"
using namespace al;

int main(){
	Window win;
	win.create();
	Main::get().interval(1);
	win.startLoop();
	return 0;
}
