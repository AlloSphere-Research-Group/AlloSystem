#include "utAllocore.h"

int utIOSocket(){

	int port = 7000;
	
	const char dataSend[] = "Hello World!";
	char dataRecv[128];

	SocketSend s("127.0.0.1", port);
	SocketRecv r(port);

	s.send(dataSend, sizeof(dataSend));
	al_sleep(0.1);
	r.recv(dataRecv, sizeof(dataRecv));

	assert(0 == strcmp(dataSend, dataRecv));

	{
		std::string v = Socket::hostName();
		printf("%s\n", v.c_str());
	}

	return 0;
}
