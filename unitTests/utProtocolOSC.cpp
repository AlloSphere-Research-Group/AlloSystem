#include "utAllocore.h"

/*
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
}


// APR-based OSC packet receive handler
void onRecv2(const osc::ReceivedMessage & p, void * userdata) {
	Data& data = *(Data *)userdata;
	const std::string tags = p.TypeTags();
	const std::string addr = p.AddressPattern();
	
	osc::ReceivedMessageArgumentStream args = p.ArgumentStream();

	if(addr=="/test"){
		if(tags=="Tfdis" || tags=="Ffdis"){
			args >> data.b >> data.f >> data.d >> data.i >> data.s;
		}
		else if(tags=="f"  ){ args >> data.f; }
		else if(tags=="fd" ){ args >> data.f >> data.d; }
		else if(tags=="fdi"){ args >> data.f >> data.d >> data.i; }
	}
}

//#define USE_APR

int utProtocolOSC(){

#ifdef USE_APR
	const int port = 12000;
	const char * ip = "127.0.0.1";

	Data data;
	
	struct wait{ wait(double t=0.01){ al_sleep(t); }};

	osc::Recv r(port);
	wait(0.5);
	
	osc::Send s(ip, port);
	char buf[OSC_DEFAULT_MAX_MESSAGE_LEN];
	osc::OutboundPacketStream packet(buf, OSC_DEFAULT_MAX_MESSAGE_LEN);
	
	// Send single message
	data.clear();
	packet << osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage;
	s.send(packet);
	packet.Clear();
	wait(); 
	r.recv(onRecv2, &data);
	assert(data.ones());
	

	// Send bundle with one message
	data.clear();
	packet	<< osc::BeginBundleImmediate
			<< osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage
		<< osc::EndBundle;
	s.send(packet);
	packet.Clear();
	wait(); 
	r.recv(onRecv2, &data);
	assert(data.ones());
	
	// check that bundle messages are ordered
	data.clear();
	packet	<< osc::BeginBundleImmediate
			<< osc::BeginMessage("/test") << false << 0.f << 0. << 0 << "0" << osc::EndMessage
			<< osc::BeginMessage("/test") << true << 1.f << 1. << 1 << "1" << osc::EndMessage
		<< osc::EndBundle;
	s.send(packet);
	packet.Clear();
	wait(); 
	r.recv(onRecv2, &data);
	assert(data.ones());

//	// check easy send methods
//	data.clear();
//	s.send("/test", 1.f);		
//	wait(); 
//	assert(data.f==1);
//	s.send("/test", 0.f, 1.);	
//	wait(); 
//	assert(data.f==0 && data.d==1);
//	s.send("/test", 1.f, 0.,1);	
//	wait(); 
//	assert(data.f==1 && data.d==0 && data.i==1);

#else
	const int port = 12000;
	const char * ip = "127.0.0.1";

	Data data;
	
	struct wait{ wait(double t=0.01){ al_sleep(t); }};

	osc::OSCRecv r(port, onRecv, &data);
	r.start();
	wait(0.5);

	osc::OSCSend s(ip, port);

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


	// check remove endpoint
	s.remove(ip, port);
	data.clear();
	s.send("/test", 1.f);		wait(); assert(data.f!=1);
	
#endif
	return 0;

}
*/
