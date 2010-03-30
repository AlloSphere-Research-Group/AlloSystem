#include <stdio.h>
#include "protocol/al_Serialize.hpp"


namespace al{

Serializer& Serializer::operator<< (const char * v){
	return add(v, strlen(v)+1);
}

Serializer& Serializer::operator<< (const std::string& v){
	return add(v.c_str(), v.size()+1);
}

const std::vector<char>& Serializer::buf() const { return mBuf; }
	


Deserializer::Deserializer(const std::vector<char>& b): mStart(0), mBuf(b){}

Deserializer::Deserializer(const char * b, uint32_t n)
:	mStart(0)
{	mBuf.insert(mBuf.begin(), b, b+n); }

Deserializer& Deserializer::operator>> (char * v){
	uint32_t n = serDecode(bufDec(), v);
	mStart += n;
	return *this;
}

Deserializer& Deserializer::operator>> (std::string& v){
	SerHeader h = serGetHeader(bufDec());
	uint32_t need = serElementsSize(&h);
	if(v.size() < need) v.resize(need);
	return *this >> &v[0];
}

const std::vector<char>& Deserializer::buf() const { return mBuf; }

char * Deserializer::bufDec(){ return &mBuf[mStart]; }

} // al::
