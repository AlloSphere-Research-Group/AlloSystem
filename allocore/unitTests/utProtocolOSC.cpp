#include "utAllocore.h"

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

#include "catch.hpp"

TEST_CASE( "ProtocolOSC", "[protocol]" ) {

	using namespace al::osc;

	Packet p;
		REQUIRE(p.size() == 0);

	p.addMessage("/test", 1, 1.f, 1.0, '1', "1", std::string("1"), Blob("1",1));
		REQUIRE(p.size() != 0);
		REQUIRE(p.isMessage());
		REQUIRE(!p.isBundle());

	p.clear();
		REQUIRE(p.size() == 0);

	p.beginBundle();
	p.addMessage("/test", 1, 1.f, 1.0, '1', "1", std::string("1"), Blob("1",1));
	p.endBundle();
		REQUIRE(p.size() != 0);
		REQUIRE(!p.isMessage());
		REQUIRE(p.isBundle());

	p.clear();

	// Test message
	{
		const char * str = "Hello World!";
		p.addMessage("/test",
			1, 1.f, 1.0, '1',
			str, std::string(str), Blob(str, strlen(str)));

		Message m(p.data(), p.size());

			REQUIRE(m.addressPattern() == "/test");
			REQUIRE(m.typeTags() == "ifdcssb");

		int i=0; float f=0; double d=0; char c=0;
		const char * cs; std::string ss;
		Blob b;

		m >> i >> f >> d >> c >> cs >> ss >> b;

			REQUIRE( 1 == i);
			REQUIRE( 1 == f);
			REQUIRE( 1 == d);
			REQUIRE('1'== c);
			REQUIRE(strcmp(cs, str) == 0);
			REQUIRE(ss == str);
			REQUIRE(int(strlen(str)) == int(b.size));
			REQUIRE(!strcmp((char *)b.data, str));
	}


	// Create a complicated OSC bundle packet
	p.clear();
	p.beginBundle(12345);
		p.addMessage("/test");
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

	REQUIRE(p.isBundle());
	REQUIRE(!p.isMessage());


	PacketData data;
	{
		struct OSCHandler : public osc::PacketHandler{
			void onMessage(osc::Message& m){

//				m.print();

				REQUIRE(m.typeTags() == "sifdcb");
				REQUIRE(m.addressPattern() == "/test");

				std::string s;
				PacketData d;
				osc::Blob b;

				d.clear();
				m >> s >> d.i >> d.f >> d.d >> d.c >> b;

				REQUIRE(s == "a string");
				REQUIRE(d.valid());
				REQUIRE(((const PacketData *)b.data)->valid());
			}
		} handler;

		int numTrials = 40;
		unsigned port = 4110;
		osc::Send s(port, "127.0.0.1");
		osc::Recv r(port);

		r.timeout(0.1);
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

			al_sleep(0.01);
		}
	}
}
