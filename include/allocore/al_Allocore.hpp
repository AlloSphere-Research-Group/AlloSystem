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
*/

//#include "allocore/graphics/al_Config.h"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GraphicsBackendOpenGL.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
#include "allocore/graphics/al_Light.hpp"
#include "allocore/graphics/al_Model.hpp"
#include "allocore/graphics/al_Stereographic.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/io/al_Socket.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/math/al_Complex.hpp"
#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Frustum.hpp"
#include "allocore/math/al_Functions.hpp"
#include "allocore/math/al_Generators.hpp"
#include "allocore/math/al_Interpolation.hpp"
#include "allocore/math/al_Interval.hpp"
#include "allocore/math/al_Plane.hpp"
#include "allocore/math/al_Quat.hpp"
#include "allocore/math/al_Random.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "allocore/protocol/al_Serialize.hpp"
#include "allocore/sound/al_Reverb.hpp"
#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/spatial/al_Camera.hpp"
#include "allocore/spatial/al_CoordinateFrame.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/system/al_Thread.hpp"
#include "allocore/system/al_Time.hpp"
#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Conversion.hpp"
#include "allocore/types/al_Array.hpp"
