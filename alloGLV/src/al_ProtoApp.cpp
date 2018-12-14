#include <math.h>
#include "allocore/io/al_File.hpp"
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
:	cnNear(1,3,8,0.01), cnFar(3,2,999,1), cnFOV(3,2,360,0), cnScale(2,4, 20,-20),
	cnGain(1,4,4,0)
{
	mTopBar.arrangement("<>");
	mTopBar.paddingX(8);
	//mTopBar.stretch(1,0);
	//mTopBar.enable(glv::DrawBorder);
	mAppLabel.stroke(1.8); // bold
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
	mGUITable.enable(glv::DrawBack | glv::Controllable | glv::KeepWithinParent);
	mGUITable.addHandler(glv::Event::MouseDrag, glv::Behavior::mouseMove);
	mGUITable.arrangement(">");
	mGUITable << mTopBar /*<< new glv::Divider(3)*/ << mParamPanel;

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

	auto * win = initWindow(dim, title, fps, mode);
	mGUI.parentWindow(*win);

	win->drawCalls().push_back(
		[this](){
			if(mShowAxes){
				Mesh& m = graphics().mesh();
				m.reset();
				m.primitive(Graphics::LINES);
				m.vertex(1,0,0); m.vertex(0,0,0);
				m.vertex(0,1,0); m.vertex(0,0,0);
				m.vertex(0,0,1); m.vertex(0,0,0);
				for(int i=0;i<2;++i) m.color(RGB(0.8,0,0));
				for(int i=0;i<2;++i) m.color(RGB(0,0.8,0));
				for(int i=0;i<2;++i) m.color(RGB(0,0,0.8));
				graphics().lineWidth(2);
				graphics().draw(m);
			}
		}
	);

	// setup GUI
	mAppLabel.setValue(App::name());

	mGUI << mGUITable;

	mTopBar.arrange();
	mParamPanel.arrange();
	mGUITable.arrange();

	// Set GUI controls from app state by default
	cnNear.setValue(lens().near());
	cnFar.setValue(lens().far());
	cnFOV.setValue(lens().fovy());

	// Add notifications to controls;
	// must do it here to preserve user-specified app state, lens().far() etc.
	struct F{
		static void ntGUIUpdate(const glv::Notification& n){
			ProtoApp& app = *n.receiver<ProtoApp>();
			app.lens()
				.near(app.cnNear.getValue())
				.far(app.cnFar.getValue())
				.fovy(app.cnFOV.getValue());
		}
	};
	cnNear.attach(F::ntGUIUpdate, glv::Update::Value, this);
	cnFar.attach(F::ntGUIUpdate, glv::Update::Value, this);
	cnFOV.attach(F::ntGUIUpdate, glv::Update::Value, this);

	// setup model manager
	if(!App::name().empty()){
		glv::ModelManager& MM = mGUI.modelManager();

		//
		std::string idName = App::name();
		toIdentifier(idName);

		MM.name(idName + "Presets");
		resourceDir(mResourceDir); // verify path
		MM.fileDir(mResourceDir);

		mGUI.refreshModels();
		MM.add("nav", *new NavModel(nav()));

		paramPanel().presetControl()
			.modelManager(MM)
			.loadFile()
		;

		MM.zeroSmallValues();
	}
}

ProtoApp& ProtoApp::resourceDir(const std::string& dir, bool searchBack){

	std::string modDir = dir;
	bool pathExists = false;

	if(searchBack){
		pathExists = al::File::searchBack(modDir);
	} else {
		pathExists = al::File::exists(modDir);
	}

	if(!pathExists){
		AL_WARN("Could not find resource directory %s", modDir.c_str());
	} else {
		mResourceDir = modDir;
	}

	if(mResourceDir.back() != AL_FILE_DELIMITER){
		mResourceDir += AL_FILE_DELIMITER_STR;
	}

	return *this;
}

ProtoApp& ProtoApp::addParam(
	glv::View& v, const std::string& label, bool nameViewFromLabel
){
	paramPanel().addParam(v,label,nameViewFromLabel);
	return *this;
}

ProtoApp& ProtoApp::addParam(
	glv::View * v, const std::string& label, bool nameViewFromLabel
){
	return addParam(*v,label,nameViewFromLabel);
}

double ProtoApp::gainFactor() const {
	float v=cnGain.getValue();
	return v*v;
}

double ProtoApp::scaleFactor() const {
	return ::pow(2., cnScale.getValue());
}

} // al::
