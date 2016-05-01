#include "alloGLV/al_ParameterGUI.hpp"


using namespace al;

ParameterGUI &ParameterGUI::addParameter(Parameter &parameter) {
	int numInt = 1 + ceil(log10(max(fabs(parameter.max()), fabs(parameter.min()))));
	glv::NumberDialer *number  = new glv::NumberDialer(numInt, 6 - numInt,
	                                                   parameter.max(),
	                                                   parameter.min());
	number->setValue(parameter.get());
	WidgetWrapper *wrapper = new WidgetWrapper;
	wrapper->parameter = &parameter;
	wrapper->lock = &mParameterGUILock;
	wrapper->widget = static_cast<glv::Widget *>(number);
	mWrappers.push_back(wrapper);
	number->attach(ParameterGUI::widgetChangeCallback, glv::Update::Value, wrapper);
	glv::Box *box = new glv::Box;
	*box << number << new glv::Label(parameter.getName());
	box->fit();
	mBox << box;
	mBox.fit();
	parameter.registerChangeCallback(ParameterGUI::valueChangedCallback, wrapper);
	return *this;
}

ParameterGUI &ParameterGUI::registerPresetHandler(PresetHandler &handler) {
	if (mPresetHandler) {
		std::cout << "IGNORED. Only one preset handler can be attached to a ParameterGUI class." << std::endl;
		return *this;
	}
	mPresetHandler = &handler;
	mPresetHandler->registerPresetCallback(ParameterGUI::presetChangeCallback, (void *) this);
	mPresetButtons.width(240* mButtonScale);
	mPresetButtons.height(96* mButtonScale);
	mBox << new glv::Label("Presets");

	glv::Box *nameBox = new glv::Box(glv::Direction::E);
	glv::TextView *presetNameView = new glv::TextView;
	*nameBox << new glv::Label("Name") << presetNameView;
	mBox << nameBox;
	mBox << mPresetButtons;
	glv::Box *storeBox = new glv::Box(glv::Direction::E);
	glv::Button *storeButton = new glv::Button(glv::Rect(150* mButtonScale, 24* mButtonScale));
	*storeBox << new glv::Label("Store", glv::Place::CL, 0, 0) << storeButton;
	mBox << storeBox;
	mPresetButtons.attach(ParameterGUI::presetSavedInButton,
	                      glv::Update::Value, (void *) storeButton);

	mPresetButtons.setPresetHandler(handler);

	storeButton->attachVariable(&mPresetButtons.mStore, 1);
	presetNameView->attachVariable(&mPresetButtons.presetName, 1);
	return *this;
}
