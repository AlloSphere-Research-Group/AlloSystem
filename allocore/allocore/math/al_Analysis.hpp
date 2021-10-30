#ifndef INCLUDE_AL_MATH_ANALYSIS_HPP
#define INCLUDE_AL_MATH_ANALYSIS_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
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
	Math analysis utilities

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <limits>
#include "allocore/math/al_Functions.hpp"

namespace al {

/// A way to analyse data acquired gradually:
///
/// @ingroup allocore
template<typename T=double>
class MinMeanMax {
public:
	MinMeanMax() { clear(); }

	void clear() {
		minimum = std::numeric_limits<T>::infinity();
		maximum = -std::numeric_limits<T>::infinity();
		sum = T(0);
		count = 0;
	}

	// add another analysis point:
	void operator()(T val) {
		minimum = al::min(val, minimum);
		maximum = al::max(val, maximum);
		sum += val;
		count++;
	}

	// read analyses:
	T min() const { return minimum; }
	T max() const { return maximum; }
	T mean() const { return sum/count; }

protected:
	T minimum, maximum, sum;
	unsigned count;
};

} // al::
#endif
