//#include <vector>
//#include <map>
//#include <string>
#include <stdio.h>

#include "allocore/types/al_Array.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Graphics::Graphics() : mInImmediateMode(false) {}
Graphics::~Graphics() {}


int Graphics::numComponents(Format v){
	switch(v){
		case BGRA:
		case RGBA:				return 4;
		case RGB:				return 3;
		case LUMINANCE_ALPHA:	return 2;
		case DEPTH_COMPONENT:
		case LUMINANCE:
		case RED:
		case GREEN:
		case BLUE:
		case ALPHA:				return 1;
		default:				return 0;
	};
}

int Graphics::numBytes(DataType v){
	#define CS(a,b) case a: return sizeof(b); 
	switch(v){
		CS(BYTE, char)
		CS(UBYTE, unsigned char)
		CS(INT, int)
		CS(UINT, unsigned int)
		CS(FLOAT, float)
		default: return 0;
	};
	#undef CS
}

bool Graphics::error(const char * msg, FILE * fp){
	GLenum err = glGetError();

	#define POST "The offending command is ignored and has no other side effect than to set the error flag."
	switch(err) {
		case GL_INVALID_ENUM:	fprintf(fp,"%s:\n %s\n", msg, "An unacceptable value is specified for an enumerated argument. "POST); return true;
		case GL_INVALID_VALUE:	fprintf(fp,"%s:\n %s\n", msg, "A numeric argument is out of range. "POST); return true;
		case GL_INVALID_OPERATION:fprintf(fp,"%s:\n %s\n", msg, "The specified operation is not allowed in the current state. "POST); return true;
		case GL_STACK_OVERFLOW:	fprintf(fp,"%s:\n %s\n", msg, "This command would cause a stack overflow. "POST); return true;
		case GL_STACK_UNDERFLOW:fprintf(fp,"%s:\n %s\n", msg, "This command would cause a stack underflow. "POST); return true;
		case GL_OUT_OF_MEMORY:	fprintf(fp,"%s:\n %s\n", msg, "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded."); return true;
		case GL_TABLE_TOO_LARGE:fprintf(fp,"%s:\n %s\n", msg, "The specified table exceeds the implementation's maximum supported table size. "POST); return true;
		case GL_NO_ERROR: break;
		default: break;
	}
	#undef POST
	return false;
}

Graphics::DataType Graphics::toDataType(AlloTy v){
	switch(v){
		case AlloFloat32Ty: return FLOAT;
		case AlloFloat64Ty: return DOUBLE;
		case AlloSInt8Ty:	return BYTE;
		case AlloUInt8Ty:	return UBYTE;
		case AlloSInt16Ty:	return SHORT;
		case AlloUInt16Ty:	return USHORT;
		case AlloSInt32Ty:	return INT;
		case AlloUInt32Ty:	return UINT;
		default:			return BYTE;
	}
}

enum DataType {
		BYTE					= GL_BYTE,
		UBYTE					= GL_UNSIGNED_BYTE,
		SHORT					= GL_SHORT,
		USHORT					= GL_UNSIGNED_SHORT,
		INT						= GL_INT,
		UINT					= GL_UNSIGNED_INT,
		FLOAT					= GL_FLOAT,
		DOUBLE					= GL_DOUBLE
	};


AlloTy Graphics :: toAlloTy(Graphics::DataType v) {
	switch (v) {
		case BYTE:		return AlloSInt8Ty;
		case UBYTE:		return AlloUInt8Ty;
		case SHORT:		return AlloSInt16Ty;
		case USHORT:	return AlloUInt16Ty;
		case INT:		return AlloSInt32Ty;
		case UINT:		return AlloUInt32Ty;
		case FLOAT:		return AlloFloat32Ty;
		case DOUBLE:	return AlloFloat64Ty;
		default:		return AlloVoidTy;
	}
}

void Graphics::antialiasing(AntiAliasMode v){
	glHint(GL_POINT_SMOOTH_HINT, v);
	glHint(GL_LINE_SMOOTH_HINT, v);
	glHint(GL_POLYGON_SMOOTH_HINT, v);

	if (FASTEST != v) {
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	} else {
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
}

void Graphics::fog(float end, float start, const Color& c){
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR); 
	glFogf(GL_FOG_START, start); glFogf(GL_FOG_END, end);
	float fogColor[4] = {c.r, c.g, c.b, c.a};
	glFogfv(GL_FOG_COLOR, fogColor);
}

void Graphics::viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
	enable(SCISSOR_TEST);
	glScissor(x, y, width, height);
}


// Immediate Mode
void Graphics::begin(Primitive v) {
	// clear buffers
	mMesh.reset();

	// start primitive drawing
	mMesh.primitive(v);
	mInImmediateMode = true;
}

void Graphics::end() {
	draw(mMesh);
	mInImmediateMode = false;
}

void Graphics::vertex(double x, double y, double z) {
	if(mInImmediateMode) {
		// make sure all buffers are the same size if > 0
		mMesh.vertex(x, y, z);
		mMesh.equalizeBuffers();
	}	
}

void Graphics::texCoord(double u, double v) {
	if(mInImmediateMode) {
		mMesh.texCoord(u, v);
	}
}

void Graphics::texCoord(double s, double t, double r) {
	if(mInImmediateMode) {
		mMesh.texCoord(s, t, r);
	}
}

void Graphics::normal(double x, double y, double z) {
	if(mInImmediateMode) {
		mMesh.normal(x, y, z);
	}
}

void Graphics::color(double r, double g, double b, double a) {
	if(mInImmediateMode) {
		mMesh.color(r, g, b, a);
	} else {
		currentColor(r, g, b, a);
	}
}



// Buffer drawing
void Graphics::draw(const Mesh& v, CommandMode mode){

	const int Nv = v.vertices().size();
	if(0 == Nv) return;
	
	const int Nc = v.colors().size();
	const int Nci= v.coloris().size();
	const int Nn = v.normals().size();
	const int Nt2= v.texCoord2s().size();
	const int Nt3= v.texCoord3s().size();
	const int Ni = v.indices().size();
	
	//printf("client %d, GPU %d\n", clientSide, gpuSide);
	//printf("Nv %i Nc %i Nn %i Nt2 %i Nt3 %i Ni %i\n", Nv, Nc, Nn, Nt2, Nt3, Ni);

	if(CLIENT_BEGIN & mode){
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
		else if(Nci >= Nv){
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, &v.coloris()[0]);
	//		printf("using integer colors\n");	
		}
		else if(0 == Nc && 0 == Nci){
			// just use whatever the last glColor() call used!
		}
		else{
			if(Nc)
				//glColor4f(v.colors()[0][0], v.colors()[0][1], v.colors()[0][2], v.colors()[0][3]);
				glColor4fv(v.colors()[0].components);
			else
				glColor3ubv(v.coloris()[0].components);
		}
		
		if(Nt2 || Nt3){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if(Nt2 >= Nv) glTexCoordPointer(2, GL_FLOAT, 0, &v.texCoord2s()[0]);
			if(Nt3 >= Nv) glTexCoordPointer(3, GL_FLOAT, 0, &v.texCoord3s()[0]);
		}
	}

	if(SERVER & mode){
		if(Ni){
			//unsigned vs=0, ve=Nv;	// range of vertex indices to prefetch
									// NOTE:	if this range exceeds the number of vertices,
									//			expect a segmentation fault...
			unsigned is=0, ie=Ni;	// range of indices to draw

	//		glDrawRangeElements(v.primitive(), vs, ve, ie-is, GL_UNSIGNED_INT, &v.indices()[is]);
			glDrawElements(
				((Graphics::Primitive)v.primitive()), 
				ie-is, 
				GL_UNSIGNED_INT, 
				&v.indices()[is]
			);
		}
		else{
			glDrawArrays(
				((Graphics::Primitive)v.primitive()), 
				0,
				v.vertices().size()
			);
		}
	}
	
	if(CLIENT_END & mode){
						glDisableClientState(GL_VERTEX_ARRAY);
		if(Nn)			glDisableClientState(GL_NORMAL_ARRAY);
		if(Nc || Nci)	glDisableClientState(GL_COLOR_ARRAY);
		if(Nt2 || Nt3)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}






//GLenum GraphicsGL :: target_from_texture_target(Texture::Target target) {
//	switch(target) {
//		case Texture::TEXTURE_1D:	return GL_TEXTURE_1D;
//		case Texture::TEXTURE_2D:	return GL_TEXTURE_2D;
//		case Texture::TEXTURE_RECT:	return GL_TEXTURE_RECTANGLE_ARB;
//		case Texture::TEXTURE_3D:	return GL_TEXTURE_3D;
//	}
//	
//	return GL_TEXTURE_2D;
//}
//
//Texture::Target check_target(Texture::Target target) {
//	switch(target) {
//		case Texture::TEXTURE_RECT:
//			/*if(! Glob::extensions().haveRectTexture()) {
//				fprintf(stderr, "Rectangular textures not supported on hard, reverting to 2D textures\n");
//				return GL_TEXTURE_2D;
//			}
//			else return mTarget;
//			*/
//			return Texture::TEXTURE_RECT;
//
//		case Texture::TEXTURE_3D:
//			/*
//			if(! Glob::extensions().have3DTexture()) {
//				fprintf(stderr, "3D textures not supported on hard, reverting to 2D textures\n");
//				return GL_TEXTURE_2D;
//			}
//			else return mTarget;
//			*/
//			return Texture::TEXTURE_3D;
//		
//		case Texture::TEXTURE_1D:
//		case Texture::TEXTURE_2D:
//		default:
//			return target;
//	}
//}
//
//
//void clamp_texture_dimensions(Texture::Target target, int &w, int &h, int &d) {
//	/*
//	switch(mTarget) {
//		case GL_TEXTURE_RECTANGLE_ARB: {
//			int sz = Glob::extensions().maxRectTextureSize();
//			mWidth = (mWidth > sz) ? sz : mWidth;
//			mHeight = (mHeight > sz) ? sz : mHeight;
//		} break;
//		
//		case GL_TEXTURE_3D: {
//			int sz = Glob::extensions().max3DTextureSize();
//			mWidth = (mWidth > sz) ? sz : mWidth;
//			mHeight = (mHeight > sz) ? sz : mHeight;
//			mDepth = (mDepth > sz) ? sz : mDepth;
//		} break;
//		
//		default: {
//			int sz = Glob::extensions().maxTextureSize();
//			mWidth = (mWidth > sz) ? sz : mWidth;
//			mHeight = (mHeight > sz) ? sz : mHeight;
//			mDepth = (mDepth > sz) ? sz : mDepth;
//		} break;
//	}
//	*/
//}
//
//GLenum wrap_from_texture_wrap(Texture::Target target, Texture::Wrap wrap) {
//	if(target == Texture::TEXTURE_RECT) {
//		switch(wrap) {
//			case Texture::CLAMP_TO_EDGE:	return GL_CLAMP_TO_EDGE;
//			case Texture::CLAMP_TO_BORDER:	return GL_CLAMP_TO_BORDER;
//			case Texture::CLAMP:			return GL_CLAMP;
//			case Texture::REPEAT:			return GL_REPEAT;
//			default:
//				return GL_CLAMP_TO_BORDER;
//		}
//	}
//	else {
//		switch(wrap) {
//			case Texture::CLAMP_TO_EDGE:	return GL_CLAMP_TO_EDGE;
//			case Texture::CLAMP_TO_BORDER:	return GL_CLAMP_TO_BORDER;
//			case Texture::CLAMP:			return GL_CLAMP;
//			case Texture::REPEAT:			return GL_REPEAT;
//			default:;
//		}
//	}
//	
//	return GL_CLAMP_TO_BORDER;
//}
//
//GLenum filter_from_texture_filter(Texture::Filter filter) {
//	switch(filter) {
//		case Texture::NEAREST:					return GL_NEAREST;
//		case Texture::LINEAR:					return GL_LINEAR;
//		case Texture::NEAREST_MIPMAP_NEAREST:	return GL_NEAREST_MIPMAP_NEAREST;
//		case Texture::LINEAR_MIPMAP_NEAREST:	return GL_LINEAR_MIPMAP_NEAREST;
//		case Texture::NEAREST_MIPMAP_LINEAR:	return GL_NEAREST_MIPMAP_LINEAR;
//		case Texture::LINEAR_MIPMAP_LINEAR:		return GL_LINEAR_MIPMAP_LINEAR;
//	}
//	
//	return GL_LINEAR;
//}
//
//GLenum format_from_texture_format(Texture::Format format) {
//	switch(format) {
//		case Texture::LUMINANCE: return GL_LUMINANCE;
//		case Texture::ALPHA: return GL_ALPHA;
//		case Texture::LUMALPHA: return GL_LUMINANCE_ALPHA;
//		case Texture::RGB: return GL_RGB;
//		case Texture::BGR: return GL_RGB;
//		case Texture::RGBA: return GL_RGBA;
//		case Texture::BGRA: return GL_RGBA;
//	}
//	
//	return GL_RGBA;
//}
//
//GLenum type_from_texture_type(Texture::Type type) {
//	switch(type) {
//		case Texture::UCHAR:	return GL_UNSIGNED_BYTE;
//		case Texture::INT:		return GL_INT;
//		case Texture::UINT:		return GL_UNSIGNED_INT;
//		case Texture::FLOAT32:	return GL_FLOAT;
//	}
//	
//	return GL_UNSIGNED_BYTE;
//}
//
//GLenum internal_format_from_format(Texture::Format format, Texture::Type type) {
//	if(type == Texture::FLOAT32) {// check for extension && mType == GL_FLOAT) {
//		switch(format) {
//			case Texture::LUMINANCE: return GL_LUMINANCE32F_ARB;
//			case Texture::ALPHA: return GL_ALPHA32F_ARB;
//			case Texture::LUMALPHA: return GL_LUMINANCE_ALPHA32F_ARB;
//			case Texture::RGB: return GL_RGB32F_ARB;
//			case Texture::BGR: return GL_RGB32F_ARB;
//			case Texture::RGBA: return GL_RGBA32F_ARB;
//			case Texture::BGRA: return GL_RGBA32F_ARB;
//		}
//	}
//	else {
//		switch(format) {
//			case Texture::LUMINANCE: return GL_LUMINANCE;
//			case Texture::ALPHA: return GL_ALPHA;
//			case Texture::LUMALPHA: return GL_LUMINANCE_ALPHA;
//			case Texture::RGB: return GL_RGB;
//			case Texture::BGR: return GL_RGB;
//			case Texture::RGBA: return GL_RGBA;
//			case Texture::BGRA: return GL_RGBA;
//		}
//	}
//	
//	return GL_RGBA;
//}
//
//void GraphicsGL :: textureCreate(Texture *tex) {
////printf("GraphicsGL::textureCreate\n");
//	GLuint texid = 0;
//	
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//	glGenTextures(1, &texid);
//	
//	if(texid <= 0) return;
//	
//	tex->id((int)texid);
//
//	// get the OpenGL texture target
//	tex->target(check_target(tex->target()));
//	GLenum gltarget = target_from_texture_target(tex->target());
//
//	glBindTexture(gltarget, texid);
//	
//
//	// check for valid dimensions
//	int w, h, d;
//	tex->getDimensions(w, h, d);
//	clamp_texture_dimensions(tex->target(), w, h, d);
//
//	
//	// set the texcoord wrapping
//	GLenum glwrap = wrap_from_texture_wrap(tex->target(), tex->wrap());
//	if(gltarget == GL_TEXTURE_RECTANGLE_ARB) {
//		glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, glwrap);
//		glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, glwrap);
//	}
//	else {
//		glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, glwrap);
//		glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, glwrap);
//		glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, glwrap);
//	}
//	
//	if(gltarget == GL_TEXTURE_3D) {
//		glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, glwrap);
//
//		#ifdef MURO_APPLE_VERSION	//this is due to a bug in the nvidia drivers on osx for z-slice updating
//		glTexParameteri(gltarget, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
//		#endif
//	}
//
//	
//	// set the filters and border color
//	GLenum glmagfilter = filter_from_texture_filter(tex->magFilter());
//	GLenum glminfilter = filter_from_texture_filter(tex->minFilter());
//	glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, glmagfilter);
//	glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, glminfilter);
//	glTexParameterfv(gltarget, GL_TEXTURE_BORDER_COLOR, tex->borderColor().components);
//
//	GLenum glformat = format_from_texture_format(tex->format());
//	GLenum gltype = type_from_texture_type(tex->type());
//	GLenum iformat = internal_format_from_format(tex->format(), tex->type());
//	
//	switch(gltarget) {
//		case GL_TEXTURE_1D:
//			glTexImage1D(gltarget, 0, iformat, w, 0, glformat, gltype, NULL);
//			break;
//			
//		case GL_TEXTURE_2D:
////printf("GraphicsGL::textureCreate glTexImage2D\n");
//		case GL_TEXTURE_RECTANGLE_ARB:
//			glTexImage2D(gltarget, 0, iformat, w, h, 0, glformat, gltype, NULL);
//			break;
//
//		case GL_TEXTURE_3D:
//			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//			glTexImage3D(gltarget, 0, iformat, w, h, d, 0, glformat, gltype, NULL);
//			break;
//	}
//
////	printf("%x, %x, (%d, %d), %x %x %p\n", gltarget, iformat, w,h, glformat, gltype, tex->data());
//
//	glBindTexture(gltarget, 0);
//	glDisable(gltarget);
//	
//	//	mRebuild = false;
//	//	mUpdate = false;
//
//	/*
//	if(pixels == 0 && mStartClear) {
//		err = clear();
//		if(err) return err;
//	}
//	*/
////	err = gloGetGLError("Generating Texture");
////	return err;
//}
//
//void GraphicsGL :: textureDestroy(Texture *tex)  {
//	if(tex->created()) {
//		GLuint tex_id = (GLuint)tex->id();
//		glDeleteTextures(1, &tex_id);
//		tex->id(0);
//	}
//}
//
//void GraphicsGL :: textureBind(Texture *tex, int unit) {
//	// multitexturing
//	glActiveTextureARB( GL_TEXTURE0_ARB+unit );
//
//	GLenum gltarget = target_from_texture_target(tex->target());
//	glEnable(gltarget);
//	glBindTexture(gltarget, tex->id());
//	
//	//printf("(unit %i, target %i, id %ld)\n", unit, tex->target(), tex->id());
//	gl_error("binding texture");
//}
//
//void GraphicsGL :: textureEnter(Texture *tex, int unit) {
//	glMatrixMode(GL_TEXTURE);
//	glPushMatrix();
//	glLoadIdentity();
//
//	if(tex->target() == Texture::TEXTURE_RECT) {
//		glScalef(tex->width(), tex->height(), 1.);
//	}
//	glMatrixMode(GL_MODELVIEW);
//
//	// set texture environment
////	glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, mCoordReplace);
//
//	gl_error("entering texture");
//}
//
//void GraphicsGL :: textureLeave(Texture *tex, int unit) {
//	// multitexturing unwind
//	glActiveTextureARB( GL_TEXTURE0_ARB+unit );
//
//	glMatrixMode(GL_TEXTURE);
//	glPopMatrix();
//
//	glMatrixMode(GL_MODELVIEW);
//	
//	gl_error("leaving texture");
//}
//
//void GraphicsGL :: textureUnbind(Texture *tex, int unit) {
//	GLenum gltarget = target_from_texture_target(tex->target());
//	glBindTexture(gltarget, 0);
//	glDisable(gltarget);
//	
//	gl_error("unbinding texture");
//}
//
//void GraphicsGL :: textureToArray(Texture *tex) {
//	if(tex->mode() == Texture::SURFACE) {
//		Surface *surface = tex->surface();
//		if(surface) {
////			SurfaceData *surface_data = (SurfaceData *)surface->surfaceData();
//			//PBO *pbo = surface_data->pbo;
////			pbo->bind();
////			pbo->fromTexture(tex, format_from_texture_format(tex->format()));
////			pbo->unbind();
////			pbo->toArray(tex->array());
//		}
//	}
//	else {
//		// glTexImage
//	}
//}
//
//void GraphicsGL :: textureSubmit(Texture *tex){
//	GLvoid *data = (GLvoid *)tex->data();
//	if(data) {
//		GLenum gltarget = target_from_texture_target(tex->target());
//		GLenum glformat = format_from_texture_format(tex->format());
//		GLenum gltype = type_from_texture_type(tex->type());
//		
//		int alignment = tex->getRowStride() % 4;
//		if(alignment == 0) {
//			alignment = 4;
//		}
//		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
//		
//		switch(gltarget) {
//			case GL_TEXTURE_1D:
//				glTexSubImage1D(
//					gltarget, 
//					0, 0, 
//					tex->width(), 
//					glformat, 
//					gltype, 
//					data
//				);
//				break;
//			case GL_TEXTURE_2D:
//			case GL_TEXTURE_RECTANGLE_ARB:
////printf("GraphicsGL::textureSubmit glTexSubImage2D %p\n", tex->data());
//				glTexSubImage2D(
//					gltarget, 
//					0, 0, 0, 
//					tex->width(), tex->height(), 
//					glformat,
//					gltype,
//					data
//				);
//				break;
//			case GL_TEXTURE_3D:
//				glTexSubImage3D(
//					gltarget, 
//					0, 0, 0, 0, 
//					tex->width(), tex->height(), tex->depth(), 
//					glformat, 
//					gltype, 
//					data
//				);
//				break;
//
//			default:
//				break;
//		}
//	}
//}
//
//
//GLenum check_fbo_status(Surface *surface) {
//	GLint fboid;
//	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fboid);
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)surface->id());
//
//	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//	switch(status) {
//		case GL_FRAMEBUFFER_COMPLETE_EXT:
//			break;
//			
//		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
//			printf("framebuffer GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");
//			/* you gotta choose different formats */
//			break;
//			
//		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
//			printf("framebuffer INCOMPLETE_ATTACHMENT\n");
//			break;
//		
//		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
//			printf("framebuffer FRAMEBUFFER_MISSING_ATTACHMENT\n");
//			break;
//		
//		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
//			printf("framebuffer FRAMEBUFFER_DIMENSIONS\n");
//			break;
//		
//		case 0x8CD8: //GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
//			printf("framebuffer INCOMPLETE_DUPLICATE_ATTACHMENT\n");
//			break;
//			
//		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
//			printf("framebuffer INCOMPLETE_FORMATS\n");
//			break;
//			
//		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
//			printf("framebuffer INCOMPLETE_DRAW_BUFFER\n");
//			break;
//			
//		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
//			printf("framebuffer INCOMPLETE_READ_BUFFER\n");
//			break;
//			
//		case GL_FRAMEBUFFER_BINDING_EXT:
//			printf("framebuffer BINDING_EXT\n");
//			break;
//			
//		case 0x8CDE: // GL_FRAMEBUFFER_STATUS_ERROR_EXT:
//			printf("framebuffer STATUS_ERROR\n");
//			break;
//			
//		default:
//			/* programming error; will fail on all hardware */
//			//exit(0);
//			break;
//	}
//	
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)fboid);
//	
//	return status;
//}
//
//// surfaces
//Surface * GraphicsGL :: surfaceNew() {
//	SurfaceData *surface_data = new SurfaceData();
//	return new Surface(this, surface_data);
//}
//
//void surfaceAttachTexture(Surface *surface, Texture *tex) {
//	// in case there's already an FBO bound, we want it rebound after this operation
//	GLint fbo_id;
//	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fbo_id);
//	
//	// attach the texture to the surface
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->id());
//	tex->bind(0);
//		glFramebufferTexture2DEXT(
//			GL_FRAMEBUFFER_EXT, 
//			GL_COLOR_ATTACHMENT0_EXT, 
//			GraphicsGL::target_from_texture_target(tex->target()), 
//			tex->id(), 
//			0
//		);
//	tex->unbind(0);
//	
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)fbo_id);
//}
//
//void surfaceAttachDepthbuffer(Surface *surface) {
//	int width = surface->width();
//	int height = surface->height();
//	
//	GraphicsGL::SurfaceData *surface_data = (GraphicsGL::SurfaceData *)surface->surfaceData();
//	
//	// generate a new renderbuffer
//	GLuint depth_id = 0;
//	glGenRenderbuffersEXT(1, &depth_id);
//	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_id);
//	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
//	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
//	surface_data->depth_id = depth_id;
//
//	// in case there's already an FBO bound, we want it rebound after this operation
//	GLint fbo_id = 0;
//	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fbo_id);
//	
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->id());
//	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_id);
//	
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)fbo_id);
//}
//
//
//void GraphicsGL :: surfaceCreate(Surface *surface) {
//	GLuint fbo_id = 0;
//	glGenFramebuffersEXT(1, &fbo_id);
//	surface->id(fbo_id);
//	
//	surfaceAttachTexture(surface, surface->texture());
//
//	// attach depth buffer
//	surfaceAttachDepthbuffer(surface);
//	
//	// see if we're FBO complete
//	check_fbo_status(surface);	
//}
//void GraphicsGL :: surfaceDestroy(Surface *surface) {
//	if(surface->created()) {
//		GLuint fbo_id = (GLuint)surface->id();
//		glDeleteFramebuffersEXT(1, &fbo_id);
//		surface->id(0);
//		
//		SurfaceData *surface_data = (SurfaceData *)surface->surfaceData();
//		surface_data->depth_id = 0;
//	}
//}
//void GraphicsGL :: surfaceBind(Surface *surface) {
//	glPushAttrib(GL_VIEWPORT_BIT);
//	glDisable(GL_SCISSOR_TEST);
//	
//	glBindTexture(GL_TEXTURE_2D, 0);
//	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
//	glDisable(GL_TEXTURE_2D);
//	glDisable(GL_TEXTURE_RECTANGLE_ARB);
//	
//	/*
//	GLint fboid;
//	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &fboid);
//	mPrevfboid = (GLuint)fboid;
//	*/
//	
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, surface->id());
//}
//void GraphicsGL :: surfaceUnbind(Surface *surface) {
//	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
//	glPopAttrib();
//}
//void GraphicsGL :: surfaceEnter(Surface *surface) {
//	glViewport(0, 0, surface->width(), surface->height());
//	
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();	
//	
//	if(surface->autoClear()) {
//		Color &c = surface->clearColor();
//		glClearColor(c.r, c.g, c.b, c.a);
//		glClearDepth(surface->clearDepth());
//		
//		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		
//		/*
//		for(int i=0; i < mNumAttachments; i++) {
//			glDrawBuffer(bufferAttachPoint(i));
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		}
//		*/
//	}
//}
//void GraphicsGL :: surfaceLeave(Surface *surface) {
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
//	glMatrixMode(GL_MODELVIEW);
//	glPopMatrix();
//}
//
//
//void GraphicsGL::surfaceClear(Surface *surface) {
//	surfaceBind(surface);
//	surfaceEnter(surface);
//	surfaceLeave(surface);
//	surfaceUnbind(surface);
//	
//	gl_error("clearing surface");
//}
//
//void GraphicsGL::surfaceCopy(Surface *surface, Texture *texture) {
//	glPushAttrib(GL_COLOR_BUFFER_BIT);	// to save blending state
//	glDisable(GL_BLEND);
//
//	surface->enter();
//	texture->bind(0);
//
//	glMatrixMode(GL_PROJECTION);
//		glPushMatrix();
//			glLoadIdentity();
//			glOrtho(-1.0, 1.0, -1.0, 1.0, -100, 100);
//
//			glMatrixMode(GL_MODELVIEW);
//			glPushMatrix();
//
//				glLoadIdentity();
//				glBegin(GL_QUADS);
//					glColor4f(1., 1., 1., 1.);
//					glTexCoord2f(0., 1.);
//					glVertex3f(-1., 1., 0.);
//
//					glTexCoord2f(1., 1.);
//					glVertex3f(1., 1., 0.);
//
//					glTexCoord2f(1., 0.);
//					glVertex3f(1., -1., 0.);
//
//					glTexCoord2f(0., 0.);
//					glVertex3f(-1., -1., 0.);
//
//				glEnd();
//
//			glPopMatrix();
//		glMatrixMode(GL_PROJECTION);
//		glPopMatrix();
//
//	glMatrixMode(GL_MODELVIEW);
//
//	texture->unbind(0);
//	surface->leave();
//
//	glPopAttrib();	// to restore blending state
//	
//	gl_error("copying surface");
//}


} // al::
