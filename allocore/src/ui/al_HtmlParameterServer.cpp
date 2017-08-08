
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
	Serve an HTML GUI to connect to a ParameterServer using interface.js

	File author(s):
	2016 Andres Cabrera andres@mat.ucsb.edu
*/

#include "allocore/ui/al_HtmlParameterServer.hpp"

std::string htmlTemplateStart = R"(
<html>
        <head>
<script src='interface.js'></script>
        </head>
        <body>
        <script>
        var iface = new Interface.Panel({ useRelativeSizesAndPositions: true });
)";

std::string htmlTemplateEnd = R"(
iface.background = 'blue';
</script>
</body>
</html>
)";

std::string sliderTemplate =
        R"(
        var b = new Interface.Slider({
        label: 'trig freq',
        bounds:[.05,.05,.1,.9],
        target:"OSC", key:'/grain/grain_trig_dev'
        }))";

using namespace al;

HtmlParameterServer::HtmlParameterServer(std::string pathToInterfaceJs)
{
	mRootPath = pathToInterfaceJs;

#ifndef AL_WINDOWS

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0) {
		std::cout << "Error setting up process pipes." << std::endl;
	}

	mPid = fork();

	if (mPid < 0) {
	} else if (mPid == 0) {
		runInterfaceJs();
	}
	std::cout << "interface.simpleserver pid = " << mPid << std::endl;
	std::cout << "to kill: kill -9 " << mPid << std::endl;
#else
	std::cout << "HtmlParameterServer() not implemented for Windows." << std::endl;
	// TODO implement for Windows.
#endif
}

HtmlParameterServer::~HtmlParameterServer() {
#ifndef AL_WINDOWS
	if (mPid > 0) {
		int status;
		kill(mPid, SIGTERM);
		if (waitpid(mPid, &status, WNOHANG) != mPid) {
			std::cout << "Error shutting down interface.simpleserver" << std::endl;
		}
	}
#else
	// TODO implement for Windows.
#endif
}

void HtmlParameterServer::runInterfaceJs() {
#ifndef AL_WINDOWS
	close(p_stdin[WRITE]);
	dup2(p_stdin[READ], READ);
	close(p_stdout[READ]);
	dup2(p_stdout[WRITE], WRITE);
	std::string path = mRootPath + "/server";
	std::cout << path << std::endl;
	if (chdir(path.c_str()) < 0) {
		std::cout << "Could not find interface.js folder" << std::endl;
	}
	execl("/usr/bin/nodejs", "/usr/bin/nodejs", "interface.simpleserver.js", "--oscPort=15439","--oscOutPort=9010", NULL);
	perror("execl");
	exit(1);
#else

#endif
}

void HtmlParameterServer::writeHtmlFile() {
	std::string code = htmlTemplateStart;
	std::string addCode = "iface.add(";
	float padding = 0.01;
	float width = 0.04;
	float height = 0.92;
	for(int i = 0; i < mParameters.size(); ++i) {
		code += "var widget_" + std::to_string(i) + " = new Interface.Slider({\n";
		code += "label: '" + mParameters.at(i)->getName() + "',\n";
		code += "bounds: [" + std::to_string(padding + ((i + padding) * width)) + ",";
		code += std::to_string(padding) + ",";
		code += std::to_string(width) + ",";
		code += std::to_string(height) + "],\n";
		code += "min: " + std::to_string(mParameters.at(i)->min()) + ",\n";
		code += "max: " + std::to_string(mParameters.at(i)->max()) + ",\n";
		code += "target: \"OSC\", key: '" + mParameters.at(i)->getFullAddress() + "'\n";
		code += "});\n";
		addCode += "widget_" + std::to_string(i) + ",";
	}
	code += addCode.substr(0, addCode.size()-1) + ");\n";
	code += htmlTemplateEnd;

	std::ofstream f(mRootPath + "/server/interfaces/allogui.html");
	if (!f.is_open()) {
		std::cout << "Error opening html interface file for writing: "
		          << mRootPath + "/server/interfaces/allogui.html" << std::endl;
		return;
	}
	f << code << std::endl;
	if (f.bad()) {
		std::cout << "Error writing html interface file: "
		          << mRootPath + "/server/interfaces/allogui.html" << std::endl;
	}
	f.close();
}

HtmlParameterServer &HtmlParameterServer::addParameter(Parameter &param) {
	mParameters.push_back(&param);
	mServer << param;
	mServer.print();
	writeHtmlFile();
	return *this;
}
