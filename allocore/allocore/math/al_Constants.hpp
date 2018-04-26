#ifndef INCLUDE_AL_MATH_CONSTANTS_HPP
#define INCLUDE_AL_MATH_CONSTANTS_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	This includes various macro constants that may not already be defined in the
	standard C/C++ math headers.

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

namespace al {

#ifndef M_E
#define M_E			2.71828182845904523536028747135266250
#endif
#ifndef M_LOG2E
#define M_LOG2E		1.44269504088896340735992468100189214
#endif
#ifndef M_LOG10E
#define M_LOG10E	0.434294481903251827651128918916605082
#endif
#ifndef M_LN2
#define M_LN2		0.693147180559945309417232121458176568
#endif
#ifndef M_LN10
#define M_LN10		2.30258509299404568401799145468436421
#endif
#ifndef M_PI
#define M_PI		3.14159265358979323846264338327950288
#endif
#ifndef M_PI_2
#define M_PI_2		1.57079632679489661923132169163975144
#endif
#ifndef M_PI_4
#define M_PI_4		0.785398163397448309615660845819875721
#endif
#ifndef M_1_PI
#define M_1_PI		0.318309886183790671537767526745028724
#endif
#ifndef M_2_PI
#define M_2_PI		0.636619772367581343075535053490057448
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI	1.12837916709551257389615890312154517
#endif
#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880168872420969808
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2	0.707106781186547524400844362104849039
#endif
#ifndef M_DEG2RAD
#define M_DEG2RAD	0.017453292519943							// pi/180
#endif
#ifndef M_RAD2DEG
#define M_RAD2DEG	57.295779513082								// 180/pi
#endif

// Some other useful constants
#ifndef M_2PI
#define M_2PI		6.283185307179586231941716828464095101		// 2pi
#endif
#ifndef M_4PI
#define M_4PI		12.566370614359172463937643765552465425		// 4pi
#endif
#ifndef M_1_2PI
#define M_1_2PI		0.159154943091895345554011992339482617		// 1/(2pi)
#endif
#ifndef M_3PI_2
#define M_3PI_2		4.712388980384689673996945202816277742		// 3pi/2
#endif
#ifndef M_3PI_4
#define M_3PI_4		2.356194490192343282632028017564707056		// 3pi/4
#endif
#ifndef M_LN001
#define M_LN001		-6.90775527898								// ln(0.001)
#endif
#ifndef M_SQRT_1_3
#define	M_SQRT_1_3	0.577350269189626							// sqrt(1./3);
#endif

} // ::al::

#endif
