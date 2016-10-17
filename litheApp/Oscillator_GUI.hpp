#ifndef OSCILLATOR_GUI_HPP
#define OSCILLATOR_GUI_HPP

#include "GLV/glv.h"
#include  "GLV/glv_binding.h"
// #include "GLV/glv_util.h"

#include "SoundEngine.hpp"
#include "Oscillator.hpp"

class Oscillator_GUI : public glv::View
{
public:
	Oscillator_GUI(void)
	{
		int id; 
		try
		{
			id = SoundEngine().instantiateModule(Oscillator::moduleID);
		}
		catch(...)
		{
			id = -1;
			throw std::range_error("Oscillator module not registered");
		}
		nodeID = id;

		glv::View(glv::Rect(100,100, 600,400));

	}

	virtual bool onEvent(glv::Event::t e, glv::GLV& glv)
	{
		switch(e){
		// case Event::MouseDrag:	return false;
		// case Event::MouseDown:	return false;
		// case glv::Event::KeyDown:	return true; 
		}
		return true;	// bubble unrecognized events to parent
	}

	~Oscillator_GUI()
	{
	}

	int nodeID = -1;
};


#endif // OSCILLATOR_GUI_HPP