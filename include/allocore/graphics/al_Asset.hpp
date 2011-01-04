#ifndef INCLUDE_AL_GRAPHICS_ASSET_HPP
#define INCLUDE_AL_GRAPHICS_ASSET_HPP

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

/*!
	Asset manages the loading and parsing of 3D asset files
	It wraps the AssImp library.
	
	A scene contains
		- array of meshes
		- array of materials
		- array of textures
		- also: array of animations, array of lights, array of cameras
		- tree of nodes, starting from a root node
		
	A mesh contains
		- name
		- material (index into scene)
		- array of vertices
		- array of normals
		- array of colors
		- array of texcoords
		- arrays of tangents, bitangents
		- bones
	
	A material contains
		- array of properties
		- helper methods for retrieving textures
	
	A node contains 
		- name
		- a 4x4 matrix transform relative to its parent
		- a list of children
		- a number of meshes (indexing scene arrays)
	
	
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/types/al_Color.hpp"

#include <string>

namespace al{

class Scene {
public:
	enum ImportPreset {
		FAST,
		QUALITY,
		MAX_QUALITY
	};
	
	struct Node {
		class Impl;
		
		Node();
		~Node();
	
		std::string name() const;
		Impl * mImpl;
	};
	
	struct Material {
		struct TextureProperty {
			bool useTexture;
			std::string texture;
			
			TextureProperty() : useTexture(false) {}
		};
		
		Material();
		
		std::string name;
		
		int shading_model, two_sided, wireframe, blend_func;
		float shininess, shininess_strength, opacity, reflectivity, refracti, bump_scaling;
		Color diffuse, ambient, specular, emissive, transparent, reflective;
		TextureProperty diffusemap, ambientmap, specularmap, opacitymap, emissivemap, shininessmap, lightmap, normalmap, heightmap, displacementmap, reflectionmap; 
		std::string background;	
		
	};
	
	static Scene * import(std::string path, ImportPreset preset = MAX_QUALITY);
	~Scene();
	
	/// return number of meshes in scene
	unsigned int meshes() const;
	/// read a mesh from the Scene:
	void mesh(unsigned int i, Mesh& mesh) const;
	/// get the material index for a given mesh:
	unsigned int meshMaterial(unsigned int i) const;
	/// get the name of a given mesh
	std::string meshName(unsigned int i) const;
	
	/// return number of materials in scene
	unsigned int materials() const;
	/// read a material from the scene
	const Material& material(unsigned int i) const;
	
	/// return number of materials in scene
	unsigned int textures() const;
	
	/// return number of nodes in scene
	unsigned int nodes() const;
	/// read a node in the scene:
	Node& node(unsigned int i) const;
	
	
	/// get scene extents
	void getBounds(Vec3f& min, Vec3f& max) const;
	
	/// print out information about the Scene
	void dump() const;
	
protected:

	std::vector<Material> mMaterials;

	class Impl;
	Impl * mImpl;
	
	Scene(Impl * impl);
};

} // al::

#endif /* include guard */
