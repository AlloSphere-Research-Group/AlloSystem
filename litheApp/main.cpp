// #include "LitheModules/LitheModules.h"
#include "allolithe/allolithe.hpp"
#include "Oscillator_GUI.hpp"

#define pr(thing) std::cout<<thing<<std::endl; /// Shorthand for printing a variable. 

int main()
{
	Oscillator_GUI osc;
	pr(Oscillator::moduleID);

	al::PatcherGUI gui;

	gui << osc;
	gui.openWindow();

    return 0;
}
