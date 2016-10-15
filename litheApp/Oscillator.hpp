#ifndef OSCILLATOR
#define OSCILLATOR
#include <iostream>
#include "alloLithe/alloLithe.hpp"
#include "Gamma/Oscillator.h"

enum OscillatorParams 
{
	OSCILLATOR_FREQUENCY,
	OSCILLATOR_AZ,
	OSCILLATOR_EL,
	OSCILLATOR_DIST,
	OSCILLATOR_NUM_PARAMS
};

class Oscillator : public al::Node<OSCILLATOR_NUM_PARAMS>
{
public:
	enum OscillatorInlets
	{
		FM, 
		NUM_INLETS
	};

	enum OscillatorOutlets
	{
		SINE,
		NUM_OUTLETS
	};

	Oscillator(void) : al::Node<OSCILLATOR_NUM_PARAMS>(NUM_INLETS, NUM_OUTLETS) 
	{
		init_parameters();
	}

	virtual void init_parameters(void) override
	{
		parameters[OSCILLATOR_FREQUENCY] = new al::Parameter("frequency", std::to_string(getID()), 440.0, "", 0.0, 20000.0 );
		parameters[OSCILLATOR_AZ] = new al::Parameter("az", std::to_string(getID()), 0, "", -1.0, +1.0 );
		parameters[OSCILLATOR_EL] = new al::Parameter("el", std::to_string(getID()), 0, "", -1.0, +1.0 );
		parameters[OSCILLATOR_DIST] = new al::Parameter("dist", std::to_string(getID()), 0, "", -1.0, +1.0 );
	}

	void updateFreqs(float frequency)
	{
		sine.freq(parameters[OSCILLATOR_FREQUENCY]->get());
	}

	virtual void DSP(void) override
	{
		getOutlet(SINE).setSample( lithe::Sample( sine(), parameters[OSCILLATOR_AZ]->get(), parameters[OSCILLATOR_EL]->get(), parameters[OSCILLATOR_DIST]->get()) );
	}

private:
	gam::Sine<> sine;
	// gam::Square<> square;
	// gam::Saw<> saw;
	// gam::DWO<> dwo; /// For triangle wave
};

#endif // OSCILLATOR