#ifndef OSCILLATOR
#define OSCILLATOR
#include <iostream>
#include "alloLithe/alloLithe.hpp"
#include "Gamma/Oscillator.h"
#include "allolithe/allolithe.hpp"

#define MODULE_NAME "Oscillator"

class Oscillator : public al::Node
{
public:
	enum OscillatorParams : int
	{
		FREQUENCY,
		AZ,
		EL,
		DIST,
		NUM_PARAMS
	};
	
	enum OscillatorInlets : int
	{
		FM, 
		NUM_INLETS
	};

	enum OscillatorOutlets : int {
		SINE,
		SQUARE, 
		SAW, 
		TRIANGLE,
		NUM_OUTLETS
	};

	Oscillator(void) : al::Node(NUM_INLETS, NUM_OUTLETS, NUM_PARAMS) 
	{
		init_parameters();
	}

	void init_parameters(void)
	{
		parameters.push_back( new al::Parameter("frequency", std::to_string(getID()), 440.0, "", 0.0, 20000.0 ));
		parameters.push_back( new al::Parameter("az", std::to_string(getID()), 0, "", -1.0, +1.0 ));
		parameters.push_back( new al::Parameter("el", std::to_string(getID()), 0, "", -1.0, +1.0 ));
		parameters.push_back( new al::Parameter("dist", std::to_string(getID()), 0, "", -1.0, +1.0 ));
	}

	void updateFreqs(float frequency)
	{
		float new_freq = parameters[FREQUENCY]->get();
		sine.freq(new_freq);
		square.freq(new_freq);
		saw.freq(new_freq);
		dwo.freq(new_freq);
	}

	virtual void DSP(void) override
	{
		/// TODO: Add modulation code here. 
		getOutlet(SINE).setSample( lithe::Sample( sine(), parameters[AZ]->get(), parameters[EL]->get(), parameters[DIST]->get()) );
		getOutlet(SQUARE).setSample( lithe::Sample( square(), parameters[AZ]->get(), parameters[EL]->get(), parameters[DIST]->get()) );
		getOutlet(SAW).setSample( lithe::Sample( saw(), parameters[AZ]->get(), parameters[EL]->get(), parameters[DIST]->get()) );
		getOutlet(TRIANGLE).setSample( lithe::Sample( dwo(), parameters[AZ]->get(), parameters[EL]->get(), parameters[DIST]->get()) );
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
int Oscillator::moduleID = al::REGISTER_MODULE(Oscillator::moduleName, OscillatorFactory);

#undef MODULE_NAME
#endif // OSCILLATOR