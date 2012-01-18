#include "alloutil/al_ProtoApp.hpp"

namespace al{

ProtoApp::ProtoApp()
:	cnFOV(3,2,360,0), cnScale(2,4, 20,-20),
	cnGain(1,4,4,0)
{
	initAudio();

	Window * win = initWindow();

	cnFOV.setValue(60);

	mTopBar.arrangement("<>");
	//mTopBar.stretch(1,0);
	//mTopBar.enable(glv::DrawBorder);
	mTopBar << mAppLabel << mGUI.detachedButton();

	mParamPanel
		.addParam(cnFOV, "FOV")
		.addParam(cnScale, "scale")
		.addParam(cnGain, "gain")
	;

	mGUITable.colors().set(glv::StyleColor::SmokyGray);
	mGUITable.enable(glv::DrawBack);		
	mGUITable.arrangement(">");
	mGUITable << mTopBar << mParamPanel;

	mGUI.cloneStyle().colors().back.set(0,0,0,1);
	mGUI.detachedButton().padding(4);
	mGUI.parentWindow(*win);
}


void ProtoApp::init(){
	// setup audio
	if(usingAudio()){
		gam::Sync::master().spu(audioIO().fps());
	}

	// setup GUI
	mAppLabel.setValue(App::name());

	mGUI << mGUITable;

	mTopBar.arrange();
	mParamPanel.arrange();
	mGUITable.arrange();

	// setup model manager
	if(!App::name().empty()){
		glv::ModelManager& MM = mGUI.modelManager();
		MM.name(App::name());
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
			printf("Could not find %s\n", modDir.c_str());
			exit(0);
		}
	}
	mResourceDir = modDir;
	return *this;
}

} // al::

/*
What should every prototyping app have?

Continuous controls:

graphics:
	camera FOV
	global scale
	navigation position/orientation
	brightness/contrast

sound:
	global gain


Configuration:


TODO:
High-res screen grabs
Recording?

*/

/*
	std::string path = "ljp/";
	if(!al::File::searchBack(path)){
		printf("Could not find %s\n", path.c_str());
		exit(0);
	}
	//printf("%s\n", path.c_str());

	topView.refreshModels();
	topView.modelManager().name("WrapturePresets").fileDir(path);
	presetControl.modelManager(topView.modelManager());
	presetControl.loadFile();

	ModelManager pathEditMM;
	pathEditMM.copyModels(topView.modelManager());
	pathEditMM.add("pose", *new PoseModel(W.nav()));
	//pathEditMM.name("WraptureEtDr").fileDir(path);
	//pathEditMM.name("WraptureTemp").fileDir(path);
	//pathEditMM.name("Wrapture000").fileDir(path);
	pathEditMM.name("WraptureDennis").fileDir(path);
	pathEditMM.snapshotsFromFile();
	pathEdit.stateModelManager(pathEditMM);
	pathEdit.pathModelManager().fileDir(path);

	pathEditMM.makeClosed();

*/

