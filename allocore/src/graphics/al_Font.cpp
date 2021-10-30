#include <cstdarg>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H

extern "C" {
	#include <stdlib.h>
	extern FT_Library front_freetype_library;
}

FT_Library front_freetype_library = 0;

#define GLYPHS_PER_ROW 16	// number of glyphs to pack in a row
#define PIX_TO_EM (64.f)

namespace al{

struct Font::Impl {
public:

	static Impl * create(){
		static bool initialized = false;
		if (!initialized) {
			if(!front_freetype_library) {
				FT_Error err = FT_Init_FreeType(&front_freetype_library);
				if(err) {
					AL_WARN("error initializing FreeType library");
					return nullptr;
				}
			}
			initialized = true;
		}

		return new Impl;
	}

	~Impl() {
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

	bool load(Font& font, const char * filename, int fontSize, bool antialias){

		FT_Face face; // preserve mFace member
		FT_Error err = FT_New_Face(front_freetype_library, filename, 0, &face);
		if(err) {
			AL_WARN("could not load font face %s", filename);
			return false;
		}
		if(FT_Set_Pixel_Sizes(face, 0, fontSize)) {
			AL_WARN("could not set font size");
			FT_Done_Face(face);
			return false;
		}

		mFace = face;

		font.mTex.width(textureWidth(fontSize));
		font.mTex.height(textureHeight(fontSize));
		font.mTex.allocate();

		Array& arr = font.mTex.array();

		const int rowstride = arr.header.stride[1];
		char * optr = arr.data.ptr;
		const int padding_x = 1;
		const int padding_y = 1;
		const int glyph_width = fontSize+2*padding_x;
		const int glyph_height = fontSize+2*padding_y;

		for(int i=0; i < ASCII_SIZE; i++) {

			// load glyph:
			int glyph_index = FT_Get_Char_Index(mFace, i);
			FT_GlyphSlot glyph = mFace->glyph;
			if(antialias) {
				FT_Load_Glyph(mFace, glyph_index, FT_LOAD_DEFAULT);
				FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
			}
			else {
				FT_Load_Glyph(mFace, glyph_index, FT_LOAD_MONOCHROME);
				FT_Render_Glyph(glyph, FT_RENDER_MODE_MONO);
			}

			// store metrics:
			font.mChars[i].width = glyph->advance.x/PIX_TO_EM;
			font.mChars[i].x_offset = glyph->bitmap_left;
			font.mChars[i].y_offset = glyph->bitmap_top;

			/*switch(glyph->format){
			case FT_GLYPH_FORMAT_BITMAP: printf("FT_GLYPH_FORMAT_BITMAP\n"); break;
			case FT_GLYPH_FORMAT_OUTLINE: printf("FT_GLYPH_FORMAT_OUTLINE\n"); break;
			case FT_GLYPH_FORMAT_COMPOSITE: printf("FT_GLYPH_FORMAT_COMPOSITE\n"); break;
			default:;
			}*/

			//printf("%c: advance (%d %d), bitmap_left %d\n", (char)i, (int)glyph->advance.x, (int)glyph->advance.y, glyph->bitmap_left);
			//printf("char %i %c %i %i\n", i, i, font.mChars[i].width, font.mChars[i].y_offset);
			//printf("bitmap.pitch %d\n", glyph->bitmap.pitch);

			// calculate texture pointer offset:
			int xidx = i % GLYPHS_PER_ROW;
			int yidx = (int)((float)i/(float)GLYPHS_PER_ROW);
			int offset = (padding_y + yidx*glyph_height)*rowstride +
						 (padding_x + xidx*glyph_width );

			unsigned char * image = (unsigned char *)(optr + offset);

			// write glyph bitmap into texture:
			FT_Bitmap *bitmap = &glyph->bitmap;
			if(antialias) {
				for(int j=0; j < int(bitmap->rows); j++) {
					unsigned char *pix = image + j*rowstride;
					unsigned char *font_pix = bitmap->buffer + j*bitmap->width;
					for(int k=0; k < int(bitmap->width); k++) {
						*pix++ = *font_pix++;
					}
				}
			}
			else{
				// Pixels are 1-bit each in bitmap
				for(int j=0; j < int(bitmap->rows); j++) {
					unsigned char *pix = image + j*rowstride;
					unsigned char *font_pix = bitmap->buffer + j*bitmap->pitch;
					for(int k=0; k < int(bitmap->width); k++) {
						int byteIdx = k/8; // byte index in bitmap
						int bitIdx  = k&7; // bit index in byte
						*pix++ = ((font_pix[byteIdx] >> (7-bitIdx)) & 1)*255;
					}
				}
			}
		}

		return true;
	}

protected:
	// factory pattern; use Impl::create()
	Impl(): mFace(0){}

	FT_Face mFace;
};


Font::Font()
:	mFontSize(12),
	mAntiAliased(true),
	mTex(0, 0, Graphics::LUMINANCE, Graphics::UBYTE)
{
	align(0,0);
	mImpl = Impl::create();
}


Font::Font(const std::string& filename, int fontSize, bool antialias)
:	mFontSize(fontSize),
	mAntiAliased(antialias),
	mTex(0, 0, Graphics::LUMINANCE, Graphics::UBYTE)
{
	align(0,0);
	mImpl = Impl::create();
	if(mImpl){
		if(!load(filename, fontSize, antialias)){
			delete mImpl;
			mImpl = nullptr;
		}
	}
}

Font::~Font() {
	if(mImpl) delete mImpl;
}

bool Font::load(const std::string& filename, int fontSize, bool antialias){
	if(mImpl->load(*this, filename.c_str(), fontSize, antialias)){
		mFontSize = fontSize;
		mAntiAliased = antialias;
		return true;
	}
	return false;
}

float Font::ascender() const { return mImpl->ascender(); }

float Font::descender() const { return mImpl->descender(); }

void Font::align(float xfrac, float yfrac){
	mAlign[0] = xfrac;
	mAlign[1] = yfrac;
}

void Font::write(Mesh& mesh, const std::string& text) {

	mesh.reset();
	mesh.primitive(Graphics::TRIANGLES);

	int nchars = text.size();
	float margin = 2.;
	float csz = (float)mFontSize;
	float cdim = csz+margin;
	float tdim = cdim*GLYPHS_PER_ROW;
	float tcdim = ((float)cdim)/((float)tdim);

	float pos[] = {0., ascender() * mAlign[1]};

	if(mAlign[0] != 0){
		pos[0] = -width(text) * mAlign[0];
	}

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

		float v_x0  = pos[0] + c.x_offset;
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

		int j = i*4;
		mesh.index(j,j+1,j+2, j+2,j+3,j);

		pos[0] += (float)c.width;
	}
}

void Font::render(Graphics& g, const std::string& text) {
	write(mMesh, text);
	mTex.bind();
	g.draw(mMesh);
	mTex.unbind();
}

void Font::renderf(Graphics& g, const char * fmt, ...) {
	static char line[1024];
	va_list args;
	va_start(args, fmt);
	AL_VSNPRINTF(line, 1024, fmt, args);
	va_end(args);
	render(g, line);
}

float Font::width(const std::string& text) const {
	float total = 0.f;
	for (unsigned i=0; i < text.size(); i++) {
		total += mChars[ (int)text[i] ].width;
	}
	return total;
}

} // al::
