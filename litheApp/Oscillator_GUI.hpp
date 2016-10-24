#ifndef OSCILLATOR_GUI_HPP
#define OSCILLATOR_GUI_HPP

// #include "GLV/glv.h"
// #include  "GLV/glv_binding.h"
#include "alloGLV/al_ParameterGUI.hpp"
// #include "GLV/glv_util.h"

#include "allolithe/allolithe.hpp"
#include "Oscillator.hpp"

class Oscillator_GUI : public glv::View
{
public:
	Oscillator_GUI(void)
	{
		int id; 
		try
		{
			nodeID = al::SoundEngine().instantiateModule(Oscillator::moduleID);
		}
		catch(...)
		{
			throw std::range_error("Oscillator module not registered");
		}

		glv::View(glv::Rect(100,100, 600,400));
		al::Node* node_ref = al::Node::getNodeRef(nodeID);
		for(int i=0; i< node_ref->parameters.size(); ++i )
		{
			// (*this) << *node_ref->parameters[i];
		}
	}

	virtual bool onEvent(glv::Event::t e, glv::GLV& glv)
	{
		switch(e){
		// case Event::MouseDrag:	return false;
		// case Event::MouseDown:	return false;
		// case glv::Event::KeyDown:	return true; 
			default:;
		}
		return true;	// bubble unrecognized events to parent
	}

	~Oscillator_GUI()
	{
	}

	int nodeID = -1;
	// al::ParameterGUI parameterGUI;
};


#endif // OSCILLATOR_GUI_HPP