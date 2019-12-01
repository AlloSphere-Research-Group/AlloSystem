#include "utAllocore.h"

int utTypesConversion(){

	assert(base10To36( 0) == '0');
	assert(base10To36( 9) == '9');
	assert(base10To36(10) == 'a');
	assert(base10To36(35) == 'z');

	assert(base36To10('0') ==  0);
	assert(base36To10('9') ==  9);
	assert(base36To10('a') == 10);
	assert(base36To10('z') == 35);

	assert(bitsToUInt("00") == 0);
	assert(bitsToUInt("01") == 1);
	assert(bitsToUInt("10") == 2);
	assert(bitsToUInt("11") == 3);

	{
		//char big[] = { 64, 73, 15, -37 };
		//float bigf = *(float *)big;
		//swapBytes(bigf);
		//assert(al::aeq( bigf, (float)M_PI ));

		uint16_t v2 = 0x0123;
		swapBytes(v2);
		assert(v2 == 0x2301);

		uint32_t v4 = 0x01234567;
		swapBytes(v4);
		assert(v4 == 0x67452301);

		uint64_t v8 = 0x0123456789abcdefULL;
		swapBytes(v8);
		assert(v8 == 0xefcdab8967452301ULL);

		union{
			int32_t i;
			float f;
		} u;

		u.i = 0x01234567;
		swapBytes(u.f);
		assert(u.i == 0x67452301);
	}

	assert(toString(1) == "1");
	assert(toString(1.1) == "1.1");
	assert(toString(-1.1) == "-1.1");

	return 0;
}


