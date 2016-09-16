
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/ui/al_Parameter.hpp"
#include "allocore/io/al_File.hpp"

using namespace al;

// Parameter ------------------------------------------------------------------
Parameter::Parameter(std::string parameterName, std::string Group,
                     float defaultValue,
                     std::string prefix,
                     float min,
                     float max) :
    ParameterWrapper<float>(parameterName, Group, defaultValue, prefix, min, max)
{
	mFloatValue = defaultValue;
}

float Parameter::get()
{
	return mFloatValue;
}

void Parameter::setNoCalls(float value, void *blockReceiver)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	if (blockReceiver) {
		for(int i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
			}
		}
	}

	mFloatValue = value;
}

void Parameter::set(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	mFloatValue = value;
	for(int i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](value, this, mCallbackUdata[i], NULL);
		}
	}
}

// ParameterBool ------------------------------------------------------------------
ParameterBool::ParameterBool(std::string parameterName, std::string Group,
                     float defaultValue,
                     std::string prefix,
                     float min,
                     float max) :
    ParameterWrapper<float>(parameterName, Group, defaultValue, prefix, min, max)
{
	mFloatValue = defaultValue;
}

float ParameterBool::get()
{
	return mFloatValue;
}

void ParameterBool::setNoCalls(float value, void *blockReceiver)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	if (blockReceiver) {
		for(int i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
			}
		}
	}

	mFloatValue = value;
}

void ParameterBool::set(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	mFloatValue = value;
	for(int i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](value, this, mCallbackUdata[i], NULL);
		}
	}
}

// ParameterServer ------------------------------------------------------------

ParameterServer::ParameterServer(std::string oscAddress, int oscPort)
    : mServer(nullptr)
{
	mServer = new osc::Recv(oscPort, oscAddress.c_str(), 0.001); // Is 1ms wait OK?
	if (mServer) {
		mServer->handler(*this);
		mServer->start();
	} else {
		std::cout << "Error starting OSC server." << std::endl;
	}
}

ParameterServer::~ParameterServer()
{
//	std::cout << "~ParameterServer()" << std::endl;
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

ParameterServer &ParameterServer::registerParameter(Parameter &param)
{
	mParameterLock.lock();
	mParameters.push_back(&param);
	mParameterLock.unlock();
	mListenerLock.lock();
	param.registerChangeCallback(ParameterServer::changeCallback,
	                           (void *) this);
	mListenerLock.unlock();
	return *this;
}

void ParameterServer::unregisterParameter(Parameter &param)
{
	mParameterLock.lock();
	std::vector<Parameter *>::iterator it = mParameters.begin();
	for(it = mParameters.begin(); it != mParameters.end(); it++) {
		if (*it == &param) {
			mParameters.erase(it);
		}
	}
	mParameterLock.unlock();
}

void ParameterServer::onMessage(osc::Message &m)
{
	std::string requestAddress = "/request";
	m.resetStream(); // Needs to be moved to caller...
	if(m.addressPattern() == requestAddress && m.typeTags() == "s") {
		std::string parameterAddress;
		m >> parameterAddress;
	}
	float val;
	m >> val;
	mParameterLock.lock();
	for (Parameter *p:mParameters) {
		if(m.addressPattern() == p->getFullAddress() && m.typeTags() == "f"){
			// Extract the data out of the packet
			p->set(val);
//			std::cout << "ParameterServer::onMessage" << val << std::endl;
		}
	}
	for (osc::PacketHandler *handler: mPacketHandlers) {
		m.resetStream();
		handler->onMessage(m);
	}
	mParameterLock.unlock();
}

void ParameterServer::print()
{
	std::cout << "Parameter server listening on " << mServer->address()
	          << ":" << mServer->port() << std::endl;
	for (Parameter *p:mParameters) {
		std::cout << "Parameter " << p->getName() << " : " <<  p->getFullAddress() << std::endl;
	}
}

void ParameterServer::stopServer()
{
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

void ParameterServer::registerOSCListener(osc::PacketHandler *handler)
{
	mParameterLock.lock();
	mPacketHandlers.push_back(handler);
	mParameterLock.unlock();
}

void ParameterServer::changeCallback(float value, void *sender, void *userData, void *blockThis)
{
	ParameterServer *server = static_cast<ParameterServer *>(userData);
	Parameter *parameter = static_cast<Parameter *>(sender);
	server->notifyListeners(parameter->getFullAddress(), parameter->get());
}
