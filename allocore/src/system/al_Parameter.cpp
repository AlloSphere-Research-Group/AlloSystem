

#include "allocore/system/al_Parameter.hpp"

using namespace al;

Parameter::Parameter(std::string parameterName, std::string group, float defaultValue,
                     std::string prefix) :
    mMin(-99999.0), mMax(99999.0),
    mParameterName(parameterName), mGroup(group), mPrefix(prefix)
{
	//TODO: Add better heuristics for slash handling
	mFullAddress = mPrefix;
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
}

Parameter::~Parameter()
{
}

void Parameter::set(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
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

// ---- ParameterServer

ParameterServer::ParameterServer(std::string oscAddress, int oscPort)
    : mServer(NULL)
{
	if (oscAddress.length() > 0) {
		mServer = new osc::Recv(oscPort, oscAddress.c_str());
		mServer->handler(*this);
		mServer->start();
	}
}

ParameterServer::~ParameterServer()
{
	if (mServer) {
		delete mServer;
	}
}

void al::ParameterServer::registerParameter(al::Parameter &param)
{
	mParameters.push_back(&param);
}

void al::ParameterServer::unregisterParameter(al::Parameter &param)
{
	std::vector<Parameter *>::iterator it = mParameters.begin();
	for(it = mParameters.begin(); it != mParameters.end(); it++) {
		if (*it == &param) {
			mParameters.erase(it);
		}
	}
}

void ParameterServer::onMessage(osc::Message &m)
{
	float val;
	m >> val;
	for (Parameter *p:mParameters) {
		if(m.addressPattern() == p->getFullAddress() && m.typeTags() == "f"){
			// Extract the data out of the packet
			p->set(val);
		}
	}
}

