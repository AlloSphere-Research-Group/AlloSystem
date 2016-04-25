#ifndef INCLUDE_AL_PARAMETERGUI_HPP
#define INCLUDE_AL_PARAMETERGUI_HPP

/*	AlloSystem --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	GUI generation from group of al::Parameter

	File author(s):
	Andres Cabrera 2016, andres@mat.ucsb.edu
*/

#include <vector>
#include <mutex>
#include <iostream>

#include "alloGLV/al_ControlGLV.hpp"
#include "allocore/system/al_Parameter.hpp"
#include "allocore/io/al_App.hpp"

#include <GLV/glv.h>

namespace al {

class ParameterGUI {
public:
	ParameterGUI() : mGui("><"){
		mGui.colors().set(glv::StyleColor::SmokyGray);
		mGui.colors().back.set(0.05, 0.95);
		mGui.colors().selection.set(glv::HSV(0.67,0.5,0.5));
		mGui.enable(glv::DrawBack | glv::DrawBorder | glv::Controllable);
		mDetachableGUI << mGui;
	}

	void setParentWindow(Window& win) {
		mDetachableGUI.parentWindow(win);
	}

	void setParentApp(App *app) {
		App::Windows ws = app->windows();
		if (ws.size() > 0) {
			setParentWindow(*(ws[0]));
		} else {
			std::cout << "ERROR: Can't attach to app. No windows available." << std::endl;
		}
	}

	ParameterGUI &addParameter(Parameter &parameter) {
		int numInt = 1 + ceil(log10(max(fabs(parameter.max()), fabs(parameter.min()))));
		glv::NumberDialer *number  = new glv::NumberDialer(numInt, 6 - numInt,
		                                                  parameter.max(),
		                                                  parameter.min());
		number->attach(ParameterGUI::widgetChangeCallback, glv::Update::Value, &parameter);
		mGui << number << new glv::Label(parameter.getName());
		mGui.arrange();
		parameter.registerProcessingCallback(ParameterGUI::valueChangedCallback, (void *) number);
		return *this;
	}

	static void widgetChangeCallback(const glv::Notification& n) {
		glv::Widget &sender = *n.sender<glv::Widget>();
		Parameter &receiver = *n.receiver<Parameter>();
		double value = sender.getValue<double>();
		receiver.set(value);
	}

	static float valueChangedCallback(float value, void *widget) {
		glv::Widget *w = static_cast<glv::Widget *>(widget);
		if (w->getValue<double>() != value) {
//			w->setValue(value);
		}
		return value;
	}

	/// Add new parameter to GUI
	ParameterGUI &operator << (Parameter& newParam){ return addParameter(newParam); }

	/// Add new parameter to GUI
	ParameterGUI &operator << (Parameter* newParam){ return addParameter(*newParam); }

private:
	glv::Table mGui;
	GLVDetachable mDetachableGUI;
};


}

#endif
