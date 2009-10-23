#include <assert.h>
#include <string.h>
#include "OSC.h"
#include "al_time.h"

using namespace allo;

// Data structure containing the possible OSC data types
struct Data{
	Data(){ clear(); }

	bool b;
	float f;
	double d;
	osc::int32 i;
	const char * s;

	void clear(){ b=false; f=0; d=0; i=0; s=""; }
	bool ones(){ return b==true && f==1 && d==1 && i==1 && (strlen(s) && s[0] == '1'); }
};


// OSC packet receive callback
void onRecv(const osc::RecvPacket& p, void * user){
	Data& data = *(Data *)user;
	const std::string tags = p.tags();
	const std::string addr = p.addr();

	if(addr=="/test"){
		if(tags=="Tfdis" || tags=="Ffdis"){
			p.args() >> data.b >> data.f >> data.d >> data.i >> data.s;
		}
		else if(tags=="f"  ){ p.args() >> data.f; }
		else if(tags=="fd" ){ p.args() >> data.f >> data.d; }
		else if(tags=="fdi"){ p.args() >> data.f >> data.d >> data.i; }
	}
	//p.print();
}


int main(int argc, char* argv[]){

	Data data;
	
	struct wait{ wait(double t=0.01){ al_sleep(t); }};

	osc::OSCRecv r(12000, onRecv, &data);
	r.start();
	wait(0.5);

	osc::OSCSend s("127.0.0.1", 12000);

	// Send single message
	s << osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage;
	s.send();
	wait(); assert(data.ones());

	// Send bundle with one message
	data.clear();
	s	<< osc::BeginBundleImmediate
			<< osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage
		<< osc::EndBundle;
	s.send();
	wait(); assert(data.ones());

	// check that bundle messages are ordered
	s	<< osc::BeginBundleImmediate
			<< osc::BeginMessage("/test") << false << 0.f << 0. << 0 << "0" << osc::EndMessage
			<< osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage
		<< osc::EndBundle;
	s.send();
	wait(); assert(data.ones());


	// check easy send methods
	data.clear();
	s.send("/test", 1.f);		wait(); assert(data.f==1);
	s.send("/test", 0.f, 1.);	wait(); assert(data.f==0 && data.d==1);
	s.send("/test", 1.f, 0.,1);	wait(); assert(data.f==1 && data.d==0 && data.i==1);

	return 0;
}
