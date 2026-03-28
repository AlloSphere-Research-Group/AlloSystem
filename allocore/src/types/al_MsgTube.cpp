#include <cstring> // memcpy
#include "allocore/types/al_MsgTube.hpp"
#include "allocore/system/al_Printing.hpp" // AL_WARN

using namespace al;

MsgTube::MsgTube(int bits)
:	now(0),
	memsize(1<<bits),
	rb(memsize)
{
	//printf("created buffer of %d size\n", memsize);
}

MsgTube::~MsgTube() {
	// TODO: empty the cache
}

void MsgTube::executeUntil(al_sec until){
	Header header;
	while(rb.readSpace() > sizeof(header)){
		rb.peek((char *)&header, sizeof(header));
		if(header.t > until){
			return;
		}
		char buf[header.size];
		rb.read(buf, header.size);
		(header.func)(buf);
	}
}

void MsgTube::send_data(void (*func)(al_sec t, char * data), char * data, size_t size){
	struct Data {
		Header header;
		void (*f)(al_sec t, char * args);

		static void call(char * args){
			const Data * d = (Data *)args;
			(d->f)(d->header.t, args + sizeof(Data));
		}
	};

	auto packetsize = sizeof(Data) + size;
	char packet[packetsize];

	auto * d = (Data *)packet;
	d->header.size = packetsize;
	d->header.t = now;
	d->header.func = Data::call;
	d->f = func;
	memcpy(packet+sizeof(Data), data, size);
	writeData(packet, packetsize);
}

void MsgTube::cache(const void * src, size_t size) {
	auto * mem = new char[size];
	memcpy(mem, src, size);
	cacheq.push(mem);
}

bool MsgTube::flushCache() {
	while(!cacheq.empty()){
		auto * data = cacheq.front();
		auto size = *((size_t *)data);
		if(size >= rb.writeSpace())
			return false;

		// send cached message:
		rb.write(data, size);
		delete[] data;
		cacheq.pop();
	}
	return true;
}

void MsgTube::writeData(char * data, size_t size) {
	if(size >= memsize){
		AL_WARN("ERROR WRITING TO RINGBUFFER");
	} else if(flushCache() && rb.writeSpace() >= size){
		//printf("scheduled message\n");
		rb.write(data, size);
	} else {
		//printf("cached message\n");
		auto * cpy = new char[size];
		memcpy(cpy, data, size);
		cache(cpy, size);
	}
}


