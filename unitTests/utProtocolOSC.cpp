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

int utProtocolOSC(){
	PacketData data;
	
	{
		struct OSCHandler : public osc::PacketHandler{
			void onMessage(osc::Message& m){
			
				//m.print();
			
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
	return 0;
}
