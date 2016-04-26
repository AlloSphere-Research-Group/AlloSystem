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
#include "allocore/protocol/al_OSC.hpp"

namespace al
{

/**
 * @brief The Parameter class
 * 
 * The Parameter class offers a convenient mechanism for safely passing 
 * parameter values from a low priority ot a high priority thread, for example
 * from a computation thread to an audio thread (i.e. the audio callback).
 * 
 * The function is thread safe and there can be any number of readers and writers.
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
 * The mechanism used to protect data is locking a mutex within the set() function
 * and doing try_lock() on the mutex to update a cached value in the get()
 * function. In the worst case this might incur some jitter when reading the value.
 * 
 * The ParameterServer class allows exposing Parameter objects via OSC.
 *
 * @ingroup allocore
 */
class Parameter{
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
   */
	Parameter(std::string parameterName, std::string Group,
	          float defaultValue,
	          std::string prefix ="",
	          float min = -99999.0,
	          float max = 99999.0
	        );
	
	~Parameter();
	
	/**
	 * @brief set the parameter's value
	 * 
	 * This function is thread-safe and can be called from any number of threads
	 */
	void set(float value);

	/**
	 * @brief set the parameter's value without calling callbacks
	 *
	 * This function is thread-safe and can be called from any number of threads.
	 * The processing callback is called, but the callbacks registered with
	 * registerChangeCallback() are not called. This is useful to avoid infinite
	 * recursion when a widget sets the parameter that then sets the widget.
	 */
	void setNoCalls(float value);

	/**
	 * @brief get the parameter's value
	 * 
	 * This function is thread-safe and can be called from any number of threads
	 * 
	 * @return the parameter value
	 */
	float get();
	
	/**
	 * @brief set the minimum value for the parameter
	 * 
	 * The value returned by the get() function will be clamped and will not go
	 * under the value set by this function.
	 */
	void min(float minValue) {mMin = minValue;}
	float min() {return mMin;}
	
	/**
	 * @brief set the maximum value for the parameter
	 * 
	 * The value returned by the get() function will be clamped and will not go
	 * over the value set by this function.
	 */
	void max(float maxValue) {mMax = maxValue;}
	float max() {return mMax;}
	
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

	typedef float (*ParameterProcessCallback)(float value, void *userData);
	typedef void (*ParameterChangeCallback)(float value, void *userData);

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

	std::vector<Parameter *> operator<< (Parameter &newParam)
	{ std::vector<Parameter *> paramList;
		paramList.push_back(&newParam);
		return paramList; }

	std::vector<Parameter *> &operator<< (std::vector<Parameter *> &paramVector)
	{ paramVector.push_back(this);
		return paramVector;
	}

	const float operator= (const float value) { this->set(value); return value; }
	
private:
	float mValue;
	float mValueCache;
	float mMin;
	float mMax;
	std::string mParameterName;
	std::string mGroup;
	std::string mPrefix;
	
	std::string mFullAddress;
	
	std::mutex mMutex;

	ParameterProcessCallback mProcessCallback;
	void * mProcessUdata;
	std::vector<ParameterChangeCallback> mCallbacks;
	std::vector<void *> mCallbackUdata;
};

/**
 * @brief The ParameterServer class creates an OSC server to receive parameter values
 * 
 * Parameter objects that are registered with a ParameterServer will receive 
 * incoming messages on their OSC address.
 *
 * @ingroup allocore
 */
class ParameterServer : public osc::PacketHandler
{
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
	
	virtual void onMessage(osc::Message& m);

	/**
	 * @brief print prints information about the server to std::out
	 *
	 * The print function will print the server configuration (address and port)
	 * and will list the parameters with their addresses.
	 */
	void print();

	/// Register parameter using the streaming operator
	ParameterServer &operator << (Parameter& newParam){ return registerParameter(newParam); }

	/// Register parameter using the streaming operator
	ParameterServer &operator << (Parameter* newParam){ return registerParameter(*newParam); }
	
private:
	osc::Recv *mServer;
	std::vector<Parameter *> mParameters;
	std::mutex mParameterLock;
};


/**
 * @brief The PresetHandler class handles sorting and recalling of presets.
 *
 * Presets are saved by name with the ".preset" suffix.
 */
class PresetHandler
{
public:
	/**
	 * @brief PresetHandler contructor
	 * @param verbose if true, print diagnostic messages
	 */
	PresetHandler(bool verbose = false);

	PresetHandler &registerParameter(Parameter &parameter);

	void storePreset(std::string name);

	void recallPreset(std::string name);

	std::vector<std::string> availablePresets();

	PresetHandler &operator << (Parameter &param) { return this->registerParameter(param); }

private:
	bool mVerbose;
	std::string mFileName;
	std::vector<Parameter *> mParameters;
	std::mutex mFileLock;
};

}


#endif // AL_PARAMETER_H
