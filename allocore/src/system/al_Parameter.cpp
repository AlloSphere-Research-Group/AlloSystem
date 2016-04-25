
#include <iostream>

#include "allocore/system/al_Parameter.hpp"

using namespace al;

Parameter::Parameter(std::string parameterName, std::string group, float defaultValue,
                     std::string prefix, float min, float max) :
    mMin(min), mMax(max),
    mParameterName(parameterName), mGroup(group), mPrefix(prefix)
{
	//TODO: Add better heuristics for slash handling
	if (mPrefix.length() > 0 && mPrefix.at(0) != '/') {
		mFullAddress = "/";
	}
	mFullAddress += mPrefix;
	if (mPrefix.length() > 0 && mPrefix.at(prefix.length() - 1) != '/') {
		mFullAddress += "/";
	}
	mFullAddress += mGroup;
	if (mGroup.length() > 0 && mGroup.at(mGroup.length() - 1) != '/') {
		mFullAddress += "/";
	}
	if (mFullAddress.length() == 0) {
		mFullAddress = "/";
	}
	mFullAddress += mParameterName;
	mValue = defaultValue;
	mValueCache = defaultValue;
}

Parameter::~Parameter()
{
}

void Parameter::set(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	mMutex.lock();
	mValue = value;
	mMutex.unlock();
	for(int i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](value, mCallbackUdata[i]);
		}
	}
}

void al::Parameter::setNoCalls(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	mMutex.lock();
	mValue = value;
	mMutex.unlock();
}

float Parameter::get()
{
	if (mMutex.try_lock()) {
		mValueCache = mValue;
		mMutex.unlock();
	}
	return mValueCache;
}

std::string Parameter::getFullAddress()
{
	return mFullAddress;
}

std::string Parameter::getName()
{
	return mParameterName;
}

void Parameter::setProcessingCallback(al::Parameter::ParameterProcessCallback cb, void *userData)
{
	mProcessCallback = cb;
	mProcessUdata = userData;
}

void al::Parameter::registerChangeCallback(al::Parameter::ParameterChangeCallback cb, void *userData)
{
	mCallbacks.push_back(cb);
	mCallbackUdata.push_back(userData);
}

// ---- ParameterServer

ParameterServer::ParameterServer(std::string oscAddress, int oscPort)
    : mServer(NULL)
{
	mServer = new osc::Recv(oscPort, oscAddress.c_str());
	mServer->handler(*this);
	mServer->start();
}

ParameterServer::~ParameterServer()
{
	if (mServer) {
		delete mServer;
	}
}

ParameterServer &ParameterServer::registerParameter(Parameter &param)
{
	mParameterLock.lock();
	mParameters.push_back(&param);
	mParameterLock.unlock();
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

