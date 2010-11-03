#include "allocore/protocol/al_OSCAPR.hpp"
#include "allocore/system/al_Thread.hpp"

#include "../private/al_ImplAPR.h"

#ifdef AL_LINUX
    #include "apr-1.0/apr_network_io.h"
#else
    #include "apr-1/apr_network_io.h"
#endif

namespace osc {

class OscRecvImplAPR : public Recv::Impl, public al::ImplAPR {
public:

	OscRecvImplAPR(Recv * recv, unsigned int port) : Recv::Impl(), ImplAPR(), mRecv(recv) {
		// create socket:
		apr_sockaddr_t * sa;
		apr_socket_t * sock;
		al::check_apr(apr_sockaddr_info_get(&sa, NULL, APR_INET, port, 0, mPool));
		al::check_apr(apr_socket_create(&sock, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));
		al::check_apr(apr_socket_bind(sock, sa));
		al::check_apr(apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1));
		mAddress = sa;
		mSock = sock;
	}
	virtual ~OscRecvImplAPR() {
		apr_socket_close(mSock);
	}

	virtual size_t recv(char * buffer, size_t maxlen) {
		apr_size_t len = maxlen;
		apr_status_t res = apr_socket_recv(mSock, buffer, &len);
		// only error check if not error# 35: Resource temporarily unavailable
		if(len){ al::check_apr(res); }
		return res == 0 ? len : 0;
	}


	static void * RecvThreadFunction(void * ptr) {
		OscRecvImplAPR * self = (OscRecvImplAPR *)ptr;
		Recv * recv = self->mRecv;
		Recv::MessageParser handler = self->mBackgroundHandler;
		void * userdata = self->mBackgroundPtr;
		size_t maxlen = self->mBackgroundMaxLen;
		al_sec period = self->mBackgroundPeriod;
		while (recv->background()) {
			recv->recv(handler, userdata, maxlen);
			al_sleep(period);
		}
		return 0;
	}

	virtual void start(Recv::MessageParser handler, void * userdata, size_t maxlen, al_sec period) {
		mBackgroundHandler = handler;
		mBackgroundMaxLen = maxlen;
		mBackgroundPeriod = period;
		mBackgroundPtr = userdata;
		mThread.start(RecvThreadFunction, this);
	}

	virtual void stop() {
		mThread.wait();
	}

	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
	al::Thread mThread;

	Recv * mRecv;
	Recv::MessageParser mBackgroundHandler;
	void * mBackgroundPtr;
	size_t mBackgroundMaxLen;
	al_sec mBackgroundPeriod;
};

class OscSendImplAPR : public Send::Impl, public al::ImplAPR {
public:
	OscSendImplAPR(const char * address, unsigned int port) :
	Send::Impl(), ImplAPR()
	{
		/* @see http://dev.ariel-networks.com/apr/apr-tutorial/html/apr-tutorial-13.html */
		// create socket:
		apr_sockaddr_t * sa;
		apr_socket_t * sock;
		al::check_apr(apr_sockaddr_info_get(&sa, address, APR_INET, port, 0, mPool));
		al::check_apr(apr_socket_create(&sock, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mPool));
		al::check_apr(apr_socket_connect(sock, sa));
		al::check_apr(apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1));
		mAddress = sa;
		mSock = sock;
	}

	virtual ~OscSendImplAPR() {
		apr_socket_close(mSock);
	}

	virtual size_t send(const char * buffer, size_t len) {
		apr_size_t size = len;
		//al::check_apr(apr_socket_send(mSock, buffer, &size));
		apr_socket_send(mSock, buffer, &size);
		return size;
	}

	apr_sockaddr_t * mAddress;
	apr_socket_t * mSock;
};

Recv::Recv(unsigned int port) : mPort(port), mBackground(false) {
	mImpl = new OscRecvImplAPR(this, port);
}

Recv::~Recv() {
	delete mImpl;
}

void Recv::start(MessageParser handler, void * userdata, size_t maxlen, al_sec period) {
	if (!mBackground) {
		mImpl->start(handler, userdata, maxlen, period);
	}
}

void Recv::stop() {
	if (mBackground) {
		mBackground = false;
		mImpl->stop();
	}
}

static void parsebundle(const osc::ReceivedBundle & p, Recv::MessageParser handler, void * userdata) {
	for(osc::ReceivedBundle::const_iterator i=p.ElementsBegin(); i != p.ElementsEnd(); ++i) {
		if(i->IsBundle()) {
			parsebundle(osc::ReceivedBundle(*i), handler, userdata);
		} else {
			(handler)(osc::ReceivedMessage(*i), userdata);
		}
	}
}

size_t Recv::recv(char * buffer, size_t maxlen) {
	return mImpl->recv(buffer, maxlen);
}

void Recv::recv(MessageParser handler, void * userdata, size_t maxlen) {
	char buffer[maxlen];
	size_t len = mImpl->recv(buffer, maxlen);
	while (len) {
		osc::ReceivedPacket p(buffer, len);
		if(p.IsBundle()) {
			parsebundle(osc::ReceivedBundle(p), handler, userdata);
		} else {
			(handler)(osc::ReceivedMessage(p), userdata);
		}
		len = mImpl->recv(buffer, maxlen);
	}
}

Send::Send(const char * address, unsigned int port, int maxPacketSizeBytes)
: mPort(port), mBuffer(0), mStream(0) {
	maxPacketSize(maxPacketSizeBytes);
	mImpl = new OscSendImplAPR(address, port);
}

Send::~Send() {
	delete mImpl;
}

size_t Send::send(const char * buffer, size_t len) {
	return mImpl->send(buffer, len);
}

size_t Send::send(osc::OutboundPacketStream & packet) {
	size_t r = mImpl->send(packet.Data(), packet.Size());
	packet.Clear();
	return r;
}

void Send::maxPacketSize(int bytes){
	if(mBuffer) delete[] mBuffer;
	mBuffer = new char[bytes];
	if(mStream) delete mStream;
	mStream = new osc::OutboundPacketStream(mBuffer, bytes);
	mStream->Clear();
}

} // osc::
