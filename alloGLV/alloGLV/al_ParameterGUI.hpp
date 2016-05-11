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
#include <algorithm>
#include <memory>
#include <map>

#include "alloGLV/al_ControlGLV.hpp"
#include "allocore/ui/al_Parameter.hpp"
#include "allocore/ui/al_Preset.hpp"
#include "allocore/io/al_App.hpp"

#include <GLV/glv.h>

namespace al {

class PresetButtons : public glv::Buttons {
public:
	PresetButtons(const glv::Rect& r=glv::Rect(), int nx=1, int ny=1,
	              bool momentary=false, bool mutExc=false,
	              glv::SymbolFunc on=glv::draw::rectangle, glv::SymbolFunc off=0) :
	    glv::Buttons(r, nx, ny, momentary, mutExc, on, off),
	    mStore(false)
	{
	}

	~PresetButtons () {
	}

	void setPresetHandler(PresetHandler &presetHandler) {
		mPresetHandler = &presetHandler;
	}

	virtual bool onAssignData(glv::Data& d, int ind1, int ind2)
	{
		d.set(true); // Always keep one button on
		int index = ind1 + (ind2 *this->sizeX());
		//		std::cout << index << std::endl;
		if (mStore){
			if (presetName == "") {
				presetName = std::to_string(index);
			}
			mPresetHandler->storePreset(index, presetName);
		} else {
			presetName = mPresetHandler->recallPreset(index);
		}
		glv::Buttons::onAssignData(d, ind1, ind2);
		return true;
	}

	bool mStore;
	std::string presetName;

private:
	PresetHandler *mPresetHandler;
	glv::Button *mStoreButton;
};


class ParameterGUI {
public:
	ParameterGUI() :
	    mBox(glv::Direction::S),
	    mPresetButtons(glv::Rect(), 10, 4, false, true),
	    mPresetHandler(nullptr),
	    mButtonScale(1.0f)
	{
		mPresetButtons.enable(glv::Property::SelectOnDrag);
		mPresetButtons.width(200);
		mBox.colors().set(glv::StyleColor::SmokyGray);
		mBox.colors().back.set(0.05, 0.95);
		mBox.colors().selection.set(glv::HSV(0.67,0.5,0.5));
		mBox.enable(glv::Controllable);
		mDetachableGUI << mBox;
	}

	~ParameterGUI() {
		for(WidgetWrapper *w: mWrappers) {
			delete w;
		}
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

	GLVDetachable &getDetachableView() {return mDetachableGUI;}

	ParameterGUI &addParameter(Parameter &parameter);

	ParameterGUI &registerPresetHandler(PresetHandler &handler);

	static void presetSavedInButton(const glv::Notification &n) {
		glv::Button *button = static_cast<glv::Button *>(n.receiver());
		button->setValue(false);
	}

	static void presetChangeCallback(int index, void *sender,void *userData)
	{
		ParameterGUI *gui = static_cast<ParameterGUI *>(userData);
		glv::Data &d = gui->mPresetButtons.data();
		const int *shape = d.shape();
		for (int x = 0; x < shape[0]; ++x) {
			for (int y = 0; y < shape[1]; ++y) {
				d.assign<int>(false, x, y); // Must manually set them all off...
			}
		}
		gui->mPresetButtons.presetName = gui->mPresetHandler->availablePresets()[index];
		d.assign<int>(true, index); // Do it this way to avoid the GUi triggering callbacks.

//		std::cout << "preset Change callback" << std::endl;
	}

	static void widgetChangeCallback(const glv::Notification& n) {
		glv::Widget &sender = *n.sender<glv::Widget>();
		WidgetWrapper &receiver = *n.receiver<WidgetWrapper>();
		receiver.lock->lock();
		double value = sender.getValue<double>();
		receiver.lock->unlock();
		receiver.parameter->setNoCalls(value);
	}

	static void valueChangedCallback(float value, void *sender, void *wrapper) {
		WidgetWrapper *w = static_cast<WidgetWrapper *>(wrapper);
		w->lock->lock(); // Is the setting of the value from the mouse also protected? Do we need to protect that?
		glv::Data &d = w->widget->data();
		d.assign<double>(value); // We need to assign this way to avoid triggering callbacks.
		w->lock->unlock();
	}

	void SetButtonBoxScale(float scale) {mButtonScale = scale;}

	/// Add new parameter to GUI
	ParameterGUI &operator << (Parameter& newParam){ return addParameter(newParam); }

	/// Add new parameter to GUI
	ParameterGUI &operator << (Parameter* newParam){ return addParameter(*newParam); }

	/// Add new View to GUI
	ParameterGUI &operator << (glv::View* newView){
		mBox << newView;
		mBox.fit();
		return *this;}
	/// Add new parameter to GUI
	ParameterGUI &operator << (PresetHandler &presetHandler){ return registerPresetHandler(presetHandler); }


	struct WidgetWrapper {
		Parameter *parameter;
		glv::Widget *widget;
		std::mutex *lock;
	};


private:
	glv::Box mBox;
	PresetButtons mPresetButtons;
	GLVDetachable mDetachableGUI;
	std::mutex mParameterGUILock;
	std::vector<WidgetWrapper *> mWrappers;
	PresetHandler *mPresetHandler;
	float mButtonScale;
};


}

#endif
