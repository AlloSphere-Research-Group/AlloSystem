/*
Allocore Example: Socket I/O

Description:
This demonstrates how to send and receive data over network sockets.

Author:
Lance Putnam, April 2014
*/

#include <stdio.h>
#include "allocore/al_Allocore.hpp"
using namespace al;


int main(){

	/* This demonstrates how to set up a client/server system using UDP.
	Here, everything is done locally. For remote connections, the client and
	server code would be running on separate machines.
	*/
	{
		unsigned short port = 11111;
		const char * addr = "127.0.0.1";
		char data[] = {'H','e','l','l','o',' ','N','e','t','w','o','r','k','!'};

		SocketClient client;
		SocketServer server;

		/* Open a client socket for sending data.

		The arguments are port number, IP address, timeout, and protocol.
		We set the timeout to 0 so that the socket does not block.
		*/
		client.open(port, addr, 0, Socket::UDP);

		/* Open a server socket for receiving data.

		The arguments are port number, IP address, timeout, and protocol.
		Here, the IP address is "" which designates that we will receive data
		from any address on the specified port. The timeout is set to -1 to make
		the socket block when a call to recv is made.
		*/
		server.open(port, "", -1, Socket::UDP);

		/* Send data

		Here we send our data out over the client socket. The return value of
		send is the number of bytes sent.
		*/
		int bytesSent = client.send(data, sizeof data);
		printf("UDP client sent %d bytes.\n", bytesSent);


		// Receive data
		char buf[128];

		// Typically, a server will loop to check for incoming packets
		while(1){
			/* Here is where we actually check to see if there is any incoming
			data on the socket. Because we set the timeout to -1, this call will
			block until a packet comes in. The return value is the number of
			bytes received.
			*/
			int bytesRecv = server.recv(buf, sizeof buf);

			if(bytesRecv > 0){
				printf("UDP server received %d bytes:\n", bytesRecv);
				for(int i=0; i<bytesRecv; ++i) printf("%c", buf[i]);
				printf("\n\n");
				break;
			}
		}
	}

	/* This shows how to make an HTTP request to a web URL.
	To do this, we create a TCP client that connects to the remote server on
	port 80. The client sends a request and then waits for a response from the
	server.
	*/
	{
		SocketClient client(80, "w2.mat.ucsb.edu", -1, Socket::TCP);

		// Always check first that the socket opened okay
		if(client.opened()){
			// This requests just the header information
			std::string req = "HEAD / HTTP/1.0\r\n\r\n";
			// This requests the default page
			//std::string req = "GET / HTTP/1.0\r\n\r\n";

			// Here we send the request out
			int bytesSent = client.send(req.c_str(), req.size());
			printf("TCP client sent %d bytes:\n%s", bytesSent, req.c_str());

			// Now, we go into a busy loop checking for a response from the server
			while(1){
				char buf[512];
				int n = client.recv(buf, sizeof buf);

				if(n <= 0) break;

				printf("TCP client received %d bytes:\n", n);
				for(int i=0; i<n; ++i) printf("%c", buf[i]);
				printf("\n\n");
			}
		}

	}
}
