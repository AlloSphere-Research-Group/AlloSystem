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
 * @code
	Parameter freq("Frequency", "Group", default, "/path/prefix"); // This recieves on osc address "/path/prefix/Group/Frequency" 
	
	
	// In a simulator thread
	freq.set(var);
	
	// In the audio thread
	float curFreq = freq.get()
 * @endcode
 */
class Parameter{
public:
	/**
   * @brief Parameter
   * 
   * 
   * @param parameterName
   * @param Group
   * @param defaultValue
   * @param prefix
   */
	Parameter(std::string parameterName, std::string Group,
	          float defaultValue,
	          std::string prefix ="");
	
	~Parameter();
	
	void set(float value);
	float get();
	
	float min() {return mMin;}
	void min(float minValue) {mMin = minValue;}
	
	float max() {return mMax;}
	void max(float maxValue) {mMax = maxValue;}
	
	std::string getFullAddress();
	
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
};


class ParameterServer : public osc::PacketHandler
{
public:
	ParameterServer(std::string oscAddress = "127.0.0.1", int oscPort = 9010);
	~ParameterServer();
	
	void registerParameter(Parameter &param);
	
	void unregisterParameter(Parameter &param);
	
	virtual void onMessage(osc::Message& m);
	
private:
	osc::Recv *mServer;
	std::vector<Parameter *> mParameters;
};
}


#endif // AL_PARAMETER_H
