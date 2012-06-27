#include "utAllocore.h"

int utProtocolSerialize(){

	// Serialization
	{	using namespace ser;

		// Single element tests
		{
			Serializer s;

			float    if1=1, of1=0;
			double   id1=1, od1=0;
			int8_t   ih1=1, oh1=0;
			int16_t  iH1=1, oH1=0;
			int32_t  ii1=1, oi1=0;
			int64_t  iI1=1, oI1=0;
			uint8_t  it1=1, ot1=0;
			uint16_t iT1=1, oT1=0;
			uint32_t iu1=1, ou1=0;
			uint64_t iU1=1, oU1=0;
			
			bool ib1=true, ob1=false;
			std::string istr = "string", ostr;

			s << if1 << id1 << ih1 << iH1 << ii1 << iI1 << it1 << iT1 << iu1 << iU1 << ib1 << istr;
			
			//for(int i=0; i<s.buf().size(); ++i) printf("% 4d (%c)\n", s.buf()[i], s.buf()[i]);
			
			Deserializer d(s.buf());

			d >> of1 >> od1 >> oh1 >> oH1 >> oi1 >> oI1 >> ot1 >> oT1 >> ou1 >> oU1 >> ob1 >> ostr;

			assert(of1 == if1);
			assert(od1 == id1);
			assert(oh1 == ih1);
			assert(oH1 == iH1);
			assert(oi1 == ii1);
			assert(oI1 == iI1);
			assert(ot1 == it1);
			assert(oT1 == iT1);
			assert(ou1 == iu1);
			assert(oU1 == iU1);
			assert(ob1 == ib1);
			//assert(ostr == istr);

			//printf("\n%f, %f, %d, %s\n", of1, od1, ob1, ostr.c_str());
		}

		// Multi-element tests
		{
			const int N = 3;
			#define IV {1,2,3}
			#define OV {0,0,0}
			
			float    ifn[N]=IV, ofn[N]=OV;
			double   idn[N]=IV, odn[N]=OV;
			int8_t   ihn[N]=IV, ohn[N]=OV;
			int16_t  iHn[N]=IV, oHn[N]=OV;
			int32_t  iin[N]=IV, oin[N]=OV;
			int64_t  iIn[N]=IV, oIn[N]=OV;
			uint8_t  itn[N]=IV, otn[N]=OV;
			uint16_t iTn[N]=IV, oTn[N]=OV;
			uint32_t iun[N]=IV, oun[N]=OV;
			uint64_t iUn[N]=IV, oUn[N]=OV;
			
			Serializer s;
			
			s	.add(ifn,N).add(idn,N)
				.add(ihn,N).add(iHn,N).add(iin,N).add(iIn,N)
				.add(itn,N).add(iTn,N).add(iun,N).add(iUn,N)
			;
			
			//for(int i=0; i<s.buf().size(); ++i) printf("% 4d (%c)\n", s.buf()[i], s.buf()[i]);
			
			Deserializer d(s.buf());
			d >> ofn >> odn >> ohn >> oHn >> oin >> oIn >> otn >> oTn >> oun >> oUn;
			
			#define ASSERT(a,b) for(int i=0; i<N; ++i) assert(a[i] == b[i]);
			
			ASSERT(ifn, ofn);
			ASSERT(idn, odn);
			ASSERT(ihn, ohn);
			ASSERT(iHn, oHn);
			ASSERT(iin, oin);
			ASSERT(iIn, oIn);
			ASSERT(itn, otn);
			ASSERT(iTn, oTn);
			ASSERT(iun, oun);
			ASSERT(iUn, oUn);
		}	
	}

	return 0;
}
