#ifndef INC_AL_ZEROCONF_HPP
#define INC_AL_ZEROCONF_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Wrapper for mDNS zeroconf discovery

	Author(s):
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

	Client(const std::string& type = "_osc._udp.", const std::string& domain = "local.");
	virtual ~Client();

	///! check for new services:
	static void poll(double interval = 0.01);

	///! called when a new service name is added:
	virtual void onServiceNew(const std::string& name) {}

	///! usually called after onServiceNew
	/// identifies the host/port/address(es) associated with the service name
	virtual void onServiceResolved(const std::string& name, const std::string& host_name, unsigned char port, const std::string& address) {}

	///! called when existing service name is removed:
	virtual void onServiceRemove(const std::string& name) {}

protected:
	std::string type, domain;
	class Impl;
	Impl * mImpl;
};

///
/// \brief The Service class
///
/// @ingroup allocore
class Service {
public:

	///! create and publish a new service
	/// the name should be unique
	Service(const std::string& name, unsigned short port=4110, const std::string& type="_osc._udp.", const std::string& domain="local.");

	virtual ~Service();

protected:
	class Impl;
	Impl * mImpl;
};

} // zero::
} // al::
#endif
