#ifndef PATCHER_GUI_HPP
#define PATCHER_GUI_HPP

#include "GLV/glv.h"
#include "GLV/glv_binding.h"
#include <atomic>

class PromptGUI : public glv::GLV
{
public:

	PromptGUI(std::atomic<bool>* result, std::string message, std::vector<std::string> options)
	{
		result_container = result;
		glv::Buttons option_buttons(glv::Rect(30, 60), 2, 1, false, true);
		*this << option_buttons;

		glv::Window win(200,200, "Message", (glv::GLV*)this);
		glv::Application::run();

	}

	std::atomic<bool>* result_container;

};

class PatcherGUI : public glv::GLV
{
public:
	void openWindow(void)
	{
		glv::Window win(800,600, "LitheModular", (glv::GLV*)this);
		glv::Application::run();
	}

	virtual bool onEvent(glv::Event::t e, glv::GLV& glv)
	{
		switch(e){
		// case Event::MouseDrag:	return false;
		// case Event::MouseDown:	return false;
		case glv::Event::KeyDown:	keyboard_event_handler(glv); 
		}
		return true;	// bubble unrecognized events to parent
	}

	void keyboard_event_handler(glv::GLV& glv)
	{
		switch(glv.keyboard().key())
		{
			case 'q': glv::Application::quit();
			case 'w': open_prompt_window("poop", {"poop", "poop"});
		}
	}

	void open_prompt_window(std::string message, std::vector<std::string> options)
	{
		PromptGUI(&result, message, options);
	}

	std::atomic<bool> result;
};


#endif // PATCHER_GUI_HPP