#ifndef INCLUDE_AL_GRAPHICS_BACKEND_HPP
#define INCLUDE_AL_GRAPHICS_BACKEND_HPP

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


#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Color.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Surface.hpp"

namespace al{

class GraphicsBackend {
public:

	GraphicsBackend() {}
	virtual ~GraphicsBackend() {}
	
	// Render State
	virtual void enable(Graphics::Capability v) = 0;
	virtual void disable(Graphics::Capability v) = 0;
	
	virtual void enableBlending(bool enable, Graphics::BlendFunc src, Graphics::BlendFunc dst) = 0;
	virtual void enableLighting(bool enable) = 0;
	virtual void enableLight(bool enable, int idx) = 0;
	virtual void enableDepthTesting(bool enable) = 0;
	virtual void enableDepthMask(bool enable) = 0;
	virtual void enableScissor(bool enable) = 0;
	virtual void setPolygonMode(Graphics::PolygonMode mode) = 0;
	virtual void setAntialiasing(Graphics::AntiAliasMode mode) = 0;
	virtual void color(const Color &c) = 0;
	virtual void pointSize(double v) = 0;
	virtual void lineWidth(double v) = 0;
	
	// Frame
	virtual void clear(int attribMask) = 0;
	virtual void clearColor(float r, float g, float b, float a) = 0;
	
	// Coordinate Transforms
	virtual void viewport(int x, int y, int width, int height) = 0;
	virtual void setProjectionMatrix(Matrix4d &m) = 0;
	virtual void setModelviewMatrix(Matrix4d &m) = 0;
	
	// Textures
	Texture * textureNew() { return new Texture(this); }
	virtual void textureCreate(Texture *tex) = 0;
	virtual void textureDestroy(Texture *tex) = 0;
	virtual void textureBind(Texture *tex, int unit) = 0;
	virtual void textureUnbind(Texture *tex, int unit) = 0;
	virtual void textureEnter(Texture *tex, int unit) = 0;
	virtual void textureLeave(Texture *tex, int unit) = 0;
	virtual void textureSubmit(Texture *tex) = 0;
	
	// surfaces
	virtual Surface * surfaceNew() = 0;
	virtual void surfaceCreate(Surface *surface) = 0;
	virtual void surfaceDestroy(Surface *surface) = 0;
	virtual void surfaceBind(Surface *surface) = 0;
	virtual void surfaceUnbind(Surface *surface) = 0;
	virtual void surfaceEnter(Surface *surface) = 0;
	virtual void surfaceLeave(Surface *surface) = 0;
	
	// Buffer drawing
	virtual void draw(const GraphicsData& v) = 0;
};

} // ::al
	
#endif
