#include "alloGLV/al_ProtoApp.hpp"

namespace al{


struct GUIKeyDownHandler : public glv::EventHandler{
	bool onEvent(glv::View& v, glv::GLV& g){
		switch(g.keyboard().key()){
			case 'g': g.toggle(glv::Visible); return false;
		}
		return true;
	}
};
static GUIKeyDownHandler mGUIKeyDownHandler;


ProtoApp::ProtoApp()
:	cnNear(1,3,8,0.01), cnFar(2,2,99,1), cnFOV(3,2,360,0), cnScale(2,4, 20,-20),
	cnGain(1,4,4,0)
{
	mTopBar.arrangement("<>");
	mTopBar.paddingX(8);
	//mTopBar.stretch(1,0);
	//mTopBar.enable(glv::DrawBorder);
	mTopBar << mAppLabel << mGUI.detachedButton();
	mGUI.detachedButton().disable(glv::DrawBack);

	cnNear.setValue(0.1);
	cnFar.setValue(20);
	cnFOV.setValue(60);
	cnScale.setValue(1);
	cnGain.setValue(0.5);

	mParamPanel
		.addParamGroup(cnNear, "near", cnFar, "far", cnFOV, "FOV", "lens")
		.addParam(cnScale, "scale")
		.addParam(cnGain, "gain")
	;

	mGUI.addHandler(glv::Event::KeyDown, mGUIKeyDownHandler);

	mGUITable.colors().set(glv::StyleColor::SmokyGray);
	//mGUITable.enable(glv::DrawBack);
	mGUITable.enable(glv::DrawBack | glv::Controllable);
	mGUITable.addHandler(glv::Event::MouseDrag, glv::Behavior::mouseMove);
	mGUITable.arrangement(">");
	mGUITable << mTopBar << new glv::Divider(1) << mParamPanel;

	mGUI.cloneStyle().colors().back.set(0,0,0,1);
	mGUI.detachedButton().padding(4);
}


static bool toIdentifier(std::string& v){
	bool modified = false;
	if(!(isalpha(v[0]) || '_'==v[0])){
		v = "_" + v;
		modified = true;
	}

	for(unsigned i=1; i<v.size(); ++i){
		if(!(isalnum(v[i]) || '_'==v[i])){
			v[i] = '_';
			modified = true;
		}
	}
	return modified;
}

void ProtoApp::init(
	const Window::Dim& dim,
	const std::string& title,
	double fps,
	Window::DisplayMode mode,
	double sampleRate,
	int blockSize,
	int chansOut,
	int chansIn
){

	initAudio(sampleRate, blockSize, chansOut, chansIn);

	// setup audio
	if(usingAudio()){
		//gam::Sync::master().spu(audioIO().fps());
	}


	Window * win = initWindow(dim, title, fps, mode);
	mGUI.parentWindow(*win);

	// setup GUI
	mAppLabel.setValue(App::name());

	mGUI << mGUITable;

	mTopBar.arrange();
	mParamPanel.arrange();
	mGUITable.arrange();

	// setup model manager
	if(!App::name().empty()){
		glv::ModelManager& MM = mGUI.modelManager();

		//
		std::string idName = App::name();
		toIdentifier(idName);

		MM.name(idName + "Presets");
		MM.fileDir(mResourceDir);

		mGUI.refreshModels();
		MM.add("pose", *new PoseModel(nav()));

		paramPanel().presetControl()
			.modelManager(MM)
			.loadFile()
		;
	}
}

ProtoApp& ProtoApp::resourceDir(const std::string& dir, bool searchBack){

	std::string modDir = dir;

	if(searchBack){
		if(!al::File::searchBack(modDir)){
			AL_WARN("Could not find %s", modDir.c_str());
			exit(0);
		}
	}
	mResourceDir = modDir;

	if(mResourceDir[mResourceDir.size()-1] != AL_FILE_DELIMITER){
		mResourceDir += AL_FILE_DELIMITER_STR;
	}

	return *this;
}

} // al::
