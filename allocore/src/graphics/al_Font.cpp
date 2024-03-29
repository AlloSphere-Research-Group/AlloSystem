#include <cstdarg>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Font.hpp"

using namespace al;

//#define AL_FONT_FREETYPE

#ifdef AL_FONT_FREETYPE

/*
Dependency installation:
Linux: sudo apt-get install libfreetype6-dev
Mac / Homebrew: brew install freetype
Mac / MacPorts: sudo port install freetype
Win / MinGW-w64 / MSYS2: pacman -S freetype

Link with -lfreetype
*/

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H

struct Font::Impl {

	// FreeType tutorial: http://freetype.org/freetype2/docs/tutorial/step1.html

	static FT_Library& ftlib(){
		static FT_Library lib = 0;
		if(!lib){
			auto err = FT_Init_FreeType(&lib);
			if(err){
				AL_WARN_ONCE("Could not initialize FreeType library");
			}
		}
		return lib;
	}

	bool load(Font& font, const char * filename, int fontSize, bool antialias){

		FT_Face face;

		auto cleanup = [&](){
			FT_Done_Face(face);
		};

		auto err = FT_New_Face(ftlib(), filename, 0, &face);

		if(FT_Err_Unknown_File_Format == err){
			AL_WARN("Unsupported font %s", filename);
			return false;
		} else if(err){
			AL_WARN("Could not load font %s", filename);
			return false;
		}

		if(FT_Set_Pixel_Sizes(face, 0, fontSize)){
			AL_WARN("Could not set font size to %d", fontSize);
			cleanup();
			return false;
		}

		constexpr float fixed26_6toFloat = 1.f/64.f;

		// https://freetype.org/freetype2/docs/reference/ft2-sizing_and_scaling.html
		{
			const auto& faceMetrics = face->size->metrics;
			font.mAscender = faceMetrics.ascender * fixed26_6toFloat;
			font.mDescender = faceMetrics.descender * fixed26_6toFloat;

			//printf("asc:%g desc:%g\n", font.mAscender, font.mDescender);
			//printf("x_ppem:%u y_ppem:%u x_scale:%g y_scale:%g height:%g\n", faceMetrics.x_ppem, faceMetrics.y_ppem, faceMetrics.x_scale/65536., faceMetrics.y_scale/65536., faceMetrics.height/64.);
		}

		int padding = 1;
		int glyphsPerRow = 16;
		const int glyphW = fontSize + 2*padding;
		const int glyphH = fontSize + 2*padding;
		int texW = glyphW*glyphsPerRow;
		int texH = glyphH*(glyphCount/glyphsPerRow);

		font.mTex.resize(texW, texH).allocate();

		for(int i=0; i<glyphCount; ++i){

			const auto c = glyphBegin + i;

			// load glyph:
			int glyph_index = FT_Get_Char_Index(face, c);
			FT_GlyphSlot glyph = face->glyph;
			if(antialias){
				FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
				FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
			}
			else {
				FT_Load_Glyph(face, glyph_index, FT_LOAD_MONOCHROME);
				FT_Render_Glyph(glyph, FT_RENDER_MODE_MONO);
			}

			/*switch(glyph->format){
			case FT_GLYPH_FORMAT_BITMAP: printf("FT_GLYPH_FORMAT_BITMAP\n"); break;
			case FT_GLYPH_FORMAT_OUTLINE: printf("FT_GLYPH_FORMAT_OUTLINE\n"); break;
			case FT_GLYPH_FORMAT_COMPOSITE: printf("FT_GLYPH_FORMAT_COMPOSITE\n"); break;
			default:;
			}*/

			//if(i>='!' && i<='~')\
				printf("%c bitmap_left:%d bitmap_top:%d\n", i, glyph->bitmap_left, glyph->bitmap_top);

			// calculate texture pointer offset:
			int xidx = i % glyphsPerRow;
			int yidx = int(float(i)/glyphsPerRow);

			// FT glyph retrieval:
			// https://freetype.org/freetype2/docs/reference/ft2-glyph_retrieval.html
			// Notes: Some FT glyph pixel coordinates in 26.6 fixed-point. After much head scratching, I've given up trying to make the char map right-side up.
			{	auto& dst = font.glyphMetric(c);
				// These in pixels
				dst.xadvance = glyph->advance.x * fixed26_6toFloat;
				float width = glyph->metrics.width * fixed26_6toFloat;
				float height = glyph->metrics.height * fixed26_6toFloat;
				dst.xoffset = glyph->bitmap_left;
				dst.yoffset = glyph->bitmap_top - height - fontSize;
				// These in normalized coords [0,1]
				dst.u0 = float(padding + xidx*glyphW)/texW;
				dst.u1 = dst.u0 + width/texW;

				// v1 top of glyph as FT glyphs are upside-down!
				dst.v1 = float(padding + yidx*glyphH)/texH;
				dst.v0 = dst.v1 + height/texH;
			}

			auto& arr = font.mTex.array();
			const int rowstride = arr.header.stride[1];
			int offset = (padding + yidx*glyphH)*rowstride +
						 (padding + xidx*glyphW);
			auto * dstData = (unsigned char *)(arr.data.ptr + offset);

			// Copy glyph bitmap into texture
			// Note that in FreeType the top is row 0, so we flip y
			FT_Bitmap& srcData = glyph->bitmap;
			if(antialias){
				for(int j=0; j < int(srcData.rows); j++) {
					unsigned char * dst = dstData + j*rowstride;
					//unsigned char * src = srcData.buffer + (srcData.rows-1-j)*srcData.width;
					unsigned char * src = srcData.buffer + j*srcData.width;
					for(int k=0; k < int(srcData.width); k++) {
						*dst++ = *src++;
					}
				}
			} else {
				// Pixels are 1-bit each in bitmap
				for(int j=0; j < int(srcData.rows); j++){
					unsigned char * dst = dstData + j*rowstride;
					//unsigned char * src = srcData.buffer + (srcData.rows-1-j)*srcData.pitch;
					unsigned char * src = srcData.buffer + j*srcData.pitch;
					for(int k=0; k < int(srcData.width); k++){
						int byteIdx = k/8; // byte index in bitmap
						int bitIdx  = k&7; // bit index in byte
						*dst++ = ((src[byteIdx] >> (7-bitIdx)) & 1)*255;
					}
				}
			}
		}

		cleanup();
		return true;
	}
};
// end AL_FONT_FREETYPE

#else

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC // private to this file
#include "external/stb_truetype.h"
#include "allocore/io/al_File.hpp"

struct Font::Impl {

	bool load(Font& font, const char * filename, int fontSize, bool antialias){
		auto fontData = File::read(filename);
		static_assert(sizeof(decltype(fontData)::value_type)==1, "File data must be 1-byte type");
		if(fontData.empty()){
			AL_WARN("Could not load font face %s", filename);
			return false;
		}

		int glyphsPerRow = 16;
		int glyphW = fontSize;
		int glyphH = fontSize;
		int texW = glyphW * glyphsPerRow;
		int texH = glyphH * glyphCount/glyphsPerRow;

		// Guess to how efficiently STB packs glyphs in char map.
		// Increase or disable if glyphs do not fit.
		texH *= 0.7;

		font.mTex.resize(texW, texH).allocate();

		{
			stbtt_fontinfo f;
			stbtt_InitFont(&f, (unsigned char *)(fontData.data()), 0);
			float scale = stbtt_ScaleForPixelHeight(&f, fontSize);
			int asc, desc, lineGap;
			stbtt_GetFontVMetrics(&f, &asc, &desc, &lineGap);
			font.mAscender = std::floor(asc * scale + 0.5);
			font.mDescender = std::floor(desc * scale + 0.5);
			//printf("ascent:%d descent:%d numGlyphs:%d\n", asc, desc, f.numGlyphs);
		}

		/*
		typedef struct
		{
		   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
		   float xoff,yoff,xadvance;
		} stbtt_bakedchar;

		STBTT_DEF int stbtt_BakeFontBitmap(const unsigned char *data, int offset,  // font location (use offset=0 for plain .ttf)
										float pixel_height,                     // height of font in pixels
										unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
										int first_char, int num_chars,          // characters to bake
										stbtt_bakedchar *chardata);             // you allocate this, it's num_chars long
		// if return is positive, the first unused row of the bitmap
		// if return is negative, returns the negative of the number of characters that fit
		// if return is 0, no characters fit and no rows were used
		// This uses a very crappy packing.
		*/

		auto * charMetrics = new stbtt_bakedchar[glyphCount];

		auto res = stbtt_BakeFontBitmap(
			(unsigned char *)(fontData.data()),
			0, // font location (use offset=0 for plain .ttf)
			fontSize,
			font.mTex.data<unsigned char>(), texW, texH,
			glyphBegin, glyphCount,
			charMetrics
		);

		//printf("stbtt_BakeFontBitmap returned %d\n", res);

		// TODO: crop texture rows based on stbtt_BakeFontBitmap result?

		for(int i=0; i<glyphCount; ++i){
			const auto& src = charMetrics[i];
			auto& dst = font.glyphMetric(glyphBegin + i);
			dst.xadvance = src.xadvance;
			float height = src.y1 - src.y0;
			dst.xoffset = src.xoff;
			dst.yoffset = -src.yoff - height - fontSize;
			dst.u0 = float(src.x0)/texW;
			dst.u1 = float(src.x1)/texW;
			dst.v0 = float(src.y1)/texH;
			dst.v1 = float(src.y0)/texH;
		}

		delete[] charMetrics;

		return true;
	}
};

// AL_FONT_STB_TRUETYPE
#endif


#define TEX_FORMAT Graphics::LUMINANCE, Graphics::UBYTE

Font::Font()
:	mTex(0, 0, TEX_FORMAT)
{
}

Font::Font(const std::string& filename, int fontSize, bool antialias)
:	mFontSize(fontSize),
	mAntiAliased(antialias),
	mTex(0, 0, TEX_FORMAT)
{
	// Hmmm, no way to indicate error from constructor
	load(filename, fontSize, antialias);
}

bool Font::load(const std::string& filename, int fontSize, bool antialias){
	if(mImpl->load(*this, filename.c_str(), fontSize, antialias)){
		mFontSize = fontSize;
		mAntiAliased = antialias;
		return true;
	}
	return false;
}

Font::GlyphMetric& Font::glyphMetric(int c){
	// TODO: check index bounds?
	return mMetrics[c - glyphBegin];
}

const Font::GlyphMetric& Font::glyphMetric(int c) const {
	return const_cast<Font*>(this)->glyphMetric(c);
}

float Font::width(int c) const {
	return glyphMetric(c).xadvance;
}

Font& Font::align(float xfrac, float yfrac){
	mAlign[0] = xfrac;
	mAlign[1] = yfrac;
	return *this;
}

void Font::write(Mesh& mesh, const std::string& text){

	mesh.reset();
	mesh.primitive(Graphics::TRIANGLES);

	float xstart = 0.f, ystart = mFontSize - ascender();

	if(mAlign[0] != 0.f || mAlign[1] != 0.f){
		float w,h; bounds(w,h, text);
		xstart = -w * mAlign[0];
		ystart =  h * mAlign[1];
	}

	float pos[2] = {xstart, ystart};
	float dx = width(' '); // default x advance
	float dy = -mFontSize*mLineSpacing;
	float u2pix = mTex.width();
	float v2pix = mTex.height();

	// Using functions as positions must be integer to avoid blur
	auto setPos = [&](int i, float v){ pos[i] = std::floor(v+0.5); };
	auto incPos = [&](int i, float d){ setPos(i, pos[i] + d); };

	auto printable = [](int c){
		return (c >= '!' && c <= '~') || (c >= 128 && c <= 254);
	};

	for(int c : text){
		if(c >= glyphBegin && c < glyphEnd && printable(c)){

			auto& metric = glyphMetric(c);

			// tri-strip pattern: xy, Xy, xY, XY
			mesh.indexRel(0,1,2, 2,1,3);
			float w = std::abs(metric.u1 - metric.u0)*u2pix;
			float x = pos[0] + metric.xoffset;
			//float X = x + metric.width;
			float X = x + w;
			float h = std::abs(metric.v1 - metric.v0)*v2pix; // glyph height in pixels
			float y = pos[1] + metric.yoffset;
			float Y = y + h;
			mesh.vertex(x,y);
			mesh.vertex(X,y);
			mesh.vertex(x,Y);
			mesh.vertex(X,Y);
			mesh.texCoord(metric.u0, metric.v0);
			mesh.texCoord(metric.u1, metric.v0);
			mesh.texCoord(metric.u0, metric.v1);
			mesh.texCoord(metric.u1, metric.v1);

			incPos(0, metric.xadvance);

		} else if('\n' == c){
			setPos(0, xstart);
			incPos(1, dy);

		} else { // unknown/non-printable character, just advance along x
			incPos(0, dx);
		}
	}
}

void Font::render(Graphics& g, const std::string& text){
	write(mMesh, text);
	mTex.bind();
	g.draw(mMesh);
	mTex.unbind();
}

void Font::renderf(Graphics& g, const char * fmt, ...){
	static char line[1024];
	va_list args;
	va_start(args, fmt);
	AL_VSNPRINTF(line, sizeof line, fmt, args);
	va_end(args);
	render(g, line);
}

void Font::bounds(float& w, float& h, const std::string& text) const {
	if(text.empty()){
		w = h = 0.f;
		return;
	}
	w = 0.f;
	h = ascender();
	float wsum = 0.f;
	for(auto c : text){
		if('\n' == c){
			wsum = 0.f;
			h += mFontSize*mLineSpacing;
		} else {
			wsum += width(c);
			if(wsum > w) w = wsum;
		}
	}
}
