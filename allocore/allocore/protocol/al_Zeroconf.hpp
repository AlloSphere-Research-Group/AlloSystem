#ifndef INCLUDE_AL_ZEROCONF_HPP
#define INCLUDE_AL_ZEROCONF_HPP

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
	Wrapper for mDNS zeroconf discovery

	File author(s):
	Charlie Roberts, 2012, charlie@charlie-roberts.com
	Graham Wakefield, 2012, grrrwaaa@gmail.com

*/

#include <string>

namespace al{
namespace zero{

///
/// \brief The Client class
///
/// @ingroup allocore
class Client {
public:
	class Impl;

	Client(const std::string& type = "_osc._udp.", const std::string& domain = "local.");
	virtual ~Client();

	///! check for new services:
	static void poll(al_sec interval = 0.01);

	///! called when a new service name is added:
	virtual void onServiceNew(const std::string& name) {}

	///! usually called after onServiceNew
	/// identifies the host/port/address(es) associated with the service name
	virtual void onServiceResolved(const std::string& name, const std::string& host_name, uint16_t port, const std::string& address) {}

	///! called when existing service name is removed:
	virtual void onServiceRemove(const std::string& name) {}

protected:
	std::string type, domain;
	Impl * mImpl;
};

///
/// \brief The Service class
///
/// @ingroup allocore
class Service {
public:
	class Impl;

	///! create and publish a new service
	/// the name should be unique
	Service(const std::string& name, uint16_t port=4110, const std::string& type="_osc._udp.", const std::string& domain="local.");

	virtual ~Service();

protected:
	Impl * mImpl;
};

} // zero::
} // al::

#endif
