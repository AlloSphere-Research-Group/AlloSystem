#include <assert.h>
#include <stdio.h>
#include "allocore/al_Allocore.hpp"
using namespace al;


struct PacketData{
	PacketData(): i(0x12345678), f(1), d(1), c(1){}
	int i;
	float f;
	double d;
	char c;
	
	void clear(){ i=0; f=0; d=0; c=0; }
	bool valid() const { return 0x12345678==i && 1==f && 1==d && 1==c; }
	void print() const { printf("%x %g %g %d\n", i, f, d, c); }
};


int main(){

	osc::Send(1000).send("/foo", 1, 2.3, "four");

	osc::Packet p;
	PacketData data;
	
	// Create a complicated OSC bundle packet
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
//	p.printRaw();


	struct MyPacketHandler : public	osc::PacketHandler {
		void onMessage(osc::Message& m){ m.print(); }
	} ph;
	ph.parse(p.data(), p.size());


	// Create an OSC message packet
	p.clear();
	p.beginMessage("/test");
		p << "a string" << data.i << data.f << data.d << data.c;
		p << osc::Blob(&data, sizeof(data));
	p.endMessage();

	assert(!p.isBundle());
	assert(p.isMessage());

	printf("\nOSC Message\n");
//	p.printRaw();
	
	// Read data from message
	{
		osc::Message m(p.data(), p.size(), 1);

		printf("%s %s\n", m.addressPattern().c_str(), m.typeTags().c_str());

		const char * s;
		PacketData d;
		osc::Blob b;

		m >> s >> d.i >> d.f >> d.d >> d.c >> b;

//		d.print();
//		((const PacketData *)b.data)->print();
	}

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
		unsigned port = 4110;
		osc::Send s(port, "127.0.0.1");
		osc::Recv r(port);

		r.timeout(1);
		r.handler(handler);
		r.start();	// make sure timeout > 0

		for(int i=0; i<numTrials; ++i){
			s.clear();
			s.beginBundle(i);
			s.beginMessage("/test");
				s << "a string" << data.i << data.f << data.d << data.c;
				s << osc::Blob(&data, sizeof(data));
			s.endMessage();
			s.endBundle();
			s.send();
			//r.recv();
			al_sleep(0.02);
		}
		
		//al_sleep(1);
		r.stop();
	}
	return 0;
}
