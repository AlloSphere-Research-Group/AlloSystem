#ifndef INC_AL_MSG_TUBE_HPP
#define INC_AL_MSG_TUBE_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Class for passing functors between a pair of threads

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/system/al_Config.h" // al_sec
#include "allocore/types/al_SingleRWRingBuffer.hpp"
#include <queue>

#define AL_MSGTUBE_DEFAULT_SIZE_BITS (14) // 16384 bytes

namespace al {

///
/// \brief The MsgTube class
/// A C++ class for deferred function calls
/// Using templates for type-checked functions of variable arguments
/// Adding a cache queue to seamlessly handle buffer overflow
///
/// @ingroup allocore

class MsgTube {
public:

	/// Header for messages in the queue
	struct Header {
		size_t size;
		al_sec t;
		void (*func)(char * args);
	};


	/// Timestamp applied to sent messages (should increase monotonically)
	al_sec now;

	size_t memsize;
	SingleRWRingBuffer rb;

	/// Cache of messages, when the ringbuffer is full
	// TODO: set a cache limit? track when flushing fails for a prolonged period?
	std::queue<char *> cacheq;


	MsgTube(int bits = AL_MSGTUBE_DEFAULT_SIZE_BITS);
	~MsgTube();


	void executeUntil(al_sec until);

	// Copies 'data', so you can safely free it after this call
	void send_data(void (*func)(al_sec t, char * data), char * data, size_t size);

	void send(void (*f)(al_sec t)){
		struct Data {
			Header header;
			void (*f)(al_sec t);
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f };
		writeData((char *)&data, sizeof(Data));
	}

	template<typename A1>
	void send(void (*f)(al_sec t, A1 a1), A1 a1){
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1);
			A1 a1;
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1 };
		writeData((char *)&data, sizeof(Data));
	}

	template<typename A1, typename A2>
	void send(void (*f)(al_sec t, A1 a1, A2 a2), A1 a1, A2 a2){
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2);
			A1 a1; A2 a2;
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2 };
		writeData((char *)&data, sizeof(Data));
	}

	template<typename A1, typename A2, typename A3>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3), A1 a1, A2 a2, A3 a3){
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3);
			A1 a1; A2 a2; A3 a3;
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3 };
		writeData((char *)&data, sizeof(Data));
	}

	template<typename A1, typename A2, typename A3, typename A4>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4), A1 a1, A2 a2, A3 a3, A4 a4){
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4);
			A1 a1; A2 a2; A3 a3; A4 a4;
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3, d->a4);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3, a4 };
		writeData((char *)&data, sizeof(Data));
	}

	template<typename A1, typename A2, typename A3, typename A4, typename A5>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5){
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
			A1 a1; A2 a2; A3 a3; A4 a4; A5 a5;
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3, d->a4, d->a5);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3, a4, a5 };
		writeData((char *)&data, sizeof(Data));
	}

	template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void send(void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6){
		struct Data {
			Header header;
			void (*f)(al_sec t, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
			A1 a1; A2 a2; A3 a3; A4 a4; A5 a5; A6 a6;
			static void call(char * args){
				const Data * d = (Data *)args;
				(d->f)(d->header.t, d->a1, d->a2, d->a3, d->a4, d->a5, d->a6);
			}
		};
		Data data = { { sizeof(Data), now, Data::call }, f, a1, a2, a3, a4, a5, a6 };
		writeData((char *)&data, sizeof(Data));
	}

protected:
	void cache(const void * src, size_t size);
	bool flushCache();
	void writeData(char * data, size_t size);
};

} // al::
#endif
