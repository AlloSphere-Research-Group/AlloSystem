#ifndef INCLUDE_AL_MSG_TUBE_HPP
#define INCLUDE_AL_MSG_TUBE_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Passing functors between a pair of threads

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/system/al_Time.h"
#include "allocore/types/al_SingleRWRingBuffer.hpp"
#include <string.h>
#include <queue>
#include <cstring>

#define AL_MSGTUBE_DEFAULT_SIZE_BITS (14) // 16384 bytes

/*
	A C++ class for deferred function calls
	Using templates for type-checked functions of variable arguments
	Adding a cache queue to seamlessly handle buffer overflow
*/

namespace al {

class MsgTube {
public:

	/* 
		Messages in the ringbuffer have the following header structure:
	*/
	struct Header {
		size_t size;
		al_sec t;
		void (*func)(char * args);
	};
	
	
	/* 
		Timestamp applied to sent messages (should increase monotonically)
	*/
	al_sec now;

	/*
		(single-reader single-writer lock-free fifo)
	*/
	size_t memsize;
	SingleRWRingBuffer rb;
	
	/*
		Cache of messages, when the ringbuffer is full
		TODO: set a cache limit? track when flushing fails for a prolonged period?
	*/
	std::queue<char *> cacheq;
	
	MsgTube(int bits = AL_MSGTUBE_DEFAULT_SIZE_BITS);
	~MsgTube();
	
	void executeUntil(al_sec until);
	
	/*
		Copies 'data', so you can safely free it after this call
	*/
	void send_data(void (*func)(al_sec t, char * data), char * data, size_t size);
	
	void send(void (*f)(al_sec t)) {
		struct Data {
			Header header;
			void (*f)(al_sec t);
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f };
		writeData((char *)&data, sizeof(Data));
	}
	
	template<typename A1>
	void send(void (*f)(al_sec t, A1 a1), A1 a1) {
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1);
			A1 a1;	
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1 };
		writeData((char *)&data, sizeof(Data));
	}
	
	template<typename A1, typename A2>
	void send(void (*f)(al_sec t, A1 a1, A2 a2), A1 a1, A2 a2) {
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2);
			A1 a1; A2 a2;
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2 };
		writeData((char *)&data, sizeof(Data));
	}
	
	template<typename A1, typename A2, typename A3>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3), A1 a1, A2 a2, A3 a3) {
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3);
			A1 a1; A2 a2; A3 a3; 
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3 };
		writeData((char *)&data, sizeof(Data));
	}
	
	template<typename A1, typename A2, typename A3, typename A4>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4), A1 a1, A2 a2, A3 a3, A4 a4) {
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4);
			A1 a1; A2 a2; A3 a3; A4 a4;
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3, d->a4);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3, a4 };
		writeData((char *)&data, sizeof(Data));
	}
	
	template<typename A1, typename A2, typename A3, typename A4, typename A5>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
			A1 a1; A2 a2; A3 a3; A4 a4; A5 a5;
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3, d->a4, d->a5);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3, a4, a5 };
		writeData((char *)&data, sizeof(Data));
	}
	
	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
			A1 a1; A2 a2; A3 a3; A4 a4; A5 a5; A6 a6;
			static void call(char * args) {
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3, d->a4, d->a5, d->a6);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3, a4, a5, a6 };
		writeData((char *)&data, sizeof(Data));
	}
	
protected:	

	void cache(const void * src, size_t size) {
		char * mem = new char[size];
		memcpy(mem, src, size);
		cacheq.push(mem);
	}
	
	bool flushCache() {
		while (!cacheq.empty()) {
			char * data = cacheq.front();
			size_t size = *((size_t *)data);
			if (size >= rb.writeSpace()) {
				return false;
			}
			
			// send cached message:
			rb.write(data, size);
			delete[] data;
			cacheq.pop();
		}
		return true;
	}
	
	void writeData(char * data, size_t size) {
		if (size >= memsize) {
			printf("ERROR WRITING TO RINGBUFFER\n");
		} else
		if (flushCache() && rb.writeSpace() >= size) {
			//printf("scheduled message\n");
			rb.write(data, size);
		} else {
			//printf("cached message\n");
			char * cpy = new char[size];
			memcpy(cpy, data, size);
			cache(cpy, size);
		}
	}
};

/*
	Inline Implementation
*/
#pragma mark Inline Implementation

inline MsgTube :: MsgTube(int bits) 
:	now(0),
	memsize(1<<bits),
	rb(memsize)
{
	//printf("created buffer of %d size\n", memsize);
}

inline MsgTube :: ~MsgTube() {
	// TODO: empty the cache
}

inline void MsgTube :: executeUntil(al_sec until) {
	Header header;
	while (rb.readSpace() > sizeof(header)) {
		rb.peek((char *)&header, sizeof(header));
		if (header.t > until) {
			return;
		}
		char buf[header.size];
		rb.read(buf, header.size);
		(header.func)(buf);
	}
}

/*
	Copies 'data', so you can safely free it after this call
*/
inline void MsgTube :: send_data(void (*func)(al_sec t, char * data), char * data, size_t size) {
	struct Data {
		Header header;
		void (*f)(al_sec t, char * args);
		
		static void call(char * args) {
			const Data * d = (Data *)args;
			(d->f)(d->header.t, args + sizeof(Data));
		}
	};

	size_t packetsize = sizeof(Data) + size;
	char packet[packetsize];
	
	Data * d = (Data *)packet;
	d->header.size = packetsize;
	d->header.t = now;
	d->header.func = Data::call;
	d->f = func;
	memcpy(packet+sizeof(Data), data, size);
	writeData(packet, packetsize);
}
	
	
} // al::

#endif /* include guard */
