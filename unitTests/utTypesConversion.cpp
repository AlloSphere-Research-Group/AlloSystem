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

	// Signed 16-bit integer and unit real conversions
	for(int i=-32768; i<32768; ++i){
		float f = float(i)/32768;
		assert(unitToInt16(f) == i);
		assert(intToUnit(int16_t(i)) == f);
	}

	assert(uintToUnit<float>(1<< 0) == 0.00);
	assert(uintToUnit<float>(1UL<<29) == 1./8);
	assert(uintToUnit<float>(1UL<<30) == 1./4);
	assert(uintToUnit<float>(1UL<<31) == 1./2);
	
	assert(uintToUnitS<float>(1UL<<31) == 0.0);
	assert(uintToUnitS<float>((1UL<<31) - (1<<30)) ==-0.5);
	assert(uintToUnitS<float>((1UL<<31) + (1<<30)) ==+0.5);

	//assert(unitToUInt2(0.0) == 0);
	//printf("%lu %lu\n", unitToUInt2(1./8), 1UL<<29);
	assert(unitToUInt2(1./8) == (1UL<<29));
	assert(unitToUInt2(1./4) == (1UL<<30));
	assert(unitToUInt2(1./2) == (1UL<<31));

	assert(unitToUInt8(0) == 0);
	assert(unitToUInt8(1./8) ==  32);
	assert(unitToUInt8(1./4) ==  64);
	assert(unitToUInt8(1./2) == 128);

	return 0;
}


