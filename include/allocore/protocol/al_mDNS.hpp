#ifndef INCLUDE_AL_MDNS_HPP
#define INCLUDE_AL_MDNS_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

	File description:
	Wrapper for mDNS zeroconf discovery

	File author(s):
	Charlie Roberts, 2012, charlie@charlie-roberts.com
	Graham Wakefield, 2012, grrrwaaa@gmail.com

*/

#include <string>
#include <vector>
#include "allocore/io/al_Socket.hpp"
#include "allocore/system/al_Thread.hpp"

namespace al{

class Zeroconf {
public:
	class Impl;

	Zeroconf(const std::string& type = "_osc._udp.", const std::string& domain = "local.");
	~Zeroconf();

	///! if timeout = 0, non-blocking
	///! if timeout < 0, block until first event
	///! if timeout > 0, block until next event or timeout seconds elapsed
	void poll(al_sec timeout=0);

protected:	
	std::string type, domain;
	Impl * mImpl;
};

} // al::
	
#endif
	
