// #include "LitheModules/LitheModules.h"
#include "alloLithe/alloLithe.hpp"
#include "Oscillator_GUI.hpp"
#include "PatcherGUI.hpp"

#define pr(thing) std::cout<<thing<<std::endl; /// Shorthand for printing a variable. 

int main()
{
	Oscillator_GUI osc;
	pr(Oscillator::moduleID);

	PatcherGUI gui;

	gui << osc;
	gui.openWindow();

    return 0;
}
