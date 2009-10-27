/*
 *  allocpp.h
 *  C++ helper functions for allo.h
 *
 *  AlloSphere, Media Arts & Technology, UCSB
 *
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
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#ifndef INCLUDE_ALLO_TYPES_CPP_H
#define INCLUDE_ALLO_TYPES_CPP_H 1

#include "al_types.h"

namespace allo {

/*
	Partial specialization function to get type
	This demonstrates the principle by which a runtime type can be understood by templates
*/
template<typename T> AlloTy getType() { return 0; }
template<> AlloTy getType<uint8_t>() { return AlloUInt8Ty; }
template<> AlloTy getType<uint16_t>() { return AlloUInt16Ty; }
template<> AlloTy getType<uint32_t>() { return AlloUInt32Ty; }
template<> AlloTy getType<uint64_t>() { return AlloUInt64Ty; }
template<> AlloTy getType<int8_t>() { return AlloSInt8Ty; }
template<> AlloTy getType<int16_t>() { return AlloSInt16Ty; }
template<> AlloTy getType<int32_t>() { return AlloSInt32Ty; }
template<> AlloTy getType<int64_t>() { return AlloSInt64Ty; }
template<> AlloTy getType<float>() { return AlloFloat32Ty; }
template<> AlloTy getType<double>() { return AlloFloat64Ty; }
template<> AlloTy getType<AlloLattice>() { return AlloLatticeTy; }
// TODO: #define for platform ptrsize
template<> AlloTy getType<void *>() { return AlloPointer32Ty; }
//template<> AlloTy getType<void *>() { return AlloPointer32Ty; }

/*
	E.g., verify a type:
*/
template<typename T> bool checkType(AlloTy ty) { return getType<T>() && ty == getType<T>(); }

/*
	Derived type
		N.B. methods and static members only... no additional instance member data!
*/
class Lattice : public AlloLattice {
protected:
	
public:
	
	// an example method:
	template<typename T> bool checkType() { return allo::checkType<T>(header.type); }
};

} // ::allo::

#endif /* INCLUDE_ALLO_TYPES_CPP_H */
