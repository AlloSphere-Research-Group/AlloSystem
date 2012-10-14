#include "allocore/graphics/al_Font.hpp"
#include <vector>

#if defined (__APPLE__) || defined (OSX)

	#include "ft2build.h"
	#include FT_FREETYPE_H
	#include FT_OUTLINE_H
	#include FT_GLYPH_H
	
	#define FONT_OSX 1
	#define FONT_SYSTEM_FONTS 1

#elif defined(__linux__)

	#include <ft2build.h>
	#include <freetype/freetype.h>
	#include <freetype/ftoutln.h>
	#include <freetype/ftglyph.h>

#elif defined WIN32

	#include <windows.h>
	#include <ft2build.h>
	#include FT_FREETYPE_H
	#include FT_OUTLINE_H
	#include FT_GLYPH_H

#endif

extern "C" {
	#include <stdlib.h>
	extern FT_Library front_freetype_library;
}

FT_Library front_freetype_library = 0;

#define GLYPHS_PER_ROW 16	// number of glyphs to pack in a row
#define PIX_TO_EM (64.f)

//#ifdef FONT_SYSTEM_FONTS
//
//extern int font_names(lua_State *L);
//extern int font_families(lua_State *L);
//
//#endif

struct FontCharacter{
	FontCharacter()
	:	width(10),
		y_offset(0)
	{}
	
	~FontCharacter() {}

	int width;
	int y_offset;
};

using namespace al;

inline bool initialize_font() {
	static bool initialized = false;
	if (!initialized) {
		if(!front_freetype_library) {
			FT_Error err = FT_Init_FreeType(&front_freetype_library);
			if(err) {
				AL_WARN("error initializing FreeType library");
				return false;
			}
		}
		initialized = true;
	}
	return initialized;
}

#pragma mark Impl
struct Font::Impl {
public:

	static Impl * create(Font& font, std::string filename) {
		if (!initialize_font()) return 0;
		
		FT_Face face;
		FT_Error err = FT_New_Face(front_freetype_library, filename.c_str(), 0, &face);
		if(err) {
			AL_WARN("error loading font face for %s", filename.c_str());
			return 0;
		}
		if(FT_Set_Pixel_Sizes(face, 0, font.mFontSize)) {
			AL_WARN("error setting font size");
			FT_Done_Face(face);
			return 0;
		}
		
		return new Impl(font, face);
	}
	
	virtual ~Impl() {
		FT_Done_Face(mFace);
	}
	
	// returns the "above-line" height of the font in pixels
	float ascender() { 
		return mFace->size->metrics.ascender/PIX_TO_EM; 
	}
	
	// returns the "below-line" height of the font in pixels
	float descender() { 
		return mFace->size->metrics.descender/PIX_TO_EM; 
	}
	
	unsigned textureWidth(unsigned fontSize) {
		return (fontSize+2)*GLYPHS_PER_ROW;
	}
	
	unsigned textureHeight(unsigned fontSize) {
		return (fontSize+2)*(ASCII_SIZE/GLYPHS_PER_ROW);
	}

protected:

	// factory pattern; use Impl::create()
	Impl(Font& font, FT_Face face) 
	:	mFace(face)
	{
		font.mTex.width(textureWidth(font.mFontSize));
		font.mTex.height(textureHeight(font.mFontSize));
		font.mTex.allocate();
		
		Array& arr = font.mTex.array();
		
		const int rowstride = arr.header.stride[1];
		char * optr = arr.data.ptr;
		const int padding_x = 1;
		const int padding_y = 1;
		const int glyph_width = font.mFontSize+2*padding_x;
		const int glyph_height = font.mFontSize+2*padding_y;
			
		for(int i=0; i < ASCII_SIZE; i++) {
			
			// load glyph:
			int glyph_index = FT_Get_Char_Index(mFace, i);
			if(font.mAntiAliased) {
				FT_Load_Glyph(mFace, glyph_index, FT_LOAD_DEFAULT);
				FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_NORMAL);
			}
			else {
				FT_Load_Glyph(mFace, glyph_index, FT_LOAD_MONOCHROME);
				FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_MONO);
			}
			
			// store metrics:
			font.mChars[i].width = mFace->glyph->advance.x/PIX_TO_EM;
			font.mChars[i].y_offset = mFace->glyph->bitmap_top;
			
			//printf("char %i %c %i %i\n", i, i, font.mChars[i].width, font.mChars[i].y_offset);
			
			// calculate texture pointer offset:
			int xidx = i % GLYPHS_PER_ROW;
			int yidx = (int)((float)i/(float)GLYPHS_PER_ROW);
			int offset = (padding_y + yidx*glyph_height)*rowstride + 
						 (padding_x + xidx*glyph_width );
			
			unsigned char * image = (unsigned char *)(optr + offset);
				
			// write glyph bitmap into texture:
			FT_Bitmap *bitmap = &mFace->glyph->bitmap;
			//if(font.mAntiAliased) {
				for(int j=0; j < bitmap->rows; j++) {
					unsigned char *pix = image + j*rowstride;
					unsigned char *font_pix = bitmap->buffer + j*bitmap->width;
					for(int k=0; k < bitmap->width; k++) {
						*pix++ = *font_pix++;
					}
				}
			//}
		}
	
	}

	FT_Face mFace;
};



Font :: Font(std::string filename, int font_size, bool anti_aliased)
:	mFontSize(font_size),
	mAntiAliased(anti_aliased),
//	mTex(g, 192, 192, Graphics::LUMINANCE, Graphics::UCHAR)
	mTex(192, 192)
{
	mTex.format(Graphics::LUMINANCE).type(Graphics::UBYTE);
	// TODO: if this fails (mImpl == NULL), fall back to native options (e.g. Cocoa)?
	mImpl = Impl::create(*this, filename);
}


Font :: ~Font() {
	delete mImpl;
}

// returns the "above-line" height of the font in pixels
float Font :: ascender() { return mImpl->ascender(); }

// returns the "below-line" height of the font in pixels
float Font :: descender() { return mImpl->descender(); }

void Font :: write(Mesh& mesh, std::string text) {
	
	mesh.reset();
	mesh.primitive(Graphics::QUADS);

	int nchars = text.size();
	float margin = 2.;
	float csz = (float)mFontSize;
	float cdim = csz+margin;
	float tdim = cdim*GLYPHS_PER_ROW;
	float tcdim = ((float)cdim)/((float)tdim);
	
	float pos[] = {0., 0.};
	
	for(int i=0; i < nchars; i++) {
		int idx = text[i];
		const FontCharacter &c = mChars[idx];
		/*int margin = 1;*/
		
		int xidx = idx % GLYPHS_PER_ROW;
		int yidx = idx / GLYPHS_PER_ROW;
		float yy = c.y_offset;
		
		float tc_x0	= ((float)(xidx))*tcdim;
		float tc_y0	= ((float)(yidx))*tcdim;
		float tc_x1	= tc_x0+tcdim;
		float tc_y1	= tc_y0+tcdim;
		
		float v_x0  = pos[0];
		float v_x1	= v_x0+cdim;
		float v_y0	= margin+yy-pos[1];
		float v_y1	= yy-csz-pos[1];
		
		// draw character quad:
		mesh.texCoord(	tc_x0,	tc_y0);
		mesh.vertex(	v_x0,	v_y0,	0);
		
		mesh.texCoord(	tc_x1,	tc_y0);
		mesh.vertex(	v_x1,	v_y0,	0);
		
		mesh.texCoord(	tc_x1,	tc_y1);
		mesh.vertex(	v_x1,	v_y1,	0);
		
		mesh.texCoord(	tc_x0,	tc_y1);
		mesh.vertex(	v_x0,	v_y1,	0);
		
		pos[0] += (float)c.width;
	}
}

