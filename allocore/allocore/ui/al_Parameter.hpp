#ifndef AL_PARAMETER_H
#define AL_PARAMETER_H

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2015. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
	Parameter class that encapsulates communication of float data in a thread
	safe way to a separate thread (e.g. audio thread)
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <mutex>
#include <atomic>
#include <iostream>
#include <float.h>

#include "allocore/protocol/al_OSC.hpp"
#include "allocore/math/al_Vec.hpp"

namespace al
{

class Parameter;

class OSCNotifier {
public:
	OSCNotifier();
	virtual ~OSCNotifier();

	/**
	 * @brief addListener enables notifiying via OSC that a preset has changed
	 * @param IPaddress The IP address of the listener
	 * @param oscPort The network port so send the value changes on
	 */
	virtual void addListener(std::string IPaddress, int oscPort) {
		mListenerLock.lock();
		mOSCSenders.push_back(new osc::Send(oscPort, IPaddress.c_str()));
		mListenerLock.unlock();
//		std::cout << "Registered listener " << IPaddress << ":" << oscPort<< std::endl;
	}

	/**
	 * @brief Notify the listeners of value changes
	 * @param OSCaddress The OSC path to send the value on
	 * @param value The value to send
	 *
	 * This will send all registered data to the listeners. This is useful to
	 * force a resfresh of an interface, e.g. when it just came online and is
	 * unaware of state. Otherwise, when calling addListener, you should
	 * register to be notified when the data changes to only do notifications
	 * then.
	 *
	 */
	void notifyListeners(std::string OSCaddress, float value);

	void notifyListeners(std::string OSCaddress, std::string value);
	void notifyListeners(std::string OSCaddress, Vec3f value);
	void notifyListeners(std::string OSCaddress, Vec4f value);

protected:
	std::mutex mListenerLock;
	std::vector<osc::Send *> mOSCSenders;
private:
};

template<class ParameterType>
class ParameterWrapper{
public:
	/**
	* @brief ParameterWrapper
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * The mechanism used to protect data is locking a mutex within the set() function
   * and doing try_lock() on the mutex to update a cached value in the get()
   * function. In the worst case this might incur some jitter when reading the value.
   */
	ParameterWrapper(std::string parameterName, std::string group,
	          ParameterType defaultValue,
	          std::string prefix ="");

	ParameterWrapper(std::string parameterName, std::string Group,
	          ParameterType defaultValue,
	          std::string prefix,
	          ParameterType min,
	          ParameterType max
	        );
	
	ParameterWrapper(const ParameterWrapper& param);

	virtual ~ParameterWrapper();
	
	/**
	 * @brief set the parameter's value
	 * 
	 * This function is thread-safe and can be called from any number of threads
	 */
	virtual void set(ParameterType value);

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
	virtual void setNoCalls(ParameterType value, void *blockReceiver = NULL);

	/**
	 * @brief set the parameter's value forcing a lock
	 */
	inline void setLocking(ParameterType value)
	{
		mMutex.lock();
		mValue = value;
		mMutex.unlock();
	}

	/**
	 * @brief get the parameter's value
	 * 
	 * This function is thread-safe and can be called from any number of threads
	 * 
	 * @return the parameter value
	 */
	virtual ParameterType get();
	
	/**
	 * @brief set the minimum value for the parameter
	 * 
	 * The value returned by the get() function will be clamped and will not go
	 * under the value set by this function.
	 */
	void min(ParameterType minValue) {mMin = minValue;}
	ParameterType min() {return mMin;}
	
	/**
	 * @brief set the maximum value for the parameter
	 * 
	 * The value returned by the get() function will be clamped and will not go
	 * over the value set by this function.
	 */
	void max(ParameterType maxValue) {mMax = maxValue;}
	ParameterType max() {return mMax;}
	
	/**
	 * @brief return the full OSC address for the parameter
	 * 
	 * The parameter needs to be registered to a ParameterServer to listen to
	 * OSC values on this address
	 */
	std::string getFullAddress();

	/**
	 * @brief getName returns the name of the parameter
	 */
	std::string getName();

	typedef float (*ParameterProcessCallback)(ParameterType value, void *userData);
	typedef void (*ParameterChangeCallback)(ParameterType value, void *sender,
	                                        void *userData, void * blockSender);

	/**
	 * @brief setProcessingCallback sets a callback to be called whenever the
	 * parameter value changes
	 *
	 * Setting a callback can be useful when specific actions need to be taken
	 * whenever a parameter changes, but it can also be used to modify the value
	 * of the incoming parameter value before it is stored in the parameter.
	 * The registered callback must return the value to be stored in the
	 * parameter.
	 * Only one callback may be registered here.
	 *
	 * @param cb The callback function
	 * @param userData user data that is passed to the callback function
	 *
	 * @return the transformed value
	 */
	void setProcessingCallback(ParameterProcessCallback cb,
	                           void *userData = nullptr);
	/**
	 * @brief registerChangeCallback adds a callback to be called when the value changes
	 *
	 * This function appends the callback to a list of callbacks to be called
	 * whenever a value changes.
	 *
	 * @param cb
	 * @param userData
	 */
	void registerChangeCallback(ParameterChangeCallback cb,
	                           void *userData = nullptr);

	std::vector<ParameterWrapper<ParameterType> *> operator<< (ParameterWrapper<ParameterType> &newParam)
	{ std::vector<ParameterWrapper<ParameterType> *> paramList;
		paramList.push_back(&newParam);
		return paramList; }

	std::vector<ParameterWrapper<ParameterType> *> &operator<< (std::vector<ParameterWrapper<ParameterType> *> &paramVector)
	{ paramVector.push_back(this);
		return paramVector;
	}

	const ParameterType operator= (const ParameterType value) { this->set(value); return value; }
	
protected:
	ParameterType mMin;
	ParameterType mMax;
	
	std::string mFullAddress;
	std::string mParameterName;
	std::string mGroup;
	std::string mPrefix;

	ParameterProcessCallback mProcessCallback;
	void * mProcessUdata;
	std::vector<ParameterChangeCallback> mCallbacks;
	std::vector<void *> mCallbackUdata;

private:
	std::mutex mMutex;
	ParameterType mValue;
	ParameterType mValueCache;
};


/**
 * @brief The Parameter class
 *
 * The Parameter class offers a simple way to encapsulate float values. It is
 * not inherently thread safe, but since floats are atomic on most platforms
 * it is safe in practice.
 *
 * Parameters are created with:
 * @code
	Parameter freq("Frequency", "Group", default, "/path/prefix");
 * @endcode
 *
 * Then values can be set from a low priority thread:
 * @code
	// In a simulator thread
	freq.set(var);
 * @endcode
 *
 * And read back from a high priority thread:
 * @code
	// In the audio thread
	float curFreq = freq.get()
 * @endcode
 *
 * The values are clamped between a minimum and maximum set using the min() and
 * max() functions.
 *
 * The ParameterServer class allows exposing Parameter objects via OSC.
 *
 * @ingroup allocore
 */

class Parameter : public ParameterWrapper<float>
{
public:
	/**
	* @brief Parameter
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Minimum value for the parameter
   * @param max Maximum value for the parameter
   *
   * This Parameter class is designed for parameters that can be expressed as a
   * single float. It realies on float being atomic on the platform so there
   * is no locking. This is a safe assumption for most platforms today.
   */
	Parameter(std::string parameterName, std::string Group,
	          float defaultValue,
	          std::string prefix = "",
	          float min = -99999.0,
	          float max = 99999.0
	        );

	Parameter(const al::Parameter& param) :
	    ParameterWrapper<float>(param)
	{
		mFloatValue = param.mFloatValue;
	}

	/**
	 * @brief set the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
	 */
	virtual void set(float value) override;

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
	virtual void setNoCalls(float value, void *blockReceiver = NULL) override;

	/**
	 * @brief get the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
	 *
	 * @return the parameter value
	 */
	virtual float get() override;

	float operator= (const float value) { this->set(value); return value; }

private:
	float mFloatValue;
};

class ParameterBool : public Parameter
{
public:
	/**
	* @brief ParameterBool
   *
   * @param parameterName The name of the parameter
   * @param Group The group the parameter belongs to
   * @param defaultValue The initial value for the parameter
   * @param prefix An address prefix that is prepended to the parameter's OSC address
   * @param min Value when off/false
   * @param max Value when on/true
   *
   * This ParameterBool class is designed for boolean parameters that have
   * float values for on or off states. It relies on floats being atomic on
   * the platform.
   */
	ParameterBool(std::string parameterName, std::string Group,
	          float defaultValue,
	          std::string prefix = "",
	          float min = 0,
	          float max = 1.0
	        );

	/**
	 * @brief set the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
	 */
	virtual void set(float value) override;

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
	virtual void setNoCalls(float value, void *blockReceiver = NULL) override;

	/**
	 * @brief get the parameter's value
	 *
	 * This function is thread-safe and can be called from any number of threads
	 *
	 * @return the parameter value
	 */
	virtual float get() override;

	float operator= (const float value) { this->set(value); return value; }

private:
	float mFloatValue;
};

// These three types are blocking, should not be used in time-critical contexts
// like the audio callback. The classes were explicitly defined to overcome
// the issues related to the > and < operators needed when validating minumum
// and maximum values for the parameter
class ParameterString: public ParameterWrapper<std::string>
{
	virtual void set(std::string value) override
	{
		if (mProcessCallback) {
			value = mProcessCallback(value, mProcessUdata);
		}
		setLocking(value);
		for(size_t i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](value, this,  mCallbackUdata[i], NULL);
			}
		}
	}

	virtual void setNoCalls(std::string value, void *blockReceiver) override
	{
		if (mProcessCallback) {
			value = mProcessCallback(value, mProcessUdata);
		}
		if (blockReceiver) {
			for(size_t i = 0; i < mCallbacks.size(); ++i) {
				if (mCallbacks[i]) {
					mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
				}
			}
		}
		setLocking(value);
	}
};

class ParameterVec3: public ParameterWrapper<al::Vec3f>
{
	virtual void set(Vec3f value) override
	{
		if (mProcessCallback) {
			value = mProcessCallback(value, mProcessUdata);
		}
		setLocking(value);
		for(size_t i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](value, this,  mCallbackUdata[i], NULL);
			}
		}
	}

	virtual void setNoCalls(Vec3f value, void *blockReceiver) override
	{
		if (mProcessCallback) {
			value = mProcessCallback(value, mProcessUdata);
		}
		if (blockReceiver) {
			for(size_t i = 0; i < mCallbacks.size(); ++i) {
				if (mCallbacks[i]) {
					mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
				}
			}
		}
		setLocking(value);
	}
};

class ParameterVec4: public ParameterWrapper<al::Vec4f>
{
	virtual void set(Vec4f value) override
	{
		if (mProcessCallback) {
			value = mProcessCallback(value, mProcessUdata);
		}
		setLocking(value);
		for(size_t i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](value, this,  mCallbackUdata[i], NULL);
			}
		}
	}

	virtual void setNoCalls(Vec4f value, void *blockReceiver) override
	{
		if (mProcessCallback) {
			value = mProcessCallback(value, mProcessUdata);
		}
		if (blockReceiver) {
			for(size_t i = 0; i < mCallbacks.size(); ++i) {
				if (mCallbacks[i]) {
					mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
				}
			}
		}
		setLocking(value);
	}
};

/**
 * @brief The ParameterServer class creates an OSC server to receive parameter values
 * 
 * Parameter objects that are registered with a ParameterServer will receive 
 * incoming messages on their OSC address.
 *
 * @ingroup allocore
 */
class ParameterServer : public osc::PacketHandler, public OSCNotifier
{
	friend class PresetServer; // To be able to take over the OSC server
public:
	/**
	 * @brief ParameterServer constructor
	 * 
	 * @param oscAddress The network address on which to listen to. If empty use all available network interfaces. Defaults to "127.0.0.1".
	 * @param oscPort The network port on which to listen. Defaults to 9010.
	 * 
	 * Usage:
	 * @code
	Parameter freq("Frequency", "", 440.0);
	Parameter amp("Amplitude", "", 0.1);
	ParameterServer paramServer;
	paramServer << freq << amp;
	 @endcode
	 */
	ParameterServer(std::string oscAddress = "127.0.0.1", int oscPort = 9010);
	~ParameterServer();
	
	/**
	 * Register a parameter with the server.
	 */
	ParameterServer &registerParameter(Parameter &param);

	/**
	 * Remove a parameter from the server.
	 */
	void unregisterParameter(Parameter &param);

    /**
	 * Register a string parameter with the server.
	 */
	ParameterServer &registerParameter(ParameterString &param);

	/**
	 * Remove a string parameter from the server.
	 */
	void unregisterParameter(ParameterString &param);

	/**
	 * Register a Vec3 parameter with the server.
	 */
	ParameterServer &registerParameter(ParameterVec3 &param);

	/**
	 * Remove a Vec3 parameter from the server.
	 */
	void unregisterParameter(ParameterVec3 &param);

	/**
	 * Register a Vec4 parameter with the server.
	 */
	ParameterServer &registerParameter(ParameterVec4 &param);

	/**
	 * Remove a Vec4 parameter from the server.
	 */
	void unregisterParameter(ParameterVec4 &param);

	/**
	 * @brief print prints information about the server to std::out
	 *
	 * The print function will print the server configuration (address and port)
	 * and will list the parameters with their addresses.
	 */
	void print();

	/**
	 * @brief stopServer stops the OSC server thread. Calling this function
	 * is sometimes required when this object is destroyed abruptly and the
	 * destructor is not called
	 */
	void stopServer();

	bool serverRunning() { return (mServer != nullptr); }

	/**
	 * @brief Get the list of registered parameters.
	 */
	std::vector<Parameter *> parameters() {return mParameters;}

	/// Register parameter using the streaming operator
	ParameterServer &operator << (Parameter& newParam){ return registerParameter(newParam); }

	/// Register parameter using the streaming operator
	ParameterServer &operator << (Parameter* newParam){ return registerParameter(*newParam); }

    /// Register generic parameter using the streaming operator
	ParameterServer &operator << (ParameterString& newParam){ return registerParameter(newParam); }

	/// Register generic parameter using the streaming operator
	ParameterServer &operator << (ParameterString* newParam){ return registerParameter(*newParam); }

	/// Register generic parameter using the streaming operator
	ParameterServer &operator << (ParameterVec3& newParam){ return registerParameter(newParam); }

	/// Register generic parameter using the streaming operator
	ParameterServer &operator << (ParameterVec3* newParam){ return registerParameter(*newParam); }

	/// Register generic parameter using the streaming operator
	ParameterServer &operator << (ParameterVec4& newParam){ return registerParameter(newParam); }

	/// Register generic parameter using the streaming operator
	ParameterServer &operator << (ParameterVec4* newParam){ return registerParameter(*newParam); }

	/**
	 * @brief Append a listener to the osc server.
	 * @param handler
	 * OSC messages received by this server will be forwarded to all
	 * registered listeners. This is the mechanism internally used to share a
	 * network port between a ParameterServer, a PresetServer and a SequenceServer
	 */
	void registerOSCListener(osc::PacketHandler *handler);

	void notifyAll();

	virtual void onMessage(osc::Message& m);

protected:
	static void changeCallback(float value, void *sender, void *userData, void *blockThis);
	static void changeStringCallback(std::string value, void *sender, void *userData, void *blockThis);
	static void changeVec3Callback(Vec3f value, void *sender, void *userData, void *blockThis);
	static void changeVec4Callback(Vec4f value, void *sender, void *userData, void *blockThis);

private:
	std::vector<osc::PacketHandler *> mPacketHandlers;
	osc::Recv *mServer;
	std::vector<Parameter *> mParameters;
	std::vector<ParameterString *> mStringParameters;
	std::vector<ParameterVec3 *> mVec3Parameters;
	std::vector<ParameterVec4 *> mVec4Parameters;
    std::mutex mParameterLock;
};

// Implementations -----------------------------------------------------------

template<class ParameterType>
ParameterWrapper<ParameterType>::~ParameterWrapper()
{
}

template<class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(std::string parameterName, std::string group,
          ParameterType defaultValue,
          std::string prefix) :
    mParameterName(parameterName), mGroup(group), mPrefix(prefix), mProcessCallback(nullptr)
{
	//TODO: Add better heuristics for slash handling
	if (mPrefix.length() > 0 && mPrefix.at(0) != '/') {
		mFullAddress = "/";
	}
	mFullAddress += mPrefix;
	if (mPrefix.length() > 0 && mPrefix.at(mPrefix.length() - 1) != '/') {
		mFullAddress += "/";
	}
	if (mGroup.length() > 0 && mGroup.at(0) != '/') {
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


template<class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(std::string parameterName, std::string group, ParameterType defaultValue,
                     std::string prefix, ParameterType min, ParameterType max) :
    ParameterWrapper<ParameterType>::ParameterWrapper(parameterName, group,
                                                      defaultValue, prefix)
{
	mMin = min;
	mMax = max;
}

template<class ParameterType>
ParameterWrapper<ParameterType>::ParameterWrapper(const ParameterWrapper<ParameterType> &param)
{
	mParameterName = param.mParameterName;
	mGroup = param.mGroup;
	mPrefix = param.mPrefix;
	mProcessCallback = param.mProcessCallback;
	mMin = param.mMin;
	mMax = param.mMax;
	mProcessCallback = param.mProcessCallback;
	mProcessUdata = param.mProcessUdata;
	mCallbacks = param.mCallbacks;
	mCallbackUdata = param.mCallbackUdata;

	//TODO: Add better heuristics for slash handling
	if (mPrefix.length() > 0 && mPrefix.at(0) != '/') {
		mFullAddress = "/";
	}
	mFullAddress += mPrefix;
	if (mPrefix.length() > 0 && mPrefix.at(mPrefix.length() - 1) != '/') {
		mFullAddress += "/";
	}
	if (mGroup.length() > 0 && mGroup.at(0) != '/') {
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

template<class ParameterType>
void ParameterWrapper<ParameterType>::set(ParameterType value)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}
	mMutex.lock();
	mValue = value;
	mMutex.unlock();
	for(size_t i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](value, this,  mCallbackUdata[i], NULL);
		}
	}
}

template<class ParameterType>
void ParameterWrapper<ParameterType>::setNoCalls(ParameterType value, void *blockReceiver)
{
	if (value > mMax) value = mMax;
	if (value < mMin) value = mMin;
	if (mProcessCallback) {
		value = mProcessCallback(value, mProcessUdata);
	}

	if (blockReceiver) {
		for(size_t i = 0; i < mCallbacks.size(); ++i) {
			if (mCallbacks[i]) {
				mCallbacks[i](value, this, mCallbackUdata[i], blockReceiver);
			}
		}
	}
	mMutex.lock();
	mValue = value;
	mMutex.unlock();
}



template<class ParameterType>
ParameterType ParameterWrapper<ParameterType>::get()
{
	if (mMutex.try_lock()) {
		mValueCache = mValue;
		mMutex.unlock();
	}
	return mValueCache;
}

template<class ParameterType>
std::string ParameterWrapper<ParameterType>::getFullAddress()
{
	return mFullAddress;
}

template<class ParameterType>
std::string ParameterWrapper<ParameterType>::getName()
{
	return mParameterName;
}

template<class ParameterType>
void ParameterWrapper<ParameterType>::setProcessingCallback(ParameterWrapper::ParameterProcessCallback cb, void *userData)
{
	mProcessCallback = cb;
	mProcessUdata = userData;
}

template<class ParameterType>
void ParameterWrapper<ParameterType>::registerChangeCallback(ParameterWrapper::ParameterChangeCallback cb, void *userData)
{
	mCallbacks.push_back(cb);
    mCallbackUdata.push_back(userData);
}

}


#endif // AL_PARAMETER_H
