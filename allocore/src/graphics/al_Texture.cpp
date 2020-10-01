#include "allocore/system/al_Printing.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Texture.hpp"

namespace al{

void Texture::init(){
	wrap(Texture::CLAMP_TO_EDGE);
	filter(Texture::LINEAR);
}


Texture::Texture()
: 	mTarget(NO_TARGET),
	mFormat(Graphics::RGBA),
	mType(Graphics::UBYTE)
{
	init();
}

#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
Texture :: Texture(
	unsigned width,
	Graphics::Format format, Graphics::DataType type,
	bool alloc
)
:	mTarget(TEXTURE_1D),
	mFormat(format),
	mType(type),
	mWidth(width)
{
	init();
	if(alloc) allocate();
}
#endif

Texture :: Texture(
	unsigned width, unsigned height,
	Graphics::Format format, Graphics::DataType type,
	bool alloc
)
:	mTarget(TEXTURE_2D),
	mFormat(format),
	mType(type),
	mWidth(width),
	mHeight(height)
{
	init();
	if(alloc) allocate();
}

#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
Texture :: Texture(
	unsigned width, unsigned height, unsigned depth,
	Graphics::Format format, Graphics::DataType type,
	bool alloc
)
:	mTarget(TEXTURE_3D),
	mFormat(format),
	mType(type),
	mWidth(width),
	mHeight(height),
	mDepth(depth)
{
	init();
	if(alloc) allocate();
}
#endif

Texture :: Texture(AlloArrayHeader& header)
{
	init();
	shapeFrom(header, true /*reallocate*/);
}

Texture :: ~Texture() {
}


void Texture::onCreate(){
	//printf("Texture onCreate\n");
	glGenTextures(1, (GLuint *)&mID);

	if(tryBind()){
		sendShape();
		sendParams();
		sendPixels();
		glBindTexture(target(), 0);
	}
	AL_GRAPHICS_ERROR("creating texture", id());
}

void Texture::onDestroy(){
	glDeleteTextures(1, (GLuint *)&mID);
}


Texture& Texture::filterMin(Filter v){
	switch(v){
	case NEAREST_MIPMAP_NEAREST:
	case LINEAR_MIPMAP_NEAREST:
	case NEAREST_MIPMAP_LINEAR:
	case LINEAR_MIPMAP_LINEAR:
		mMipmap = true;
		break;
	default:
		mMipmap = false;
	}
	return update(v, mFilterMin, mParamsUpdated);
}

Texture& Texture::filterMag(Filter v){
	// Mip-map not sensible for mag filter, so set to best match
	switch(v){
	case NEAREST_MIPMAP_NEAREST:
	case NEAREST_MIPMAP_LINEAR:
		v = NEAREST;
		break;
	case LINEAR_MIPMAP_NEAREST:
	case LINEAR_MIPMAP_LINEAR:
		v = LINEAR;
		break;
	default:;
	}
	return update(v, mFilterMag, mParamsUpdated);
}

Texture& Texture::wrap(Wrap S, Wrap T, Wrap R){
	if(S!=mWrapS || T!=mWrapT || R!=mWrapR){
		mWrapS = S; mWrapT = T; mWrapR = R;
		mParamsUpdated = true;
	}
	return *this;
}


void Texture::shapeFrom(const AlloArrayHeader& hdr, bool realloc){
	switch(hdr.dimcount){
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		case 1: target(TEXTURE_1D); break;
		#endif
		case 2: target(TEXTURE_2D); break;
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		case 3: target(TEXTURE_3D); break;
		#endif
		default:
			AL_WARN("invalid array dimensions for texture");
			return;
	}

	switch(hdr.dimcount){
		case 3:	 depth(hdr.dim[2]);
		case 2:	height(hdr.dim[1]);
		case 1:	 width(hdr.dim[0]); break;
		default:
			AL_WARN("texture array must have 1, 2 or 3 dimensions");
			return;
	}

	// Set pixel format only if number of components differs.
	// No need to change semantics of data.
	if(Graphics::numComponents(mFormat) != hdr.components){
		switch(hdr.components){
			case 1:	format(Graphics::LUMINANCE); break; // alpha or luminance?
			case 2:	format(Graphics::LUMINANCE_ALPHA); break;
			case 3:	format(Graphics::RGB); break;
			case 4:	format(Graphics::RGBA); break;
			default:
				AL_WARN("invalid array component count for texture");
				return;
		}
	}

	switch(hdr.type){
		case AlloUInt8Ty:	type(Graphics::UBYTE); break;
		case AlloSInt8Ty:	type(Graphics::BYTE); break;
		case AlloUInt16Ty:	type(Graphics::SHORT); break;
		case AlloSInt16Ty:	type(Graphics::USHORT); break;
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		case AlloUInt32Ty:	type(Graphics::INT); break;
		case AlloSInt32Ty:	type(Graphics::UINT); break;
		#endif
		case AlloFloat32Ty:	type(Graphics::FLOAT); break;
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		case AlloFloat64Ty:	type(Graphics::DOUBLE); break;
		#endif
		default:
			AL_WARN("invalid array type for texture");
			return;
	}

	// Reconfigure internal array AND resize its memory (if necessary).
	if(realloc){
		mArray.format(hdr);
	}

	// Reconfigure internal array WITHOUT resizing its memory.
	else{
		mArray.configure(hdr);
	}
}

void Texture::shapeFromArray(){
	// If someone requested a mutable reference to the internal array, we will
	// assume we need to sync the texture attributes with the array.
	if(mArrayDirty){
		// ensure texture attributes match internal array
		shapeFrom(mArray.header, false);
		// force texture to be submitted since the pixel data could have been
		// tampered with
		dirty(); // mPixelsUpdated=true;
		mArrayDirty = false;
	}
}

void Texture::deriveTarget(){
	if(mDepth != 0){
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		target(TEXTURE_3D);
		#endif
	}
	else if(mHeight != 0){
		target(TEXTURE_2D);
	}
	else if(mWidth != 0){
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		target(TEXTURE_1D);
		#endif
	}
	else{
		target(NO_TARGET);
	}
}

bool Texture::tryBind(){

	// Sync shape if array is dirty
	shapeFromArray();

	// Ensure target is synchronized before bind
	deriveTarget();

	if(target() != Texture::NO_TARGET){
		glBindTexture(target(), id());
		return true;
	}
	return false;
}

void Texture :: bind(int unit) {
	AL_GRAPHICS_ERROR("(before Texture::bind)", id());

	// Ensure texture is created
	validate();
		AL_GRAPHICS_ERROR("validate binding texture", id());

	// Specify multitexturing
	glActiveTexture(GL_TEXTURE0 + unit);
		AL_GRAPHICS_ERROR("active texture binding texture", id());

	// Do the actual binding
	if(tryBind()){
		AL_GRAPHICS_ERROR("binding texture", id());

		#ifdef AL_GRAPHICS_TEXTURE_NEEDS_ENABLE
		glEnable(target());
			AL_GRAPHICS_ERROR("enable target binding texture", id());
		#endif

		shapeFromArray();

		// Synchronize client texture state with GPU
		sendShape(false);

		sendParams(false);
			//AL_GRAPHICS_ERROR("sendparams binding texture", id());
		sendPixels(false);
			//AL_GRAPHICS_ERROR("sendpixels binding texture", id());
	}
}

void Texture :: unbind(int unit) {
	// multitexturing:
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target(), 0);
	#ifdef AL_GRAPHICS_TEXTURE_NEEDS_ENABLE
	glDisable(target());
	#endif
}

void Texture :: resetArray(unsigned align) {
	//printf("resetArray %p\n", this);
	deallocate();

	// reconfigure the internal array according to the current settings:
	switch (mTarget) {
		case TEXTURE_2D:
		default:
			mArray.header.type = Graphics::toAlloTy(mType);
			mArray.header.components = Graphics::numComponents(mFormat);
			mArray.header.dimcount = 2;
			mArray.header.dim[0] = mWidth;
			mArray.header.dim[1] = mHeight;
			mArray.deriveStride(mArray.header, align);
			break;

		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		case TEXTURE_1D:
			mArray.header.type = Graphics::toAlloTy(mType);
			mArray.header.components = Graphics::numComponents(mFormat);
			mArray.header.dimcount = 1;
			mArray.header.dim[0] = mWidth;
			mArray.deriveStride(mArray.header, align);
			break;
		#endif

		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		case TEXTURE_3D:
			mArray.header.type = Graphics::toAlloTy(mType);
			mArray.header.components = Graphics::numComponents(mFormat);
			mArray.header.dimcount = 3;
			mArray.header.dim[0] = mWidth;
			mArray.header.dim[1] = mHeight;
			mArray.header.dim[2] = mDepth;
			mArray.deriveStride(mArray.header, align);
			break;
		#endif
	}
}

void Texture :: allocate(unsigned align) {
	deallocate();
	resetArray(align);
	mArray.dataCalloc();
	mPixelsUpdated = true;
}

void Texture :: allocate(const Array& src, bool reconfigure) {

	// Here we reconfigure the internal Array to match the passed in Array
	if (reconfigure) {
		//printf("allocating & reconfiguring %p from\n", this); src.print();
		shapeFrom(src.header, true /*reallocate*/);

		//printf("allocating & reconfigured %p\n", this); mArray.print();

		// FIXME:
		// re-allocate array:
		//allocate(src.alignment());
		/*
		Texture::allocate does this:
			mArray.dataFree();
			resetArray(align);
			mArray.dataCalloc();
			mPixelsUpdated = true;
		*/

		//printf("allocated & reconfigured %p\n", this);
		//mArray.print();

	// Here we ???
	} else {

		// TODO: read the source into the dst without changing dst layout
		//printf("allocating without reconfiguring %p\n", this);

		// ensure that array matches texture:
		if (!src.isFormat(mArray.header)) {
			AL_WARN("couldn't allocate array, mismatch format");
			mArray.print();
			src.print();
			return;
		}

		// re-allocate array:
		allocate();
	}

	//src.print();
	//mArray.print();

	// Perform a deep copy of data:
	// The Array formats must match for this to make sense!
	memcpy(mArray.data.ptr, src.data.ptr, src.size());

	//printf("copied to mArray %p\n", this);
}

void Texture :: deallocate() {
	mArray.dataFree();
}

void Texture::sendParams(bool force){
	if(mParamsUpdated || force){
		glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, filterMag());
		glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, filterMin());
		glTexParameteri(target(), GL_TEXTURE_WRAP_S, mWrapS);
		glTexParameteri(target(), GL_TEXTURE_WRAP_T, mWrapT);
		#ifdef GL_TEXTURE_WRAP_R
		glTexParameteri(target(), GL_TEXTURE_WRAP_R, mWrapR);
		#endif
		/*if (filterMin() != LINEAR && filterMin() != NEAREST) {
			// deprecated in OpenGL 3.0 and above
			glTexParameteri(target(), GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		}*/
			AL_GRAPHICS_ERROR("Texture::sendParams (glTexParameteri)", id());
		mParamsUpdated = false;
	}
}

void Texture::sendPixels(const void * pixels, unsigned align){
	//printf("Texture::sendPixels:"); print();
	// Note: pixels cannot be NULL for TexSubImage
	if(pixels){

		// Tell GPU the row alignment of the pixels we are sending
		glPixelStorei(GL_UNPACK_ALIGNMENT, align);
			AL_GRAPHICS_ERROR("Texture::sendPixels (glPixelStorei set)", id());

		auto genMipmap = [this](){
			#ifdef AL_GRAPHICS_SUPPORTS_MIPMAP
			if(mMipmap){
				glGenerateMipmap(target());
					AL_GRAPHICS_ERROR("Texture::sendPixels (glGenerateMipmap)", id());
			}
			#endif
		};

		switch(target()){

			/*void glTexSubImage3D(
				GLenum target, GLint level,
				GLint xoffset, GLint yoffset, GLint zoffset,
				GLsizei width, GLsizei height, GLsizei depth,
				GLenum format, GLenum type,
				const GLvoid *pixels
			);*/

			#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
			case TEXTURE_1D:
				glTexSubImage1D(target(), 0, 0, width(), format(), type(), pixels);
					AL_GRAPHICS_ERROR("Texture::sendPixels (glTexSubImage)", id());
				genMipmap();
				break;
			#endif

			case TEXTURE_2D:
				glTexSubImage2D(target(), 0, 0,0, width(), height(), format(), type(), pixels);
					AL_GRAPHICS_ERROR("Texture::sendPixels (glTexSubImage)", id());
				genMipmap();
				break;

			#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
			case TEXTURE_3D:
				glTexSubImage3D(target(), 0, 0,0,0, width(), height(), depth(), format(), type(), pixels);
					AL_GRAPHICS_ERROR("Texture::sendPixels (glTexSubImage)", id());
				genMipmap();
				break;
			#endif

			case NO_TARGET:;
				// Unconfigured
			default:
				AL_WARN("invalid texture target %d", target());
		}

		// Set alignment back to default
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			AL_GRAPHICS_ERROR("Texture::sendPixels (glPixelStorei unset)", id());
	}
}

void Texture::sendPixels(bool force){
	if(mPixelsUpdated || force){
		//printf("%p submitting %p\n", this, mArray.data.ptr);
		//submit(mArray.data.ptr, mArray.alignment());
		sendPixels(mArray.data.ptr, mArray.alignment());
		mPixelsUpdated = false;
	}
}

void Texture::sendShape(bool force){
	if((mShapeUpdated || force)){

		// Determine texel format (on GPU)
		// Use specified texel format if defined, otherwise pixel format
		int intFmt = mTexelFormat ? mTexelFormat : format();

		//printf("Texture::sendShape calling glTexImage\n");
		switch(target()){
		/*void glTexImage3D(
			GLenum target, GLint level, GLenum internalformat,
			GLsizei width, GLsizei height, GLsizei depth,
			GLint border, GLenum format, GLenum type, const GLvoid *pixels);*/
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		case TEXTURE_1D:
			glTexImage1D(target(), 0, intFmt, width(), 0, format(), type(), NULL);
			break;
		#endif

		case TEXTURE_2D:
			glTexImage2D(target(), 0, intFmt, width(), height(), 0, format(), type(), NULL);
			//printf("glTexImage2D(%s, 0, %s, %u, %u, 0, %s, %s, NULL)\n", toString(target()), toString(Graphics::Format(intFmt)), width(), height(), toString(format()), toString(type()));
			break;

		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		case TEXTURE_3D:
			glTexImage3D(target(), 0, intFmt, width(), height(), depth(), 0, format(), type(), NULL);
			break;
		#endif

		case NO_TARGET:;
			// Unconfigured
		default:;
		}
		AL_GRAPHICS_ERROR("Texture::sendShape (glTexImage)", id());
		mShapeUpdated = false;
	}
}


void Texture :: submit(const void * pixels, uint32_t align) {
	AL_GRAPHICS_ERROR("(before Texture::submit)", id());

	// This ensures that the texture is created on the GPU
	GPUObject::validate();

	glActiveTexture(GL_TEXTURE0);
		AL_GRAPHICS_ERROR("Texture::submit (glActiveTexture)", id());

	if(tryBind()){
		AL_GRAPHICS_ERROR("Texture::submit (glBindTexture)", id());

		// Sync client state with GPU
		sendShape(false);
		sendParams(false);

		// Calls glTexSubImage
		sendPixels(pixels, align);

		/* OpenGL may have changed the internal format to one it supports:
		GLint format;
		glGetTexLevelParameteriv(mTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		if (format != mInternalFormat) {
			printf("converted from %X to %X format\n", mInternalFormat, format);
			mInternalFormat = format;
		}
		//*/

		//printf("submitted texture data %p\n", pixels);

		// Unbind texture
		glBindTexture(target(), 0);
			AL_GRAPHICS_ERROR("Texture::submit (glBindTexture 0)", id());
	}
}


void Texture :: submit(const Array& src, bool reconfigure) {

	// Here we basically do a deep copy of the passed in Array
	if (reconfigure) {
		shapeFrom(src.header, true /*reallocate*/);
		//printf("configured to target=%X(%dD), type=%X(%X), format=%X, align=(%d)\n", mTarget, src.dimcount(), type(), src.type(), mFormat, src.alignment());
	}

	// In this case, we simply do some sanity checks to make sure the passed
	// in Array is compatible with our currently set texture parameters
	else {
		if (src.width() != width()) {
			AL_WARN("submit failed: source array width does not match");
			return;
		}
		if (height() && src.height() != height()) {
			AL_WARN("submit failed: source array height does not match");
			return;
		}
		if (depth() && src.depth() != depth()) {
			AL_WARN("submit failed: source array depth does not match");
			return;
		}

		if (Graphics::toDataType(src.type()) != type()) {
			AL_WARN("submit failed: source array type does not match texture");
			return;
		}

		switch (format()) {
			case Graphics::LUMINANCE:
			#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
			case Graphics::DEPTH_COMPONENT:
			#endif
			case Graphics::ALPHA:
				if (src.components() != 1) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 1)", src.components());
					return;
				}
				break;
			case Graphics::LUMINANCE_ALPHA:
				if (src.components() != 2) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 2)", src.components());
					return;
				}
				break;
			case Graphics::RGB:
				if (src.components() != 3) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 3)", src.components());
					return;
				}
				break;
			case Graphics::RGBA:
				if (src.components() != 4) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 4)", src.components());
					return;
				}
				break;
			default:;
		}
	}

	submit(src.data.ptr, src.alignment());
}

void Texture::submit(){
	// Note: not using array() as it may flag the array as dirty
	submit(mArray.data.ptr, mArray.alignment());
}

void Texture::copyFrameBuffer(
	int w, int h, int fbx, int fby, int texx, int texy, int texz
){
	if(w < 0){
		w += 1 + width();
	}
	if(h < 0){
		h += 1 + height();
	}

	bind();
	switch(target()){
	#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
	case TEXTURE_1D:
		glCopyTexSubImage1D(GL_TEXTURE_1D, 0, texx, fbx,fby, w);
		break;
	#endif
	case TEXTURE_2D:
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, texx,texy, fbx,fby, w, h);
		break;
	#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
	case TEXTURE_3D:
		glCopyTexSubImage3D(GL_TEXTURE_3D, 0, texx,texy,texz, fbx,fby, w, h);
		break;
	#endif
	default:;
	}
	unbind();
}

void Texture :: quad(Graphics& gl, double w, double h, double x, double y, double z){
	//Graphics::error(id(), "prebind quad texture");
	bind();
	Mesh& m = gl.mesh();
	m.reset();
	//Graphics::error(id(), "reset mesh quad texture");
	m.primitive(gl.TRIANGLE_STRIP);
		m.texCoord	(0, 0);
		m.vertex	(x, y, z);
		m.texCoord	(1, 0);
		m.vertex	(x+w, y, z);
		m.texCoord	(0, 1);
		m.vertex	(x, y+h, z);
		m.texCoord	(1, 1);
		m.vertex	(x+w, y+h, z);
	//Graphics::error(id(), "set mesh quad texture");
	gl.draw(m);
	//Graphics::error(id(), "draw mesh quad texture");
	unbind();
}

void Texture::quadViewport(
	Graphics& g, const Color& color,
	double w, double h, double x, double y, double z
){
	g.pushMatrix(g.PROJECTION);
	g.loadIdentity();
	g.pushMatrix(g.MODELVIEW);
	g.loadIdentity();
	g.depthMask(false); // write only to color buffer
		g.color(color);
		quad(g, w,h, x,y,z);
	g.depthMask(true);
	g.popMatrix(g.PROJECTION);
	g.popMatrix(g.MODELVIEW);
}

void Texture::assign(const std::function<void(int i, int j, float * rgba)>& onPixel){
	if(mTarget == TEXTURE_2D){
		for(int j=0; j<height(); ++j){
			for(int i=0; i<width(); ++i){
				float rgba[4] = {0.f, 0.f, 0.f, 1.f};
				onPixel(i,j, rgba);
				#define SET_PIXEL(type, op){\
					auto * pixel = data<type>() + (j*height()+i)*numComponents();\
					switch(mFormat){\
					case Graphics::RGBA:\
						pixel[0] = rgba[0] op;\
						pixel[1] = rgba[1] op;\
						pixel[2] = rgba[2] op;\
						pixel[3] = rgba[3] op;\
						break;\
					case Graphics::RGB:\
						pixel[0] = rgba[0] op;\
						pixel[1] = rgba[1] op;\
						pixel[2] = rgba[2] op;\
						break;\
					case Graphics::LUMINANCE_ALPHA:\
						pixel[0] = rgba[0] op;\
						pixel[1] = rgba[3] op;\
						break;\
					case Graphics::LUMINANCE:\
						pixel[0] = rgba[0] op;\
						break;\
					case Graphics::ALPHA:\
						pixel[0] = rgba[3] op;\
						break;\
					default:;\
					}\
				}
				switch(mType){
				case Graphics::UBYTE:  SET_PIXEL(uint8_t,  *  255.99); break;
				case Graphics:: BYTE:  SET_PIXEL( int8_t,  *  127.99 + 128.); break;
				case Graphics::USHORT: SET_PIXEL(uint16_t, *65535.99); break;
				case Graphics:: SHORT: SET_PIXEL( int16_t, *32767.99 + 32768.); break;
				case Graphics::FLOAT:  SET_PIXEL(float,             ); break;
				default:;
				}
			}
		}
	}
}

void Texture::assignFromTexCoord(const std::function<void(float s, float t, float * rgba)>& onPixel){
	const float rw = 1.f/(width()-1);
	const float rh = 1.f/(height()-1);
	assign([rw,rh, &onPixel](int i, int j, float * rgba){
		onPixel(float(i)*rw, float(j)*rh, rgba);
	});
}

void Texture :: print() {

	printf("Texture ");

	switch (mTarget) {
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		case TEXTURE_1D:
			printf("target=%s, %d(%d)", toString(mTarget), width(), mArray.width());
			break;
		#endif

		case TEXTURE_2D:
			printf("target=%s, %dx%d(%dx%d)", toString(mTarget), width(), height(), mArray.width(), mArray.height());
			break;

		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		case TEXTURE_3D:
			printf("target=%s, %dx%dx%d(%dx%dx%d)", toString(mTarget), width(), height(), depth(), mArray.width(), mArray.height(), mArray.depth());
			break;
		#endif

		case NO_TARGET:
			printf("target=%s, %dx%dx%d(%dx%dx%d)", toString(mTarget), width(), height(), depth(), mArray.width(), mArray.height(), mArray.depth());
			break;

		default:
			printf("target=(unknown)");
	}

	printf(", type=%s(%s), format=%s(%d), unpack=%d\n",
		toString(mType), allo_type_name(mArray.type()), toString(mFormat), mArray.components(), mArray.alignment()
	);
	//mArray.print();
}


void Texture :: configure(AlloArrayHeader& hdr) {
	AL_WARN_ONCE("Texture::configure() deprecated, use Texture::shapeFrom()");
	shapeFrom(hdr, false);
}

Texture& Texture::updatePixels(){
	AL_WARN_ONCE("Texture::updatePixels() deprecated, use Texture::dirty()");
	return dirty();
}


#define CS(t) case Texture::t: return #t;
const char * toString(Texture::Target v){
	switch(v){
		CS(TEXTURE_2D)
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		CS(TEXTURE_1D)
		#endif
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		CS(TEXTURE_3D)
		#endif
		CS(NO_TARGET)
		default: return "";
	}
}
const char * toString(Texture::Wrap v){
	switch(v){
		CS(CLAMP_TO_EDGE) CS(REPEAT)
		#ifdef AL_GRAPHICS_SUPPORTS_WRAP_EXTRA
		CS(CLAMP) CS(CLAMP_TO_BORDER) CS(MIRRORED_REPEAT)
		#endif
		default: return "";
	}
}
const char * toString(Texture::Filter v){
	switch(v){
		CS(NEAREST) CS(LINEAR)
		CS(NEAREST_MIPMAP_NEAREST) CS(LINEAR_MIPMAP_NEAREST)
		CS(NEAREST_MIPMAP_LINEAR) CS(LINEAR_MIPMAP_LINEAR)
		default: return "";
	}
}

} // al::
