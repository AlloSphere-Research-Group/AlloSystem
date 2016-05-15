#include "alloGLV/al_ParameterGUI.hpp"


using namespace al;

ParameterGUI &ParameterGUI::addParameter(Parameter &parameter) {
	int numInt = 2 + ceil(log10(max(fabs(parameter.max()), fabs(parameter.min()))));
	glv::NumberDialer *number  = new glv::NumberDialer(numInt, 7 - numInt,
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

	glv::Box *morphBox = new glv::Box(glv::Direction::E);
	glv::NumberDialer *morphTime = new glv::NumberDialer(2,2, 20, 0);
	*morphBox << new glv::Label("Morph time") << morphTime;
	mBox << morphBox;

	// Now register callbacks for morph button
	morphTime->attach([](const glv::Notification &n) {
		glv::Widget &sender = *n.sender<glv::Widget>();
		static_cast<PresetHandler *>(n.receiver())->setMorphTime(sender.getValue<double>());
		std::cout << sender.getValue<double>() << std::endl;
	},
	glv::Update::Value, mPresetHandler);

	mPresetHandler->registerMorphTimeCallback([](float value, void *sender,
	                                     void *userData, void * blockSender) {
		static_cast<glv::NumberDialer *>(userData)->setValue(value);
	}, morphTime);

//	WidgetWrapper *wrapper = new WidgetWrapper;
//	wrapper->parameter = &mMorphTime;
//	wrapper->lock = &mParameterGUILock;
//	wrapper->widget = static_cast<glv::Widget *>(morphTime);
//	mWrappers.push_back(wrapper);

//	mMorphTime.registerChangeCallback([](float value, void *sender,
//	                                  void *userData, void * blockSender) {
//		valueChangedCallback(value, sender, userData, blockSender);
//	}, wrapper);

	// Register callbacks for preset buttons
	mPresetButtons.attach(ParameterGUI::presetSavedInButton,
	                      glv::Update::Value, (void *) storeButton);

	mPresetButtons.setPresetHandler(handler);

	storeButton->attachVariable(&mPresetButtons.mStore, 1);
	presetNameView->attachVariable(&mPresetButtons.presetName, 1);

	return *this;
}
