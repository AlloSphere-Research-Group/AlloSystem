
#include "allocore/protocol/al_mDNS.hpp"

#include <stdio.h>
#include <stdlib.h>

// On OSX these are implemented in al_mDNS_OSX.mm instead
#ifndef AL_OSX

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>
#include <avahi-common/timeval.h>

using namespace al;
using namespace al::mdns;

template <typename Subclass>
class ImplBase {
public:
	ImplBase() : poller(0), client(0) {}

	void start() {
		int error;
		AvahiClientFlags flags = AVAHI_CLIENT_NO_FAIL;
		/* Allocate main loop object */
		if (!(poller = avahi_simple_poll_new())) {
	   		fprintf(stderr, "Zeroconf: Failed to create simple poll object.\n");
	    	return;
		}
		client = avahi_client_new(avahi_simple_poll_get(poller), flags, client_callback, this, &error);
		/* Check wether creating the client object succeeded */
		if (!client) {
			fprintf(stderr, "Zeroconf: Failed to create client: %s\n", avahi_strerror(error));
			return;
		}
	}

	~ImplBase() {
		if (client) avahi_client_free(client);
		if (poller) avahi_simple_poll_free(poller);
	}

	void poll(al_sec timeout) {
		int sleep_time = timeout * 1000;
		int result = avahi_simple_poll_iterate(poller, sleep_time);
	}

	static void client_callback(AvahiClient *client, AvahiClientState state, void * userdata) {
		assert(client);
		Subclass * self = (Subclass *)userdata;
		
		switch (state) {
		    case AVAHI_CLIENT_S_RUNNING:
		        /* The server has startup successfully and registered its host
		         * name on the network, so it's time to create our services */
				Subclass::create_services(client, (Subclass *)userdata);
		        break;

		    case AVAHI_CLIENT_FAILURE:
		        fprintf(stderr, "Zeroconf: Client failure: %s\n", avahi_strerror(avahi_client_errno(client)));
		        avahi_simple_poll_quit(self->poller);
		        break;

		    case AVAHI_CLIENT_S_COLLISION:
		        /* Let's drop our registered services. When the server is back
		         * in AVAHI_SERVER_RUNNING state we will register them
		         * again with the new host name. */

		    case AVAHI_CLIENT_S_REGISTERING:
		        /* The server records are now being established. This
		         * might be caused by a host name change. We need to wait
		         * for our own records to register until the host name is
		         * properly esatblished. */

		        //if (group) avahi_entry_group_reset(group);

		        break;

		    default:
				break;
		}
	}

	AvahiSimplePoll * poller;
	AvahiClient * client;
};

class Client::Impl : public ImplBase<Client::Impl> {
public:
	
	Impl(Client * master) : browser(0), master(master) {
		int error;
		AvahiClientFlags flags = AVAHI_CLIENT_NO_FAIL;
		start();
		if (client) {	
			/* Create the service browser */
			if (!(browser = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, master->type.c_str(), master->domain.c_str(), (AvahiLookupFlags)0, browse_callback, this))) {
				fprintf(stderr, "Zeroconf: Failed to create service browser: %s\n", avahi_strerror(avahi_client_errno(client)));
				return;
			}
		}
	}

	virtual ~Impl() {
		if (browser) avahi_service_browser_free(browser);
	}

	static void create_services(AvahiClient * client, Impl * self) {}

	static void browse_callback(
		AvahiServiceBrowser *b,
		AvahiIfIndex interface,
		AvahiProtocol protocol,
		AvahiBrowserEvent event,
		const char *name,
		const char *type,
		const char *domain,
		AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
		void* userdata) {
		Impl * self = (Impl *)userdata;
		AvahiClient * client = self->client;

		assert(b);

		/* Called whenever a new services becomes available on the LAN or is removed from the LAN */
		switch (event) {
		    case AVAHI_BROWSER_FAILURE:
		        fprintf(stderr, "Zeroconf: %s\n", avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(b))));
		        avahi_simple_poll_quit(self->poller);
		        return;

		    case AVAHI_BROWSER_NEW:
				self->master->onServiceNew(name, type, domain);

				/* We ignore the returned resolver object. In the callback
				   function we free it. If the server is terminated before
				   the callback function is called the server will free
				   the resolver for us. */

				if (!(avahi_service_resolver_new(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolve_callback, self)))
				    fprintf(stderr, "Zeroconf: failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(client)));
		        break;

		    case AVAHI_BROWSER_REMOVE:
				self->master->onServiceRemove(name, type, domain);
		        break;

		    case AVAHI_BROWSER_ALL_FOR_NOW:
		    case AVAHI_BROWSER_CACHE_EXHAUSTED:
				// TODO: what do these mean?		        
				fprintf(stderr, "Zeroconf: %s\n", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
		        break;
		}
	}

	static void resolve_callback(
		AvahiServiceResolver *r,
		AVAHI_GCC_UNUSED AvahiIfIndex interface,
		AVAHI_GCC_UNUSED AvahiProtocol protocol,
		AvahiResolverEvent event,
		const char *name,
		const char *type,
		const char *domain,
		const char *host_name,
		const AvahiAddress *address,
		uint16_t port,
		AvahiStringList *txt,
		AvahiLookupResultFlags flags,
		void* userdata) {

		Impl * self = (Impl *)userdata;

		assert(r);

		/* Called whenever a service has been resolved successfully or timed out */

		switch (event) {
		    case AVAHI_RESOLVER_FAILURE:
		        fprintf(stderr, "Zeroconf: failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
		        break;

		    case AVAHI_RESOLVER_FOUND: {
		        char a[AVAHI_ADDRESS_STR_MAX], *t;

		        avahi_address_snprint(a, sizeof(a), address);
				self->master->onServiceResolved(name, host_name, port, a);

				/*
		        t = avahi_string_list_to_string(txt);
		        fprintf(stderr,
		                "\tname=%s\n"
						"\tport=%u\n"
						"\taddress= %s\n"
		                "\tTXT=%s\n"
		                "\tcookie is %u\n"
		                "\tis_local: %i\n"
		                "\tour_own: %i\n"
		                "\twide_area: %i\n"
		                "\tmulticast: %i\n"
		                "\tcached: %i\n",
		                host_name, port, a,
		                t,
		                avahi_string_list_get_service_cookie(txt),
		                !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
		                !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
		                !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
		                !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
		                !!(flags & AVAHI_LOOKUP_RESULT_CACHED));
		        avahi_free(t);
				*/
		    }
		}

		avahi_service_resolver_free(r);
	}

	AvahiServiceBrowser * browser;
	Client * master;
};

class Service::Impl : public ImplBase<Service::Impl> {
public:
	Impl(Service * master, const std::string& name, const std::string& host, uint16_t port, const std::string& type, const std::string& domain) 
:	name(name), host(host), type(type), domain(domain), port(port), group(0), master(master) {
		int ret;
		start();
		if (client) {	
			if (!(group = avahi_entry_group_new(client, entry_group_callback, NULL))) {
		        fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
		        return;
		    }

			/* If the group is empty (either because it was just created, or
				 * because it was reset previously, add our entries.  */

			if (avahi_entry_group_is_empty(group)) {
				printf("Zeroconf: Adding service '%s'\n", name.c_str());
			}

		    /* Add the service for IPP */
		    if ((ret = avahi_entry_group_add_service(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, name.c_str(), type.c_str(), domain.c_str(), host.c_str(), port, NULL)) < 0) {

		        if (ret == AVAHI_ERR_COLLISION)
		            printf("Zeroconf: name collision");

		        printf("Zeroconf: failed to add service %s on %s:%u: %s\n", name.c_str(), host.c_str(), port, avahi_strerror(ret));
		        return;
		    }

		}
	}

	~Impl() {

	}

	static void create_services(AvahiClient * client, Impl * self) {
		char *n, r[128];
		int ret;
		assert(client);

		/* If this is the first time we're called, let's create a new
		 * entry group if necessary */

		if (!self->group)
		    if (!(self->group = avahi_entry_group_new(client, entry_group_callback, self))) {
		        fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
		        goto fail;
		    }

		/* If the group is empty (either because it was just created, or
		 * because it was reset previously, add our entries.  */

		if (avahi_entry_group_is_empty(self->group)) {
		    fprintf(stderr, "Adding service '%s'\n", self->name.c_str());

		    /* Create some random TXT data */
		    snprintf(r, sizeof(r), "random=%i", rand());

		    /* Add the service for IPP */
		    if ((ret = avahi_entry_group_add_service(self->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, self->name.c_str(), self->type.c_str(), NULL, NULL, self->port, "test=blah", r, NULL)) < 0) {

		        if (ret == AVAHI_ERR_COLLISION)
		            goto collision;

		        fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
		        goto fail;
		    }

		    /* Tell the server to register the service */
		    if ((ret = avahi_entry_group_commit(self->group)) < 0) {
		        fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
		        goto fail;
		    }
		}

		return;

	collision:

		/* A service name collision with a local service happened. Let's
		 * pick a new name */
		n = avahi_alternative_service_name(self->name.c_str());
		self->name = n;

		fprintf(stderr, "Service name collision, renaming service to '%s'\n", self->name.c_str());

		avahi_entry_group_reset(self->group);

		create_services(client, self);
		return;

	fail:
		avahi_simple_poll_quit(self->poller);
	}

	static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
		Service * self = (Service *)userdata;
	}

	std::string name, host, type, domain;
	uint16_t port;
	AvahiEntryGroup * group;
	Service * master;
};

Client::Client(const std::string& type, const std::string& domain) 
:	type(type), domain(domain) {
	mImpl = new Impl(this);
}

Client::~Client() {
	delete mImpl;
}

void Client::poll(al_sec timeout) {
	mImpl->poll(timeout);
}

Service::Service(const std::string& name, uint16_t port, const std::string& type, const std::string& domain) {
	mImpl = new Impl(this, name, Socket::hostName(), port, type, domain);
}

Service::~Service() {
	delete mImpl;
}

void Service::poll(al_sec timeout) {
	mImpl->poll(timeout);
}


// end of non-OSX implementation
#endif