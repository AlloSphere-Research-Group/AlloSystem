#ifndef INCLUDE_AL_GRAPHICS_ASSET_HPP
#define INCLUDE_AL_GRAPHICS_ASSET_HPP

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
	Asset manages the loading and parsing of 3D asset files (models and scenes)

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Pablo Colapinto, 2010, wolftype@gmail.com
*/

/*!
	Asset manages the loading and parsing of 3D asset files
	Asset wraps the AssImp library.
	
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
		
		// standard OpenGL properties:
		int two_sided, wireframe;
		Color diffuse, ambient, specular, emissive;
		float shininess;
		
		// other properites:
		int shading_model, blend_func;
		float shininess_strength, opacity, reflectivity, refracti, bump_scaling;
		Color transparent, reflective;
		TextureProperty diffusemap, ambientmap, specularmap, opacitymap, emissivemap, shininessmap, lightmap, normalmap, heightmap, displacementmap, reflectionmap; 
		std::string background;	
		
	};
	
	static Scene * import(const std::string& path, ImportPreset preset = MAX_QUALITY);
	~Scene();
	
	/// return number of meshes in scene
	unsigned int meshes() const;
	/// read a mesh from the Scene:
	void mesh(unsigned int i, Mesh& mesh) const;
	/// alternative read a mesh from the Scene:
	void meshAlt(unsigned int i, Mesh& mesh) const;	
	// read all meshes:
	void meshAll(Mesh& dst) const { for (unsigned i=0; i<meshes(); i++) mesh(i, dst); }
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
	
	/// toggle verbose mode
	static void verbose(bool b);
	
protected:

	std::vector<Material> mMaterials;

	class Impl;
	Impl * mImpl;
	
	Scene(Impl * impl);
};

} // al::

#endif /* include guard */
