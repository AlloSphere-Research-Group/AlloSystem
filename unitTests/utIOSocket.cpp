#include "utAllocore.h"

int utIOSocket(){

	int numTrials = 20000;
	unsigned port = 4110;

	const char dataSend[] = "On the other hand, we denounce with righteous indignation and dislike men who are so beguiled and demoralized by the charms of pleasure of the moment, so blinded by desire, that they cannot foresee the pain and trouble that are bound to ensue; and equal blame belongs to those who fail in their duty through weakness of will, which is the same as saying through shrinking from toil and pain. These cases are perfectly simple and easy to distinguish. In a free hour, when our power of choice is untrammelled and when nothing prevents our being able to do what we like best, every pleasure is to be welcomed and every pain avoided. But in certain circumstances and owing to the claims of duty or the obligations of business it will frequently occur that pleasures have to be repudiated and annoyances accepted. The wise man therefore always holds in these matters to this principle of selection: he rejects pleasures to secure other greater pleasures, or else he endures pains to avoid worse pains.";
	char dataRecv[sizeof(dataSend)];

	SocketSend s(port, "127.0.0.1");
	SocketRecv r(port);
	
	assert(s.port() == port);

	// Make receiver block forever until a packet is received. 
	// All packets should be received.
	r.timeout(-1);

	for(int i=0; i<numTrials; ++i){
		dataRecv[0] = '\0';
		s.send(dataSend, sizeof(dataSend));
		r.recv(dataRecv, sizeof(dataRecv));
		assert(0 == strcmp(dataSend, dataRecv));
	}


	// Make receiver return immediately after checking for packets.
	// Most packets will likely not be received.
	r.timeout(0);
	int dropped = 0;

	for(int i=0; i<numTrials; ++i){
		dataRecv[0] = '\0';
		s.send(dataSend, sizeof(dataSend));
		r.recv(dataRecv, sizeof(dataRecv));
		if(!strcmp(dataSend, dataRecv)) ++dropped;
	}
	//printf("%d\n", dropped);
	assert(dropped);


	// Make receiver block for a short duration waiting for incoming packets.
	// All packets should be received.
	r.timeout(0.2);

	for(int i=0; i<numTrials; ++i){
		dataRecv[0] = '\0';
		s.send(dataSend, sizeof(dataSend));
		r.recv(dataRecv, sizeof(dataRecv));
		assert(0 == strcmp(dataSend, dataRecv));
	}


	{
		printf("%s\n", Socket::hostName().c_str());
		printf("%s\n", Socket::hostIP().c_str());
	}
	return 0;
}
