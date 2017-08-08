#ifndef INCLUDE_AL_HTMLINTERFACESERVER_HPP
#define INCLUDE_AL_HTMLINTERFACESERVER_HPP
/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
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
	Serve an HTML GUI to connect to a ParameterServer and PresetServer
    using interface.js

	File author(s):
	2016 Andres Cabrera andres@mat.ucsb.edu
*/

#include <string>
#include <iostream>
#include <fstream>

#ifndef AL_WINDOWS
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1
#else

#endif

#include "allocore/ui/al_Parameter.hpp"
#include "allocore/ui/al_Preset.hpp"

namespace al
{
/**
 * @brief The HtmlInterfaceServer class runs an interface.simpleserver.js
 * server and builds the html interface for the registered set of controllers
 * and presets.
 *
 * The HtmlInterfaceServer class runs an html service that provides a
 * GUI for Parameter objects. It runs interface.js in the background, so it
 * relies on an existing working installation of interface.js and interface.simpleserver.js
 * in particular. The class generates the html file for interface.js from
 * the registered parameters. Because it uses Parameter obejcts, the HTML
 * GUI can be kept in sync with other control interfaces like ParameterGUI
 * and other devices that set the parameters via OSC.
 */
class HtmlInterfaceServer {
public:
	HtmlInterfaceServer(std::string pathToInterfaceJs = "../interface.js");
	~HtmlInterfaceServer();

	HtmlInterfaceServer &addParameterServer(ParameterServer &paramServer,
	                                        std::string interfaceName = "");
	HtmlInterfaceServer &addPresetServer(PresetServer &presetServer,
	                                     std::string interfaceName = "");

	HtmlInterfaceServer &operator <<(ParameterServer &paramServer) {
		return this->addParameterServer(paramServer);
	}
	HtmlInterfaceServer &operator <<(PresetServer &presetServer) {
		return this->addPresetServer(presetServer);
	}

private:
	void writeHtmlFile(ParameterServer &paramServer, std::string interfaceName = "");
	void writeHtmlFile(PresetServer &presetServer, std::string interfaceName = "", int numPresets = -1);

	std::string mRootPath;
	std::string mNodeJsPath;
	void runInterfaceJs();
	pid_t  mPid;
	int p_stdin[2], p_stdout[2];

	int mInterfaceSendPort; // Interface.js sends OSC on this port
	int mInterfaceRecvPort; // Interface.js receives OSC on this port
};

} // ::al



#endif // INCLUDE_AL_HTMLINTERFACESERVER_HPP
