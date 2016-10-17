#ifndef OSCILLATOR
#define OSCILLATOR
#include <iostream>
#include "alloLithe/alloLithe.hpp"
#include "Gamma/Oscillator.h"
#include "SoundEngine.hpp"

#define MODULE_NAME "Oscillator"

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
		SQUARE, 
		SAW, 
		TRIANGLE,
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
		float new_freq = parameters[OSCILLATOR_FREQUENCY]->get();
		sine.freq(new_freq);
		square.freq(new_freq);
		saw.freq(new_freq);
		dwo.freq(new_freq);
	}

	virtual void DSP(void) override
	{
		/// TODO: Add modulation code here. 
		getOutlet(SINE).setSample( lithe::Sample( sine(), parameters[OSCILLATOR_AZ]->get(), parameters[OSCILLATOR_EL]->get(), parameters[OSCILLATOR_DIST]->get()) );
		getOutlet(SQUARE).setSample( lithe::Sample( square(), parameters[OSCILLATOR_AZ]->get(), parameters[OSCILLATOR_EL]->get(), parameters[OSCILLATOR_DIST]->get()) );
		getOutlet(SAW).setSample( lithe::Sample( saw(), parameters[OSCILLATOR_AZ]->get(), parameters[OSCILLATOR_EL]->get(), parameters[OSCILLATOR_DIST]->get()) );
		getOutlet(TRIANGLE).setSample( lithe::Sample( dwo(), parameters[OSCILLATOR_AZ]->get(), parameters[OSCILLATOR_EL]->get(), parameters[OSCILLATOR_DIST]->get()) );
	}

	static int moduleID;
	static std::string moduleName;

private:
	gam::Sine<> sine;
	gam::Square<> square;
	gam::Saw<> saw;
	gam::DWO<> dwo; /// For triangle wave
};

/// Factory for Oscillators. Returns the lithe nodeID
int OscillatorFactory(void)
{
	Oscillator* new_instance = new Oscillator;
	return new_instance->getID();
}

std::string Oscillator::moduleName = MODULE_NAME;
int Oscillator::moduleID = REGISTER_MODULE(Oscillator::moduleName, OscillatorFactory);

#undef MODULE_NAME

#endif // OSCILLATOR