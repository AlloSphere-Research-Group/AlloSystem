#ifndef INC_AL_PROTOAPP_HPP
#define INC_AL_PROTOAPP_HPP
/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Helper app for audio/visual prototyping

	Author(s):
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
