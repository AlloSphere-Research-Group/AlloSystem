
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/system/al_Parameter.hpp"
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
	mAtomicValue.store(defaultValue);
}

float Parameter::get()
{
	return mAtomicValue.load();
}

void Parameter::setNoCalls(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}

	mAtomicValue.store(value);
}

void Parameter::set(float value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	mAtomicValue.store(value);
	for(int i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](value, this, mCallbackUdata[i]);
		}
	}
}


// ParameterServer ------------------------------------------------------------

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
	mListenerLock.lock();
	param.registerChangeCallback(ParameterServer::changeCallback,
	                           (void *) &param);
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

void ParameterServer::addListener(std::string oscAddress, int oscPort)
{
	OSCListenerInfo listener;
	listener.OSCaddress = oscAddress;
	listener.port = oscPort;
	listener.lock = &mListenerLock;
	mListeners.push_back(listener);
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

void ParameterServer::changeCallback(float value, void *sender, void *userData)
{
	ParameterServer *server = static_cast<ParameterServer *>(userData);
	Parameter *parameter = static_cast<Parameter *>(sender);
	server->mListenerLock.lock();
	for(OSCListenerInfo listener: server->mListeners) {
		server->mOSCSender.send(parameter->getFullAddress(), value);
	}
	server->mListenerLock.unlock();
}

// PresetHandler --------------------------------------------------------------

PresetHandler::PresetHandler(std::string rootDirectory, bool verbose) :
    mRootDir(rootDirectory), mVerbose(verbose)
{
	if (!File::exists(rootDirectory)) {
		if (!Dir::make(rootDirectory, true)) {
			std::cout << "Error creating directory: " << rootDirectory << std::endl;
		}
	}
}

PresetHandler &PresetHandler::registerParameter(Parameter &parameter)
{
	mParameters.push_back(&parameter);
	return *this;
}

void PresetHandler::setSubDirectory(std::string directory)
{
	mSubDir = directory;
	std::string path = getCurrentPath();
	if (!File::exists(path)) {
		if (!Dir::make(path, true)) {
			std::cout << "Error creating directory: " << mRootDir << std::endl;
		}
	}
}

std::vector<std::string> al::PresetHandler::availableSubDirectories()
{
	std::vector<std::string> subDirList;
	Dir presetDir(mRootDir);
	while(presetDir.read()) {
		FileInfo info = presetDir.entry();
		if (info.type() == FileInfo::DIR) {
			subDirList.push_back(info.name());
		}
	}
	return subDirList;
}

void PresetHandler::storePreset(std::string name)
{
	std::lock_guard<std::mutex> lock(mFileLock);
	std::string line;
	std::ofstream f(name + ".preset");
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset file: " << mFileName << std::endl;
		}
		return;
	}
	f << "::" + name << std::endl;
	for(Parameter *parameter: mParameters) {
		std::string line = parameter->getFullAddress() + " f " + std::to_string(parameter->get());
		f << line << std::endl;
	}
	f << "::" << std::endl;
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset file: " << mFileName << std::endl;
		}
	}
	f.close();
}

void PresetHandler::recallPreset(std::string name)
{
	std::lock_guard<std::mutex> lock(mFileLock);
	std::string line;
	std::ifstream f(name + ".preset");
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset file: " << mFileName << std::endl;
		}
	}
	while(getline(f, line)) {
		if (line.substr(0, 2) == "::") {
			if (mVerbose) {
				std::cout << "Found preset : " << line << std::endl;
			}
			while (getline(f, line)) {
				if (line.substr(0, 2) == "::") {
					if (mVerbose) {
						std::cout << "End preset."<< std::endl;
					}
					break;
				}
				bool parameterFound = false;
				std::stringstream ss(line);
				std::string address, type, value;
				std::getline(ss, address, ' ');
				std::getline(ss, type, ' ');
				std::getline(ss, value, ' ');
				for(Parameter *param: mParameters) {
					if (param->getFullAddress() == address) {
						if (type == "f") {
							param->set(std::stof(value));
							parameterFound = true;
							break;
						}
					}
				}
				if (!parameterFound && mVerbose) {
					std::cout << "Preset in parameter not present: " << address;
				}
			}
		}
	}
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset file: " << mFileName << std::endl;
		}
	}
	f.close();
}

std::string al::PresetHandler::getCurrentPath()
{
	std::string relPath = mRootDir;
	if (relPath.back() != '/') {
		relPath += "/";
	}
	relPath += mSubDir;
	return relPath;
}

