#include "utAllocore.h"

int utIOSocket(){

	int port = 7000;
	
	const char dataSend[] = "Hello World!";
	char dataRecv[128];

	SocketSend s(port); //, "127.0.0.1");
	SocketRecv r(port);

	s.send(dataSend, sizeof(dataSend));
	al_sleep(0.1);
	r.recv(dataRecv, sizeof(dataRecv));

	assert(0 == strcmp(dataSend, dataRecv));

	{
		printf("%s\n", Socket::hostName().c_str());
		printf("%s\n", Socket::hostIP().c_str());
	}

	return 0;
}
