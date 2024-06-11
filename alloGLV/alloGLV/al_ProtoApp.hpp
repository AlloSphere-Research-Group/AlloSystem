#ifndef INCLUDE_AL_PROTOAPP_HPP
#define INCLUDE_AL_PROTOAPP_HPP
/*	Allocore --
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
	Helper app for audio/visual prototyping

	File author(s):
	Lance Putnam, 2012, putnam.lance@gmail.com
*/

#include <string>
#include "allocore/io/al_App.hpp"
#include "alloGLV/al_ControlGLV.hpp"
#include "GLV/glv.h"


namespace al{

/// Application for audio/visual prototyping

/// This App subclass provides a simpler way to setup an audio/visual app
/// with a GUI. Parameters can easily be added to the GUI and saved/loaded
/// to/from file.
class ProtoApp : public App{
public:

	glv::NumberDialer cnNear, cnFar, cnFOV, cnScale;
	glv::NumberDialer cnGain;

	ProtoApp();

	/// This should be called after configuring everything else with the app
	void init(
		const Window::Dim& dim = Window::Dim(800,600),
		const std::string& title="",
		double fps=40,
		Window::DisplayMode mode = Window::DEFAULT_BUF,
		double sampleRate = 44100,
		int blockSize = 256,
		int chansOut = -1,
		int chansIn = -1
	);

	/// Set the directory for application resources
	ProtoApp& resourceDir(const std::string& dir, bool searchBack = true);
	const std::string& resourceDir() const { return App::resourceDir(); }

	/// Get GUI
	GLVDetachable& gui(){ return mGUI; }

	/// Get parameter panel GUI element
	glv::ParamPanel& paramPanel(){ return mParamPanel; }

	/// Add a parameter to GUI
	ProtoApp& addParam(
		glv::View& v, const std::string& label="", bool nameViewFromLabel=true
	);

	/// Add a parameter to GUI
	ProtoApp& addParam(
		glv::View * v, const std::string& label="", bool nameViewFromLabel=true
	);

	ProtoApp& showAxes(bool v){ mShowAxes=v; return *this; }
	ProtoApp& toggleAxes(){ return showAxes(!mShowAxes); }

	double gainFactor() const;
	double scaleFactor() const;

protected:
	GLVDetachable mGUI;
	glv::Table mGUITable;
	glv::ParamPanel mParamPanel;
	glv::Table mTopBar;
	glv::Label mAppLabel;
	bool mShowAxes = false;
};

} // al::

#endif
