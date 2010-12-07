#ifndef INCLUDE_AL_GRAPHICS_BACKEND_OPENGL_HPP
#define INCLUDE_AL_GRAPHICS_BACKEND_OPENGL_HPP

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

#include "allocore/graphics/al_GraphicsOpenGL.hpp"
#include "allocore/graphics/al_GraphicsBackend.hpp"

namespace al{

class GraphicsBackendOpenGL : public GraphicsBackend {
public:
	static void gl_error(const char *msg);
	static GLenum type_for_array_type(AlloTy type);

	struct SurfaceData{
		SurfaceData()
		:	depth_id(0)
		{}
		
		~SurfaceData() {}
	
		int depth_id;
	};
	
	static Graphics * makeGraphics() {
		return new Graphics(new GraphicsBackendOpenGL());
	}

	GraphicsBackendOpenGL();
	virtual ~GraphicsBackendOpenGL();
	
	// Render State
	virtual void enable(Graphics::Capability v);
	virtual void disable(Graphics::Capability v);

	virtual void enableBlending(bool enable, Graphics::BlendFunc src, Graphics::BlendFunc dst);
	virtual void enableLighting(bool enable);
	virtual void enableLight(bool enable, int idx);
	virtual void enableDepthTesting(bool enable);
	virtual void enableDepthMask(bool enable);
	virtual void enableScissor(bool enable);
	virtual void setPolygonMode(Graphics::PolygonMode mode);
	virtual void setAntialiasing(Graphics::AntiAliasMode mode);
	virtual void color(const Color &c);
	virtual void pointSize(double v);
	virtual void lineWidth(double v);
	
	// Frame
	virtual void clear(int attribMask);
	virtual void clearColor(float r, float g, float b, float a);
	
	// Coordinate Transforms
	virtual void viewport(int x, int y, int width, int height);
	virtual void setProjectionMatrix(Matrix4d &m);
	virtual void setModelviewMatrix(Matrix4d &m);
	
	// Textures
	virtual void textureCreate(Texture *tex);
	virtual void textureDestroy(Texture *tex);
	virtual void textureBind(Texture *tex, int unit);
	virtual void textureUnbind(Texture *tex, int unit);
	virtual void textureEnter(Texture *tex, int unit);
	virtual void textureLeave(Texture *tex, int unit);
	virtual void textureSubmit(Texture *tex);
	
	// surfaces
	virtual Surface * surfaceNew();

protected:
	void surfaceAttachTexture(Surface *surface, Texture *tex);
	void surfaceAttachDepthbuffer(Surface *surface);

public:	
	virtual void surfaceCreate(Surface *surface);
	virtual void surfaceDestroy(Surface *surface);
	virtual void surfaceBind(Surface *surface);
	virtual void surfaceUnbind(Surface *surface);
	virtual void surfaceEnter(Surface *surface);
	virtual void surfaceLeave(Surface *surface);
	
	
	// Buffer drawing
	virtual void draw(const GraphicsData& v);
	
};

} // ::al
	
#endif
