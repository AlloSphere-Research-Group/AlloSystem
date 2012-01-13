
#include "allocore/protocol/al_mDNS.hpp"

#ifdef AL_LINUX
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>
#include <avahi-common/timeval.h>
#else

#endif

#include <stdio.h>

using namespace al;

#ifdef AL_LINUX

class Zeroconf::Impl {
public:
	
	Impl(Zeroconf& self) : poller(0), client(0), browser(0) {
		int error;
		AvahiClientFlags flags = AVAHI_CLIENT_NO_FAIL;

		/* Allocate main loop object */
		if (!(poller = avahi_simple_poll_new())) {
	   		fprintf(stderr, "Failed to create simple poll object.\n");
	    	return;
		}	
		client = avahi_client_new(avahi_simple_poll_get(poller), flags, client_callback, &self, &error);
		/* Check wether creating the client object succeeded */
		if (!client) {
		    fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
			return;
		}

		/* Create the service browser */
		if (!(browser = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, self.type.c_str(), self.domain.c_str(), (AvahiLookupFlags)0, browse_callback, &self))) {
			fprintf(stderr, "Failed to create service browser: %s\n", avahi_strerror(avahi_client_errno(client)));
			return;
		}
	}

	~Impl() {
		if (client) avahi_client_free(client);
		if (poller) avahi_simple_poll_free(poller);
		if (browser) avahi_service_browser_free(browser);
	}

	static void client_callback(AvahiClient *client, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
		assert(client);
		Zeroconf * self = (Zeroconf *)userdata;
		/* Called whenever the client or server state changes */
		if (state == AVAHI_CLIENT_FAILURE) {
		    fprintf(stderr, "Server connection failure: %s\n", avahi_strerror(avahi_client_errno(client)));
		    avahi_simple_poll_quit(self->mImpl->poller);
		}
	}

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
		Zeroconf * self = (Zeroconf *)userdata;
		AvahiClient * client = self->mImpl->client;

		assert(b);

		/* Called whenever a new services becomes available on the LAN or is removed from the LAN */
		switch (event) {
		    case AVAHI_BROWSER_FAILURE:

		        fprintf(stderr, "(Browser) %s\n", avahi_strerror(avahi_client_errno(avahi_service_browser_get_client(b))));
		        avahi_simple_poll_quit(self->mImpl->poller);
		        return;

		    case AVAHI_BROWSER_NEW:
		        fprintf(stderr, "(Browser) NEW: service '%s' of type '%s' in domain '%s'\n", name, type, domain);

		        /* We ignore the returned resolver object. In the callback
		           function we free it. If the server is terminated before
		           the callback function is called the server will free
		           the resolver for us. */

		        if (!(avahi_service_resolver_new(client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolve_callback, client)))
		            fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(client)));

		        break;

		    case AVAHI_BROWSER_REMOVE:
		        fprintf(stderr, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'\n", name, type, domain);
		        break;

		    case AVAHI_BROWSER_ALL_FOR_NOW:
		    case AVAHI_BROWSER_CACHE_EXHAUSTED:
		        fprintf(stderr, "(Browser) %s\n", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
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

		Zeroconf * self = (Zeroconf *)userdata;

		assert(r);

		/* Called whenever a service has been resolved successfully or timed out */

		switch (event) {
		    case AVAHI_RESOLVER_FAILURE:
		        fprintf(stderr, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
		        break;

		    case AVAHI_RESOLVER_FOUND: {
		        char a[AVAHI_ADDRESS_STR_MAX], *t;

		        fprintf(stderr, "Service '%s' of type '%s' in domain '%s':\n", name, type, domain);

		        avahi_address_snprint(a, sizeof(a), address);
		        t = avahi_string_list_to_string(txt);
		        fprintf(stderr,
		                "\t%s:%u (%s)\n"
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
		    }
		}

		avahi_service_resolver_free(r);
	}

	void poll(al_sec timeout) {
		int sleep_time = timeout * 1000;
		int result = avahi_simple_poll_iterate(poller, sleep_time);
	}

	AvahiSimplePoll * poller;
	AvahiClient * client;
	AvahiServiceBrowser * browser;
};

#else

///! TODO: OSX
class Zeroconf::Impl {
public:
	
	Impl(Zeroconf& self) {}
	~Impl() {}

	void poll(al_sec timeout) {}
};

#endif

Zeroconf::Zeroconf(const std::string& type, const std::string& domain) 
:	type(type), domain(domain) {
	mImpl = new Impl(*this);
}

Zeroconf::~Zeroconf() {
	delete mImpl;
}

void Zeroconf::poll(al_sec timeout) {
	mImpl->poll(timeout);
}
