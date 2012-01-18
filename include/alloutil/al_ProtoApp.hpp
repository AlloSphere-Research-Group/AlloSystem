#ifndef INCLUDE_AL_PROTOAPP_HPP
#define INCLUDE_AL_PROTOAPP_HPP
/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Helper app for audio/visual prototyping

	File author(s):
	Lance Putnam, 2012, putnam.lance@gmail.com
*/

#include <math.h>
#include <string>
#include "alloutil/al_App.hpp"
#include "alloutil/al_ControlGLV.hpp"

#include "GLV/glv.h"

#define GAMMA_H_INC_ALL
#include "Gamma/Gamma.h"

namespace al{


/// Application for audio/visual prototyping

/// This App subclass provides a simpler way to setup an audio/visual app
/// with a GUI. Parameters can easily be added to the GUI and saved/loaded
/// to/from file.
class ProtoApp : public App{
public:

	glv::NumberDialer cnFOV, cnScale;
	glv::NumberDialer cnGain;

	ProtoApp();
	
	/// This should be called after configuring everything else with the app
	void init();
	
	/// Set the directory for application resources
	ProtoApp& resourceDir(const std::string& dir, bool searchBack=true);
	
	glv::ParamPanel& paramPanel(){ return mParamPanel; }

	double gainFactor() const { float v=cnGain.getValue(); return v*v; }
	double scaleFactor() const { return ::pow(2., cnScale.getValue()); }


	/// This should still be called via ProtoApp::onAnimate(dt) if overridden
	virtual void onAnimate(double dt){
		camera().fovy(cnFOV.getValue());
	}

//	virtual void onDraw(Graphics& g, const Viewpoint& v){}
//	virtual void onSound(AudioIOData& io){}

protected:
	GLVDetachable mGUI;
	glv::Table mGUITable;
	glv::ParamPanel mParamPanel;
	glv::Table mTopBar;
	glv::Label mAppLabel;
	std::string mResourceDir;
};

} // al::

#endif
