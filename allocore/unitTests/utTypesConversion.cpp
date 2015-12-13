#include "utAllocore.h"

#include "catch.hpp"

TEST_CASE( "TypesConversion", "[types]" ) {

	REQUIRE(base10To36( 0) == '0');
	REQUIRE(base10To36( 9) == '9');
	REQUIRE(base10To36(10) == 'a');
	REQUIRE(base10To36(35) == 'z');

	REQUIRE(base36To10('0') ==  0);
	REQUIRE(base36To10('9') ==  9);
	REQUIRE(base36To10('a') == 10);
	REQUIRE(base36To10('z') == 35);

	REQUIRE(bitsToUInt("00") == 0);
	REQUIRE(bitsToUInt("01") == 1);
	REQUIRE(bitsToUInt("10") == 2);
	REQUIRE(bitsToUInt("11") == 3);

	{
		//char big[] = { 64, 73, 15, -37 };
		//float bigf = *(float *)big;
		//swapBytes(bigf);
		//REQUIRE(al::aeq( bigf, (float)M_PI ));

		uint16_t v2 = 0x0123;
		swapBytes(v2);
		REQUIRE(v2 == 0x2301);

		uint32_t v4 = 0x01234567;
		swapBytes(v4);
		REQUIRE(v4 == 0x67452301);

		uint64_t v8 = 0x0123456789abcdefULL;
		swapBytes(v8);
		REQUIRE(v8 == 0xefcdab8967452301ULL);

		union{
			int32_t i;
			float f;
		} u;

		u.i = 0x01234567;
		swapBytes(u.f);
		REQUIRE(u.i == 0x67452301);
	}

	REQUIRE(toString(1) == "1");
	REQUIRE(toString(1.1) == "1.1");
	REQUIRE(toString(-1.1) == "-1.1");

	// Signed 16-bit integer and unit real conversions
	for(int i=-32768; i<32768; ++i){
		float f = float(i)/32768;
		REQUIRE(unitToInt16(f) == i);
		REQUIRE(intToUnit(int16_t(i)) == f);
	}

	REQUIRE(uintToUnit<float>(1<< 0) == 0.00);
	REQUIRE(uintToUnit<float>(1UL<<29) == 1./8);
	REQUIRE(uintToUnit<float>(1UL<<30) == 1./4);
	REQUIRE(uintToUnit<float>(1UL<<31) == 1./2);

	REQUIRE(uintToUnitS<float>(1UL<<31) == 0.0);
	REQUIRE(uintToUnitS<float>((1UL<<31) - (1<<30)) ==-0.5);
	REQUIRE(uintToUnitS<float>((1UL<<31) + (1<<30)) ==+0.5);

	//REQUIRE(unitToUInt2(0.0) == 0);
	//printf("%lu %lu\n", unitToUInt2(1./8), 1UL<<29);
	REQUIRE(unitToUInt2(1./8) == (1UL<<29));
	REQUIRE(unitToUInt2(1./4) == (1UL<<30));
	REQUIRE(unitToUInt2(1./2) == (1UL<<31));

	REQUIRE(unitToUInt8(0) == 0);
	REQUIRE(unitToUInt8(1./8) ==  32);
	REQUIRE(unitToUInt8(1./4) ==  64);
	REQUIRE(unitToUInt8(1./2) == 128);
}


