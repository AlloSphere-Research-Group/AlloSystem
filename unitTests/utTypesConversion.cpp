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

	assert(intToUnit(     0) ==     0./32768);
	assert(intToUnit(     1) ==     1./32768);
	assert(intToUnit( 32767) == 32767./32768);
	assert(intToUnit(    -1) ==    -1./32768);
	assert(intToUnit(-32768) ==-32768./32768);

	assert(toString(1) == "1");
	assert(toString(1.1) == "1.1");
	assert(toString(-1.1) == "-1.1");

	assert(uintToUnit<float>(1<< 0) == 0.00);
	assert(uintToUnit<float>(1UL<<29) == 1./8);
	assert(uintToUnit<float>(1UL<<30) == 1./4);
	assert(uintToUnit<float>(1UL<<31) == 1./2);
	
	assert(uintToUnitS<float>(1UL<<31) == 0.0);
	assert(uintToUnitS<float>((1UL<<31) - (1<<30)) ==-0.5);
	assert(uintToUnitS<float>((1UL<<31) + (1<<30)) ==+0.5);

	//assert(unitToUInt2(0.0) == 0);	// not working exactly...
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


