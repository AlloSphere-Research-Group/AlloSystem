// #include "LitheModules/LitheModules.h"
#include "alloLithe/alloLithe.hpp"
#include "Oscillator.hpp"

#define prr(thing) std::cout<<thing<<std::endl;

int main()
{
	Oscillator osc;
	prr(osc.parameters[OSCILLATOR_FREQUENCY]->get());
    return 0;
}
