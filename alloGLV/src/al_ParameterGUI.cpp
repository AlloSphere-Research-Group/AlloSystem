#include "alloGLV/al_ParameterGUI.hpp"
#include "alloGLV/al_SequencerGUI.hpp"

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

ParameterGUI &ParameterGUI::addParameterBool(ParameterBool &parameter)
{
	glv::Button *button  = new glv::Button;
	button->setValue(parameter.get() == parameter.max());
	WidgetWrapper *wrapper = new WidgetWrapper;
	wrapper->parameter = &parameter;
	wrapper->lock = &mParameterGUILock;
	wrapper->widget = static_cast<glv::Widget *>(button);
	mWrappers.push_back(wrapper);
	button->attach(ParameterGUI::widgetChangeCallback, glv::Update::Value, wrapper);
	glv::Box *box = new glv::Box;
	*box << button << new glv::Label(parameter.getName());
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
	mBox << SequencerGUI::makePresetHandlerView(handler, 1.0f);
	mBox.fit();
	return *this;
}

