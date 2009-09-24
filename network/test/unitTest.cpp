#include <assert.h>
#include "OSC.h"
#include "Timer.h"

using namespace allo;

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
	
	p.args() >> data.b >> data.f >> data.d >> data.i >> data.s;
	
	p.print();
}


int main(int argc, char* argv[]){

	Data data;

	osc::OSCRecv r(12000, onRecv, &data);
	r.start();
	Timer::sleepSec(0.5);


	osc::OSCSend s("127.0.0.1", 12000);

	// Send single message
	s << osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage;
	s.send();

	Timer::sleepSec(0.1);
	assert(data.ones());


	// Send bundle with one message
	data.clear();
	s	<< osc::BeginBundleImmediate
			<< osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage
		<< osc::EndBundle;
	s.send();

	Timer::sleepSec(0.1);
	assert(data.ones());
	
	return 0;
}
