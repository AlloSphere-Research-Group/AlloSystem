#include <stdio.h>
#include "protocol/al_Serialize.h"

uint32_t serDecode(const char * b, void * data){
	struct SerHeader h = serGetHeader(b);
	uint32_t SOH = serHeaderSize();
	uint32_t r = SOH;
	switch(h.type){
		case SER_FLOAT32:
		case SER_INT32:
		case SER_UINT32: r += serCopy4(data, b+SOH, h.num); break;
		case SER_FLOAT64:
		case SER_INT64:
		case SER_UINT64: r += serCopy8(data, b+SOH, h.num); break;
		case SER_INT8:
		case SER_UINT8:  r += serCopy1(data, b+SOH, h.num); break;
		case SER_INT16:
		case SER_UINT16: r += serCopy2(data, b+SOH, h.num); break;
		default:;
	}
	return r;
}

uint32_t serElementsSize(const SerHeader * h){
	return serTypeSize(h->type) * h->num;
}

SerHeader serGetHeader(const char * buf){
	SerHeader h;
	h.type = buf[0];
	serCopy4(&h.num, buf+1, 1);
	return h;
}

const char * serStringifyHeader(const SerHeader * h){
	static char s[32];
	sprintf(s, "%s %d", serStringifyType(h->type), h->num);
	return s;
}

const char * serStringifyType(uint8_t t){
	switch(t){
		case SER_FLOAT32:	return "float32";
		case SER_FLOAT64:	return "float64";
		case SER_INT8:		return "int8";
		case SER_INT16:		return "int16";
		case SER_INT32:		return "int32";
		case SER_INT64:		return "int64";
		case SER_UINT8:		return "uint8";
		case SER_UINT16:	return "uint16";
		case SER_UINT32:	return "uint32";
		case SER_UINT64:	return "uint64";
		case SER_SUB:		return "sub";
		default:			return "unknown";
	}
}

int serTypeSize(uint8_t t){
	switch(t){
		case SER_FLOAT32:	return 4;
		case SER_FLOAT64:	return 8;
		case SER_INT8:		return 1;
		case SER_INT16:		return 2;
		case SER_INT32:		return 4;
		case SER_INT64:		return 8;
		case SER_UINT8:		return 1;
		case SER_UINT16:	return 2;
		case SER_UINT32:	return 4;
		case SER_UINT64:	return 8;
		case SER_SUB:		return serHeaderSize();
		default:			return 0;
	}
}
