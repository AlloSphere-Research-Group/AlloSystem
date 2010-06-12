#include "utAllocore.h"

int utIOSocket(){

	int port = 4110;
	
	const char dataSend[] = "Hello World!";
	char dataRecv[128];

	SocketSend s(port, "ljp.local"); //, "127.0.0.1");
	SocketRecv r(port);

	s.send(dataSend, sizeof(dataSend));
	al_sleep(0.1);
	r.recv(dataRecv, sizeof(dataRecv));
	
//	while(true){
//		al_sleep(0.1);
//		dataRecv[0] = '\0';
//		r.recv(dataRecv, sizeof(dataRecv));
//		if(dataRecv[0]) printf("%s\n", dataRecv);
//	}

	assert(0 == strcmp(dataSend, dataRecv));

	{
		printf("%s\n", Socket::hostName().c_str());
		printf("%s\n", Socket::hostIP().c_str());
	}

	return 0;
}
