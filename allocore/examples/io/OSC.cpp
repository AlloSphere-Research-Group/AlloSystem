/*
Allocore Example: OSC

Description:
The example demonstrates how to construct, send, and receive OSC packets.

Author:
Lance Putnam, 4/25/2011
*/


#include <assert.h>
#include <stdio.h>
#include "allocore/al_Allocore.hpp"
using namespace al;


/// Test data we would like to send/receive
struct PacketData{
	PacketData(): i(123456), f(1), d(1), c(1){}
	int i;
	float f;
	double d;
	char c;
	
	void clear(){ i=0; f=0; d=0; c=0; }
	bool valid() const { return 123456==i && 1==f && 1==d && 1==c; }
	void print() const { printf("%x %g %g %d\n", i, f, d, c); }
};


int main(){

	int port = 11111;
	const char * addr = "127.0.0.1";

	// Some non-trivial test data to pass around
	PacketData data;


	// -------------------------------------------------------------------------
	// Constructing packets
	{
		osc::Packet p;

		// Create a message-only packet
		p.beginMessage("/test");
			p << "a string" << data.i << data.f << data.d << data.c;
			p << osc::Blob(&data, sizeof(data));
		p.endMessage();

		assert(!p.isBundle());
		assert(p.isMessage());

		printf("\nOSC Message\n");
		p.printRaw();


		// Create a more complicated packet containing bundles
		p.clear(); // clear any previous contents
		p.beginBundle(12345);
			p.addMessage("/message11", (int)0x12345678, 1.f, 1., "hello world!");
			p.addMessage("/message12", (int)0x23456789);
			p.beginBundle(12346);
				p.addMessage("/message21", (int)0x3456789a);
				p.beginBundle(12347);
					p.addMessage("/message31", (int)0x456789ab);
				p.endBundle();
			p.endBundle();
			p.addMessage("/message13", (int)0x56789abc);
		p.endBundle();
		
		assert(p.isBundle());
		assert(!p.isMessage());

		printf("\nOSC Bundle\n");
		p.printRaw();
	}

	// -------------------------------------------------------------------------
	// Sending packets
	{		
		osc::Send s(port, addr);
		
		// A simple way to send a message
		s.send("/foo", 1, 2.3, "four");

		// Sending a fairly complex time-tagged bundle
		osc::TimeTag timeNow = 0;
		osc::TimeTag dt = 1;

		s.beginBundle(timeNow);		
			s.addMessage("/message11", 12345678, 1.f, 1., "hello world!");
			s.addMessage("/message12", 23456789);
			s.beginBundle(timeNow + dt);
				s.addMessage("/message21", 3456789);
				s.beginBundle(timeNow + dt*2);
					s.addMessage("/message31", 456789);
				s.endBundle();
			s.endBundle();
			s.addMessage("/message13", 56789);
		s.endBundle();
		
		s.send();

		// If sending data infrequently, we can just use a temporary object
		osc::Send(port, addr).send("/foo", 1, 2.3, "four");
	}


	// -------------------------------------------------------------------------
	// Receiving packets
	{
		struct OSCHandler : public osc::PacketHandler{
			void onMessage(osc::Message& m){
			
				m.print();
			
				assert(m.typeTags() == "sifdcb");
				assert(m.addressPattern() == "/test");

				std::string s;
				PacketData d;
				osc::Blob b;

				d.clear();
				m >> s >> d.i >> d.f >> d.d >> d.c >> b;

				assert(s == "a string");
				assert(d.valid());
				assert(((const PacketData *)b.data)->valid());
			}
		} handler;


		int numTrials = 40;
		osc::Send s(port, addr);
		osc::Recv r(port);

		// Assign a handler to the receiver
		r.handler(handler);

		// Here we launch a background thread that automatically checks the
		// socket for incoming OSC packets.
		r.timeout(0.1); // set receiver to block with timeout
		r.start();

		for(int i=0; i<numTrials; ++i){
			s.clear();
			s.beginBundle(i);
			s.beginMessage("/test");
				s << "a string" << data.i << data.f << data.d << data.c;
				s << osc::Blob(&data, sizeof(data));
			s.endMessage();
			s.endBundle();
			s.send();
			al_sleep(0.02);
		}

		r.stop();


		// If we want more control over when to check for packets, we can poll
		// the receiver manually.
//		r.timeout(0);	// do not block
//
//		for(int i=0; i<numTrials; ++i){
//			s.clear();
//			s.beginBundle(i);
//			s.beginMessage("/test");
//				s << "a string" << data.i << data.f << data.d << data.c;
//				s << osc::Blob(&data, sizeof(data));
//			s.endMessage();
//			s.endBundle();
//			s.send();
//			
//			while(r.recv()){}
//		}

	}

	return 0;
}

/*
TODO: example code previous in header file

	// Poll socket manually at periodic intervals ...
	r.timeout(0);	// set to be non-blocking
	void myThreadFunc(){
		while (r.recv()) {}	// use while loop to empty queue
	}

	// or, launch an automatic background thread
	r.timeout(1);	// ensure waiting period is greater than 0
	r.start(); 

*/
