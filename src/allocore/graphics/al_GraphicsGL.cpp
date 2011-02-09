#include "allocore/graphics/al_GraphicsOpenGL.hpp"
#include <stdio.h>

using namespace al;

void GraphicsGL::gl_error(const char *msg) {
	GLenum err = glGetError();

	#define POST "The offending command is ignored and has no other side effect than to set the error flag."
	switch(err) {
		case GL_INVALID_ENUM:	printf("%s:\n %s\n", msg, "An unacceptable value is specified for an enumerated argument. "POST); break;
		case GL_INVALID_VALUE:	printf("%s:\n %s\n", msg, "A numeric argument is out of range. "POST); break;
		case GL_INVALID_OPERATION:printf("%s:\n %s\n", msg, "The specified operation is not allowed in the current state. "POST); break;
		case GL_STACK_OVERFLOW:	printf("%s:\n %s\n", msg, "This command would cause a stack overflow. "POST); break;
		case GL_STACK_UNDERFLOW:printf("%s:\n %s\n", msg, "This command would cause a stack underflow. "POST); break;
		case GL_OUT_OF_MEMORY:	printf("%s:\n %s\n", msg, "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded."); break;
		case GL_TABLE_TOO_LARGE:printf("%s:\n %s\n", msg, "The specified table exceeds the implementation's maximum supported table size. "POST); break;
		case GL_NO_ERROR: break;
		default: break;
	}
	#undef POST
}

#define CS(TYPE) case Graphics::TYPE: return GL_##TYPE;

GLenum GraphicsGL :: gl_antialias_mode(Graphics::AntiAliasMode v) {
	switch(v){
		CS(NICEST) CS(FASTEST)
		default: return GL_DONT_CARE;
	}
}

GLenum GraphicsGL :: gl_matrix_mode(Graphics::MatrixMode v) {
	switch(v){
		CS(MODELVIEW) CS(PROJECTION)
		default: return GL_MODELVIEW;
	}
}

GLenum GraphicsGL :: gl_blend_func(Graphics::BlendFunc v) {
	switch(v){	
		CS(SRC_COLOR) 
		CS(ONE_MINUS_SRC_COLOR)
		CS(DST_COLOR) 
		CS(ONE_MINUS_DST_COLOR)
		CS(SRC_ALPHA) 
		CS(ONE_MINUS_SRC_ALPHA)
		CS(DST_ALPHA)
		CS(ONE_MINUS_DST_ALPHA)	
		CS(ZERO) 
		CS(ONE) 
		CS(SRC_ALPHA_SATURATE)		
		default: return GL_SRC_ALPHA;
	}
}

GLenum GraphicsGL :: gl_polygon_mode(Graphics::PolygonMode v) {
	switch(v){
		CS(POINT) CS(LINE)
		default: return GL_FILL;
	}
}

GLenum GraphicsGL :: gl_primitive(Graphics::Primitive v){
	switch(v){
		CS(POINTS) CS(LINES) CS(LINE_STRIP) CS(LINE_LOOP) CS(TRIANGLES)
		CS(TRIANGLE_STRIP) CS(TRIANGLE_FAN) CS(QUADS) CS(QUAD_STRIP) CS(POLYGON)
		default: return GL_POINTS;
	}
}

GLenum GraphicsGL :: gl_capability(Graphics::Capability v){
	switch(v){
		CS(BLEND) CS(COLOR_MATERIAL) CS(DEPTH_TEST) CS(LIGHTING)
		CS(SCISSOR_TEST)
		default: return 0;
	}
}

#undef CS


GraphicsGL :: GraphicsGL() : Graphics() {}
GraphicsGL :: ~GraphicsGL() {}
	
	// Rendering State
void GraphicsGL :: p_blending(bool enable, BlendFunc src, BlendFunc dst) {
	if(enable) {
		glEnable(GL_BLEND);
		glBlendFunc(
			gl_blend_func(src),
			gl_blend_func(dst)
		);
	}
	else {
		glDisable(GL_BLEND);
	}
}
void GraphicsGL :: p_enable(Capability v) {
	if (v == DEPTH_MASK) {
		glDepthMask(GL_TRUE);
	} else {
		glEnable(gl_capability(v));
	}
}
void GraphicsGL :: p_disable(Capability v) {
	if (v == DEPTH_MASK) {
		glDepthMask(GL_FALSE);
	} else {
		glDisable(gl_capability(v));
	}
}

void GraphicsGL::enableLight(bool enable, int idx) {
	if(enable) {
		glEnable(GL_LIGHT0+idx);
	}
	else {
		glDisable(GL_LIGHT0+idx);
	}
}


void GraphicsGL::p_clear(int attribMask) {
	int bits = 
		(attribMask & Graphics::COLOR_BUFFER_BIT ? GL_COLOR_BUFFER_BIT : 0) |
		(attribMask & Graphics::DEPTH_BUFFER_BIT ? GL_DEPTH_BUFFER_BIT : 0) |
		(attribMask & Graphics::ENABLE_BIT ? GL_ENABLE_BIT : 0) |
		(attribMask & Graphics::VIEWPORT_BIT ? GL_VIEWPORT_BIT : 0);
	glClear(bits);
}

void GraphicsGL::p_clearColor(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
}
	
	// Coordinate Transforms
void GraphicsGL :: p_viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, width, height);
}

void GraphicsGL :: p_matrixMode(MatrixMode mode) {
	glMatrixMode(gl_matrix_mode(mode));
}	

void GraphicsGL :: p_pushMatrix() { glPushMatrix(); }
void GraphicsGL :: p_popMatrix() { glPopMatrix(); }
void GraphicsGL :: p_loadIdentity() { glLoadIdentity(); }
void GraphicsGL :: p_loadMatrix(const Matrix4d &m) { glLoadMatrixd(m.elems); }
void GraphicsGL :: p_multMatrix(const Matrix4d &m) { glMultMatrixd(m.elems); }
void GraphicsGL :: p_translate(double x, double y, double z) { glTranslated(x, y, z); }
void GraphicsGL :: p_rotate(double angle, double x, double y, double z) { glRotated(angle, x, y, z); }
void GraphicsGL :: p_scale(double x, double y, double z) { glScaled(x, y, z); }

	// Other state
void GraphicsGL :: p_antialiasing(AntiAliasMode mode) {
	GLenum m = gl_antialias_mode(mode);
	glEnable(GL_POINT_SMOOTH_HINT);
	glEnable(GL_LINE_SMOOTH_HINT);
	glEnable(GL_POLYGON_SMOOTH_HINT);
	glHint(GL_POINT_SMOOTH_HINT, m);
	glHint(GL_LINE_SMOOTH_HINT, m);
	glHint(GL_POLYGON_SMOOTH_HINT, m);
	if (GL_FASTEST != m) {
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	} else {
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
}

void GraphicsGL :: p_lineWidth(double v) { glLineWidth(v); }
void GraphicsGL :: p_pointSize(double v) { glPointSize(v); }


void GraphicsGL :: p_currentColor(double r, double g, double b, double a) {
	glColor4f(r, g, b, a);
}


void GraphicsGL :: setPolygonMode(Graphics::PolygonMode mode) {
	glPolygonMode(GL_FRONT_AND_BACK, gl_polygon_mode(mode));
}



// Buffer drawing
void GraphicsGL :: p_draw(const Mesh& v) {

	int Nv = v.vertices().size();
	if(0 == Nv) return;
	
	int Nc = v.colors().size();
	int Nn = v.normals().size();
	int Nt2= v.texCoord2s().size();
	int Nt3= v.texCoord3s().size();
	int Ni = v.indices().size();

	// Enable arrays and set pointers...
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &v.vertices()[0]);

	if(Nn >= Nv){
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &v.normals()[0]);
	}
	
	if(Nc >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &v.colors()[0]);			
	}
	else if(0 == Nc){
		// no; just use whatever the last glColor() call used!
		//glColor4f(1, 1, 1, 1);
	}
	else{
		glColor4f(v.colors()[0][0], v.colors()[0][1], v.colors()[0][2], v.colors()[0][3]);
	}
	
	if(Nt2 || Nt3){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(Nt2 >= Nv) glTexCoordPointer(2, GL_FLOAT, 0, &v.texCoord2s()[0]);
		if(Nt3 >= Nv) glTexCoordPointer(3, GL_FLOAT, 0, &v.texCoord3s()[0]);
	}
	
	
	if(Ni){
		//unsigned vs=0, ve=Nv;	// range of vertex indices to prefetch
								// NOTE:	if this range exceeds the number of vertices,
								//			expect a segmentation fault...
		unsigned is=0, ie=Ni;	// range of indices to draw

//		glDrawRangeElements(v.primitive(), vs, ve, ie-is, GL_UNSIGNED_INT, &v.indices()[is]);
		glDrawElements(
			gl_primitive((Graphics::Primitive)v.primitive()), 
			ie-is, 
			GL_UNSIGNED_INT, 
			&v.indices()[is]
		);
	}
	else{
		glDrawArrays(
			gl_primitive((Graphics::Primitive)v.primitive()), 
			0, 
			v.vertices().size()
		);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	if(Nn) glDisableClientState(GL_NORMAL_ARRAY);
	if(Nc) glDisableClientState(GL_COLOR_ARRAY);
	if(Nt2 || Nt3) glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}




GLenum GraphicsGL :: type_for_array_type(AlloTy type) {
	switch(type) {
		case AlloFloat32Ty: return GL_FLOAT;
		case AlloFloat64Ty: return GL_DOUBLE;

		case AlloSInt8Ty:	return GL_BYTE;
		case AlloSInt16Ty:	return GL_SHORT;
		case AlloSInt32Ty:	return GL_INT;
		
		case AlloUInt8Ty:	return GL_UNSIGNED_BYTE;
		case AlloUInt16Ty:	return GL_UNSIGNED_SHORT;
		case AlloUInt32Ty:	return GL_UNSIGNED_INT;
	}
	
	return GL_BYTE;
}

GLenum GraphicsGL :: target_from_texture_target(Texture::Target target) {
	switch(target) {
		case Texture::TEXTURE_1D:	return GL_TEXTURE_1D;
		case Texture::TEXTURE_2D:	return GL_TEXTURE_2D;
		case Texture::TEXTURE_RECT:	return GL_TEXTURE_RECTANGLE_ARB;
		case Texture::TEXTURE_3D:	return GL_TEXTURE_3D;
	}
	
	return GL_TEXTURE_2D;
}

Texture::Target check_target(Texture::Target target) {
	switch(target) {
		case Texture::TEXTURE_RECT:
			/*if(! Glob::extensions().haveRectTexture()) {
				fprintf(stderr, "Rectangular textures not supported on hard, reverting to 2D textures\n");
				return GL_TEXTURE_2D;
			}
			else return mTarget;
			*/
			return Texture::TEXTURE_RECT;

		case Texture::TEXTURE_3D:
			/*
			if(! Glob::extensions().have3DTexture()) {
				fprintf(stderr, "3D textures not supported on hard, reverting to 2D textures\n");
				return GL_TEXTURE_2D;
			}
			else return mTarget;
			*/
			return Texture::TEXTURE_3D;
		
		case Texture::TEXTURE_1D:
		case Texture::TEXTURE_2D:
		default:
			return target;
	}
}


void clamp_texture_dimensions(Texture::Target target, int &w, int &h, int &d) {
	/*
	switch(mTarget) {
		case GL_TEXTURE_RECTANGLE_ARB: {
			int sz = Glob::extensions().maxRectTextureSize();
			mWidth = (mWidth > sz) ? sz : mWidth;
			mHeight = (mHeight > sz) ? sz : mHeight;
		} break;
		
		case GL_TEXTURE_3D: {
			int sz = Glob::extensions().max3DTextureSize();
			mWidth = (mWidth > sz) ? sz : mWidth;
			mHeight = (mHeight > sz) ? sz : mHeight;
			mDepth = (mDepth > sz) ? sz : mDepth;
		} break;
		
		default: {
			int sz = Glob::extensions().maxTextureSize();
			mWidth = (mWidth > sz) ? sz : mWidth;
			mHeight = (mHeight > sz) ? sz : mHeight;
			mDepth = (mDepth > sz) ? sz : mDepth;
		} break;
	}
	*/
}

GLenum wrap_from_texture_wrap(Texture::Target target, Texture::Wrap wrap) {
	if(target == Texture::TEXTURE_RECT) {
		switch(wrap) {
			case Texture::CLAMP_TO_EDGE:	return GL_CLAMP_TO_EDGE;
			case Texture::CLAMP_TO_BORDER:	return GL_CLAMP_TO_BORDER;
			case Texture::CLAMP:			return GL_CLAMP;
			//case Texture::REPEAT:
			default:
				return GL_CLAMP_TO_BORDER;
		}
	}
	else {
		switch(wrap) {
			case Texture::CLAMP_TO_EDGE:	return GL_CLAMP_TO_EDGE;
			case Texture::CLAMP_TO_BORDER:	return GL_CLAMP_TO_BORDER;
			case Texture::CLAMP:			return GL_CLAMP;
			//case Texture::REPEAT:
			default:;
		}
	}
	
	return GL_CLAMP_TO_BORDER;
}

GLenum filter_from_texture_filter(Texture::Filter filter) {
	switch(filter) {
		case Texture::NEAREST:					return GL_NEAREST;
		case Texture::LINEAR:					return GL_LINEAR;
		case Texture::NEAREST_MIPMAP_NEAREST:	return GL_NEAREST_MIPMAP_NEAREST;
		case Texture::LINEAR_MIPMAP_NEAREST:	return GL_LINEAR_MIPMAP_NEAREST;
		case Texture::NEAREST_MIPMAP_LINEAR:	return GL_NEAREST_MIPMAP_LINEAR;
		case Texture::LINEAR_MIPMAP_LINEAR:		return GL_LINEAR_MIPMAP_LINEAR;
	}
	
	return GL_LINEAR;
}

GLenum format_from_texture_format(Texture::Format format) {
	switch(format) {
		case Texture::LUMINANCE: return GL_LUMINANCE;
		case Texture::ALPHA: return GL_ALPHA;
		case Texture::LUMALPHA: return GL_LUMINANCE_ALPHA;
		case Texture::RGB: return GL_RGB;
		case Texture::BGR: return GL_RGB;
		case Texture::RGBA: return GL_RGBA;
		case Texture::BGRA: return GL_RGBA;
	}
	
	return GL_RGBA;
}

GLenum type_from_texture_type(Texture::Type type) {
	switch(type) {
		case Texture::UCHAR:	return GL_UNSIGNED_BYTE;
		case Texture::INT:		return GL_INT;
		case Texture::UINT:		return GL_UNSIGNED_INT;
		case Texture::FLOAT32:	return GL_FLOAT;
	}
	
	return GL_UNSIGNED_BYTE;
}

GLenum internal_format_from_format(Texture::Format format, Texture::Type type) {
	if(type == Texture::FLOAT32) {// check for extension && mType == GL_FLOAT) {
		switch(format) {
			case Texture::LUMINANCE: return GL_LUMINANCE32F_ARB;
			case Texture::ALPHA: return GL_ALPHA32F_ARB;
			case Texture::LUMALPHA: return GL_LUMINANCE_ALPHA32F_ARB;
			case Texture::RGB: return GL_RGB32F_ARB;
			case Texture::BGR: return GL_RGB32F_ARB;
			case Texture::RGBA: return GL_RGBA32F_ARB;
			case Texture::BGRA: return GL_RGBA32F_ARB;
		}
	}
	else {
		switch(format) {
			case Texture::LUMINANCE: return GL_LUMINANCE;
			case Texture::ALPHA: return GL_ALPHA;
			case Texture::LUMALPHA: return GL_LUMINANCE_ALPHA;
			case Texture::RGB: return GL_RGB;
			case Texture::BGR: return GL_RGB;
			case Texture::RGBA: return GL_RGBA;
			case Texture::BGRA: return GL_RGBA;
		}
	}
	
	return GL_RGBA;
}

void GraphicsGL :: textureCreate(Texture *tex) {
	GLuint texid = 0;
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texid);
	
	if(texid <= 0) return;
	
	tex->id((int)texid);

	// get the OpenGL texture target
	tex->target(check_target(tex->target()));
	GLenum gltarget = target_from_texture_target(tex->target());
	

	glBindTexture(gltarget, texid);
	

	// check for valid dimensions
	int w, h, d;
	tex->getDimensions(w, h, d);
	clamp_texture_dimensions(tex->target(), w, h, d);

	
	// set the texcoord wrapping
	GLenum glwrap = wrap_from_texture_wrap(tex->target(), tex->wrap());
	if(gltarget == GL_TEXTURE_RECTANGLE_ARB) {
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, glwrap);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, glwrap);
	}
	else {
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, glwrap);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, glwrap);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, glwrap);
	}
	
	if(gltarget == GL_TEXTURE_3D) {
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, glwrap);

		#ifdef MURO_APPLE_VERSION	//this is due to a bug in the nvidia drivers on osx for z-slice updating
		glTexParameteri(gltarget, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
		#endif
	}

	
	// set the filters and border color
	GLenum glmagfilter = filter_from_texture_filter(tex->magFilter());
	GLenum glminfilter = filter_from_texture_filter(tex->minFilter());
	glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, glmagfilter);
	glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, glminfilter);
	glTexParameterfv(gltarget, GL_TEXTURE_BORDER_COLOR, tex->borderColor().to_ptr());


	GLenum glformat = format_from_texture_format(tex->format());
	GLenum gltype = type_from_texture_type(tex->type());
	GLenum iformat = internal_format_from_format(tex->format(), tex->type());
	
	switch(gltarget) {
		case GL_TEXTURE_1D:
			glTexImage1D(gltarget, 0, iformat, w, 0, glformat, gltype, NULL);
			break;
			
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE_ARB:
			glTexImage2D(gltarget, 0, iformat, w, h, 0, glformat, gltype, NULL);
			break;

		case GL_TEXTURE_3D:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glTexImage3D(gltarget, 0, iformat, w, h, d, 0, glformat, gltype, NULL);
			break;
	}


	glBindTexture(gltarget, 0);
	glDisable(gltarget);
	
	//	mRebuild = false;
	//	mUpdate = false;

	/*
	if(pixels == 0 && mStartClear) {
		err = clear();
		if(err) return err;
	}
	*/
//	err = gloGetGLError("Generating Texture");
//	return err;
}

void GraphicsGL :: textureDestroy(Texture *tex)  {
	if(tex->created()) {
		GLuint tex_id = (GLuint)tex->id();
		glDeleteTextures(1, &tex_id);
		tex->id(0);
	}
}

void GraphicsGL :: textureBind(Texture *tex, int unit) {
	// multitexturing
	glActiveTextureARB( GL_TEXTURE0_ARB+unit );

	GLenum gltarget = target_from_texture_target(tex->target());
	glEnable(gltarget);
	glBindTexture(gltarget, tex->id());
	
	//printf("(unit %i, target %i, id %ld)\n", unit, tex->target(), tex->id());
	gl_error("binding texture");
}

void GraphicsGL :: textureEnter(Texture *tex, int unit) {
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	if(tex->target() == Texture::TEXTURE_RECT) {
		glScalef(tex->width(), tex->height(), 1.);
	}
	glMatrixMode(GL_MODELVIEW);

	// set texture environment
//	glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, mCoordReplace);

	gl_error("entering texture");
}

void GraphicsGL :: textureLeave(Texture *tex, int unit) {
	// multitexturing unwind
	glActiveTextureARB( GL_TEXTURE0_ARB+unit );

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	
	gl_error("leaving texture");
}

void GraphicsGL :: textureUnbind(Texture *tex, int unit) {
	GLenum gltarget = target_from_texture_target(tex->target());
	glBindTexture(gltarget, 0);
	glDisable(gltarget);
	
	gl_error("unbinding texture");
}

void GraphicsGL :: textureToArray(Texture *tex) {
	if(tex->mode() == Texture::SURFACE) {
		Surface *surface = tex->surface();
		if(surface) {
//			SurfaceData *surface_data = (SurfaceData *)surface->surfaceData();
			//PBO *pbo = surface_data->pbo;
//			pbo->bind();
//			pbo->fromTexture(tex, format_from_texture_format(tex->format()));
//			pbo->unbind();
//			pbo->toArray(tex->array());
		}
	}
	else {
		// glTexImage
	}
}

void GraphicsGL :: textureSubmit(Texture *tex){
	GLvoid *data = (GLvoid *)tex->data();
	if(data) {
		GLenum gltarget = target_from_texture_target(tex->target());
		GLenum glformat = format_from_texture_format(tex->format());
		GLenum gltype = type_from_texture_type(tex->type());
		
		int alignment = tex->getRowStride() % 4;
		if(alignment == 0) {
			alignment = 4;
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
		
		switch(gltarget) {
			case GL_TEXTURE_1D:
				glTexSubImage1D(
					gltarget, 
					0, 0, 
					tex->width(), 
					glformat, 
					gltype, 
					data
				);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_RECTANGLE_ARB:
				glTexSubImage2D(
					gltarget, 
					0, 0, 0, 
					tex->width(), tex->height(), 
					glformat, 
					gltype, 
					data
				);
				break;
			case GL_TEXTURE_3D:
				glTexSubImage3D(
					gltarget, 
					0, 0, 0, 0, 
					tex->width(), tex->height(), tex->depth(), 
					glformat, 
					gltype, 
					data
				);
				break;

			default:
				break;
		}
	}
}


GLenum check_fbo_status(Surface *surface) {
	GLint fboid;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fboid);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)surface->id());

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;
			
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			printf("framebuffer GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");
			/* you gotta choose different formats */
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			printf("framebuffer INCOMPLETE_ATTACHMENT\n");
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			printf("framebuffer FRAMEBUFFER_MISSING_ATTACHMENT\n");
			break;
		
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			printf("framebuffer FRAMEBUFFER_DIMENSIONS\n");
			break;
		
		case 0x8CD8: //GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			printf("framebuffer INCOMPLETE_DUPLICATE_ATTACHMENT\n");
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			printf("framebuffer INCOMPLETE_FORMATS\n");
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			printf("framebuffer INCOMPLETE_DRAW_BUFFER\n");
			break;
			
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			printf("framebuffer INCOMPLETE_READ_BUFFER\n");
			break;
			
		case GL_FRAMEBUFFER_BINDING_EXT:
			printf("framebuffer BINDING_EXT\n");
			break;
			
		case 0x8CDE: // GL_FRAMEBUFFER_STATUS_ERROR_EXT:
			printf("framebuffer STATUS_ERROR\n");
			break;
			
		default:
			/* programming error; will fail on all hardware */
			//exit(0);
			break;
	}
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)fboid);
	
	return status;
}

// surfaces
Surface * GraphicsGL :: surfaceNew() {
	SurfaceData *surface_data = new SurfaceData();
	return new Surface(this, surface_data);
}

void surfaceAttachTexture(Surface *surface, Texture *tex) {
	// in case there's already an FBO bound, we want it rebound after this operation
	GLint fbo_id;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fbo_id);
	
	// attach the texture to the surface
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->id());
	tex->bind(0);
		glFramebufferTexture2DEXT(
			GL_FRAMEBUFFER_EXT, 
			GL_COLOR_ATTACHMENT0_EXT, 
			GraphicsGL::target_from_texture_target(tex->target()), 
			tex->id(), 
			0
		);
	tex->unbind(0);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)fbo_id);
}

void surfaceAttachDepthbuffer(Surface *surface) {
	int width = surface->width();
	int height = surface->height();
	
	GraphicsGL::SurfaceData *surface_data = (GraphicsGL::SurfaceData *)surface->surfaceData();
	
	// generate a new renderbuffer
	GLuint depth_id = 0;
	glGenRenderbuffersEXT(1, &depth_id);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_id);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	surface_data->depth_id = depth_id;

	// in case there's already an FBO bound, we want it rebound after this operation
	GLint fbo_id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fbo_id);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->id());
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_id);
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)fbo_id);
}


void GraphicsGL :: surfaceCreate(Surface *surface) {
	GLuint fbo_id = 0;
	glGenFramebuffersEXT(1, &fbo_id);
	surface->id(fbo_id);
	
	surfaceAttachTexture(surface, surface->texture());

	// attach depth buffer
	surfaceAttachDepthbuffer(surface);
	
	// see if we're FBO complete
	check_fbo_status(surface);	
}
void GraphicsGL :: surfaceDestroy(Surface *surface) {
	if(surface->created()) {
		GLuint fbo_id = (GLuint)surface->id();
		glDeleteFramebuffersEXT(1, &fbo_id);
		surface->id(0);
		
		SurfaceData *surface_data = (SurfaceData *)surface->surfaceData();
		surface_data->depth_id = 0;
	}
}
void GraphicsGL :: surfaceBind(Surface *surface) {
	glPushAttrib(GL_VIEWPORT_BIT);
	glDisable(GL_SCISSOR_TEST);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	
	/*
	GLint fboid;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fboid);
	mPrevfboid = (GLuint)fboid;
	*/
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->id());
}
void GraphicsGL :: surfaceUnbind(Surface *surface) {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPopAttrib();
}
void GraphicsGL :: surfaceEnter(Surface *surface) {
	glViewport(0, 0, surface->width(), surface->height());
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	
	if(surface->autoClear()) {
		Color &c = surface->clearColor();
		glClearColor(c.r, c.g, c.b, c.a);
		glClearDepth(surface->clearDepth());
		
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		/*
		for(int i=0; i < mNumAttachments; i++) {
			glDrawBuffer(bufferAttachPoint(i));
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		*/
	}
}
void GraphicsGL :: surfaceLeave(Surface *surface) {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void GraphicsGL::surfaceClear(Surface *surface) {
	surfaceBind(surface);
	surfaceEnter(surface);
	surfaceLeave(surface);
	surfaceUnbind(surface);
	
	gl_error("clearing surface");
}

void GraphicsGL::surfaceCopy(Surface *surface, Texture *texture) {
	glPushAttrib(GL_COLOR_BUFFER_BIT);	// to save blending state
	glDisable(GL_BLEND);

	surface->enter();
	texture->bind(0);

	glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrtho(-1.0, 1.0, -1.0, 1.0, -100, 100);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();

				glLoadIdentity();
				glBegin(GL_QUADS);
					glColor4f(1., 1., 1., 1.);
					glTexCoord2f(0., 1.);
					glVertex3f(-1., 1., 0.);

					glTexCoord2f(1., 1.);
					glVertex3f(1., 1., 0.);

					glTexCoord2f(1., 0.);
					glVertex3f(1., -1., 0.);

					glTexCoord2f(0., 0.);
					glVertex3f(-1., -1., 0.);

				glEnd();

			glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	texture->unbind(0);
	surface->leave();

	glPopAttrib();	// to restore blending state
	
	gl_error("copying surface");
}

