#ifndef INCLUDE_AL_GRAPHICS_MODEL_HPP
#define INCLUDE_AL_GRAPHICS_MODEL_HPP

/*
 *  A collection of functions and classes related to application mainloops
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

#include <string>
#include <vector>
#include <map>

#include "protocol/al_Graphics.hpp"
#include "graphics/al_Common.hpp"
#include "graphics/al_Light.hpp"

namespace al {
namespace gfx{

class Model {
public:

//	struct Triangle {
//		unsigned int indices[3];
//		GraphicsData::Normal normal;
//	};

	struct Group {
	public:
		Group(std::string name = "default") 
		: mName(name), mMaterial("default") {}
		
		std::string name() { return mName; }
		std::string material() { return mMaterial; }
		GraphicsData& data() { return mData; }
		
		void name(std::string n) { mName = n; }
		void material(std::string m) { mMaterial = m; }
		
	protected:
		std::string     mName;           /* name of this group */
		std::string     mMaterial;       /* index to material for group */
		GraphicsData	mData;
		//std::vector<Triangle> mTriangles;
	};
	
	Model() {}
	~Model() {}
	
	void readOBJ(const char * filename);
	
	void center() {
		std::map<std::string, Group>::iterator iter = mGroups.begin();
		while (iter != mGroups.end()) {	
			Group& g = iter->second;
			g.data().center();
			iter++;
		}
	}
	
	void draw(Graphics& gl) {
		std::map<std::string, Group>::iterator iter = mGroups.begin();
		while (iter != mGroups.end()) {	
			Group& gr = iter->second;
			// apply materials:
			mMaterials[gr.material()]();
			// render data:
			gl.draw(gr.data());
			iter++;
		}
	}
	
	
	Material& material(std::string name);
	Group& group(std::string name);
	
protected:

	void readMTL(std::string path);
	Group* addGroup(std::string name);

	std::map<std::string, Group> mGroups;
	std::map<std::string, Material> mMaterials;
	
	std::string mPath;				/* path to this model */
	std::string mMaterialLib;       /* name of the material library */
};

} // ::al::gfx
} // ::al

#endif
