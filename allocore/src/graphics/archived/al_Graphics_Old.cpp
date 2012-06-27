#include <vector>
#include <map>
#include <string>

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GraphicsOpenGL.hpp"

namespace al{

void GraphicsData::resetBuffers() {
	vertices().clear();
	normals().clear();
	colors().clear();
	texCoord2s().clear();
	texCoord3s().clear();
	indices().clear();
}

void GraphicsData::equalizeBuffers() {
	int VS = vertices().size();
	int NS = normals().size();
	int CS = colors().size();
	int T2S = texCoord2s().size();
	int T3S = texCoord3s().size();

	if(NS > 0) {
		for(int i=NS; i < VS; i++) {
			normals().append(normals()[NS-1]);
		}
	}
	if(CS > 0) {
		for(int i=CS; i < VS; i++) {
			colors().append(colors()[CS-1]);
		}
	}
	if(T2S > 0) {
		for(int i=T2S; i < VS; i++) {
			texCoord2s().append(texCoord2s()[T2S-1]);
		}
	}
	if(T3S > 0) {
		for(int i=T3S; i < VS; i++) {
			texCoord3s().append(texCoord3s()[T3S-1]);
		}
	}
}

class TriFace {
public:
	GraphicsData::Vertex vertices[3];
	Vec3f norm;

	TriFace(const TriFace& cpy)
	: norm(cpy.norm) {
		vertices[0] = cpy.vertices[0];
		vertices[1] = cpy.vertices[1];
		vertices[2] = cpy.vertices[2];
	}
	TriFace(GraphicsData::Vertex p0, GraphicsData::Vertex p1, GraphicsData::Vertex p2)
	{
		vertices[0] = p0;
		vertices[1] = p1;
		vertices[2] = p2;
		// calculate norm for this face:
		normal<float>(norm, p0, p1, p2);
	}
};

void GraphicsData::generateNormals(float angle) {
//	/*
//		Multi-pass algorithm:
//			generate a list of faces (assume triangles?)
//				(vary according to whether mIndices is used)
//			calculate normal per face (use normal<float>(dst, p0, p1, p2))
//			vertices may be used in multiple faces; their norm should be an average of the uses
//				easy enough if indices is being used; not so easy otherwise.
//					create a lookup table by hashing on vertex x,y,z
//
//
//			write avg into corresponding normals for each vertex
//				EXCEPT: if edge is sharper than @angle, just use the face normal directly
//	*/
//	std::vector<TriFace> faces;
//
//	std::map<std::string, int> vertexHash;
//
//	int Ni = indices().size();
//	int Nv = vertices().size();
//	if (Ni) {
//		for (int i=0; i<Ni;) {
//			TriFace face(
//				mVertices[mIndices[i++]],
//				mVertices[mIndices[i++]],
//				mVertices[mIndices[i++]]
//			);
//			faces.push_back(face);
//		}
//	} else {
//		for (int i=0; i<Nv;) {
//			TriFace face(
//				mVertices[i++],
//				mVertices[i++],
//				mVertices[i++]
//			);
//			faces.push_back(face);
//		}
//	}
}

void GraphicsData::getBounds(Vertex& min, Vertex& max) {
	if (mVertices.size() > 0) {
		min.set(mVertices[0][0], mVertices[0][1], mVertices[0][2]);
		min.set(mVertices[0][0], mVertices[0][1], mVertices[0][2]);
		for (int v=1; v<mVertices.size(); v++) {
			Vertex& vt = mVertices[v];
			for (int i=0; i<3; i++) {
				min[i] = MIN(min[i], vt[i]);
				max[i] = MAX(max[i], vt[i]);
			}
		}
	}
}

void GraphicsData::unitize() {
	Vertex min(0), max(0);
	getBounds(min, max);
	Vertex avg = (max-min)*0.5;
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		for (int i=0; i<3; i++) {
			vt[i] = -1. + (vt[i]-min[i])/avg[i];
		}
	}
}

Vec3f GraphicsData::getCenter() {
	Vertex min(0), max(0);
	getBounds(min, max);
	return min+(max-min)*0.5;
}

void GraphicsData::translate(double x, double y, double z) {
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		vt[0] += x;
		vt[1] += y;
		vt[2] += z;
	}
}

void GraphicsData::scale(double x, double y, double z) {
	for (int v=0; v<mVertices.size(); v++) {
		Vertex& vt = mVertices[v];
		vt[0] *= x;
		vt[1] *= y;
		vt[2] *= z;
	}
}


//==============================================================================

Graphics::Graphics(GraphicsBackend *backend)
:	mBackend(backend),
	mInImmediateMode(false),
	mMatrixMode(MODELVIEW)
{
	mProjectionMatrix.push(Matrix4d::identity());
	mModelViewMatrix.push(Matrix4d::identity());
	State nullstate;
	nullstate.depth_enable = false;
	mState.push(nullstate);
}

Graphics::~Graphics() {
}

// Rendering State

void Graphics::blending(bool v, BlendFunc src, BlendFunc dst){ backend()->enableBlending(v, src,dst); }

void Graphics::depthMask(bool v) { backend()->enableDepthMask(v); }

void Graphics::depthTesting(bool v){ backend()->enableDepthTesting(v); }

void Graphics::lighting(bool v){ backend()->enableLighting(v); }

void Graphics::scissor(bool v){ backend()->enableScissor(v); }

void Graphics::pushState(State &state) {
	compareState(mState.top(), state);
	mState.push(state);
}

void Graphics::popState() {
	State &prev_state = mState.top();
	mState.pop();
	compareState(prev_state, mState.top());
}

void Graphics::compareState(const State &prev_state, const State &state) {
	// blending
	if(	prev_state.blend_enable == state.blend_enable &&
		prev_state.blend_src == state.blend_src &&
		prev_state.blend_dst == state.blend_dst)
	{
		mStateChange.blending = false;
	}

	// lighting
	if(prev_state.lighting_enable != state.lighting_enable) {
		mStateChange.lighting = true;
	}

	// depth testing
	if(prev_state.depth_enable != state.depth_enable) {
		mStateChange.depth_testing = true;
	}

	// polygon mode
	if(prev_state.polygon_mode != state.polygon_mode) {
		mStateChange.polygon_mode = true;
	}

	// anti-aliasing
	if(prev_state.antialias_mode != state.antialias_mode) {
		mStateChange.antialiasing = true;
	}

}


// Frame
void Graphics::clear(int attribMask) {
	mBackend->clear(attribMask);
}

void Graphics::clearColor(float r, float g, float b, float a) {
	mBackend->clearColor(r, g, b, a);
}


// Coordinate Transforms
void Graphics::viewport(int x, int y, int width, int height) {
	mBackend->viewport(x, y, width, height);
}

void Graphics::matrixMode(MatrixMode mode) {
	mMatrixMode = mode;
}

Graphics::MatrixMode Graphics::matrixMode() {
	return mMatrixMode;
}

stack<Matrix4d> & Graphics::matrixStackForMode(MatrixMode mode) {
	if(mode == PROJECTION) {
		return mProjectionMatrix;
	}
	else {
		return mModelViewMatrix;
	}
}

void Graphics::pushMatrix() {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.push(matrix_stack.top());
}

void Graphics::popMatrix() {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.pop();
}

void Graphics::loadIdentity() {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.top() = Matrix4d::identity();
}

void Graphics::loadMatrix(const Matrix4d &m) {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.top() = m;
}

void Graphics::multMatrix(const Matrix4d &m) {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.top() *= m;
}

void Graphics::translate(double x, double y, double z) {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.top() *= Matrix4d::translate(x, y, z);
}

void Graphics::rotate(double angle, double x, double y, double z) {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.top() *= Matrix4d::rotate(angle, Vec3d(x, y, z)); // * matrix_stack.top();
}

void Graphics::scale(double x, double y, double z) {
	stack<Matrix4d> &matrix_stack = matrixStackForMode(mMatrixMode);
	matrix_stack.top() *= Matrix4d::scale(x, y, z); // * matrix_stack.top();
}

// Immediate Mode
void Graphics::begin(Primitive mode) {
	// clear buffers
	mGraphicsData.resetBuffers();

	// start primitive drawing
	mGraphicsData.primitive(mode);
	mInImmediateMode = true;
}

void Graphics::end() {
	draw(mGraphicsData);
	mInImmediateMode = false;
}

void Graphics::vertex(double x, double y, double z) {
	if(mInImmediateMode) {
		// make sure all buffers are the same size if > 0
		mGraphicsData.addVertex(x, y, z);
		mGraphicsData.equalizeBuffers();
	}
}

void Graphics::texcoord(double u, double v) {
	if(mInImmediateMode) {
		mGraphicsData.addTexCoord(u, v);
	}
}

void Graphics::normal(double x, double y, double z) {
	if(mInImmediateMode) {
		mGraphicsData.addNormal(x, y, z);
	}
}

void Graphics::color(double r, double g, double b, double a) {
	if(mInImmediateMode) {
		mGraphicsData.addColor(r, g, b, a);
	}
	else {
		mBackend->color(Color(r, g, b, a));
	}
}

void Graphics::enableState() {
	State &state = mState.top();

	if(mStateChange.blending) {
		mBackend->enableBlending(
			state.blend_enable,
			state.blend_src,
			state.blend_dst
		);
		mStateChange.blending = false;
	}

	if(mStateChange.lighting) {
		mBackend->enableLighting(
			state.lighting_enable
		);
		mStateChange.lighting = false;
	}

	if(mStateChange.depth_testing) {
		mBackend->enableDepthTesting(
			state.depth_enable
		);
		mStateChange.depth_testing = false;
	}

	if(mStateChange.polygon_mode) {
		mBackend->setPolygonMode(
			state.polygon_mode
		);
		mStateChange.polygon_mode = false;
	}

	if(mStateChange.antialiasing) {
		mBackend->setAntialiasing(state.antialias_mode);
		mStateChange.antialiasing = false;
	}
}

void Graphics::antialiasing(AntiAliasMode v){
	backend()->setAntialiasing(v);
}

void Graphics::lineWidth(double v) {
	mBackend->lineWidth(v);
}

void Graphics::pointSize(double v) {
	mBackend->pointSize(v);
}


void Graphics::drawBegin() {
// Draw Begin (generic)
//		update state?
	enableState();

	mBackend->setProjectionMatrix(mProjectionMatrix.top());
	mBackend->setModelviewMatrix(mModelViewMatrix.top());


//		update matrices?
//		update textures?
/*
	vector<Texture *>::iterator tex_it = mTextures.begin();
	vector<Texture *>::iterator tex_ite = mTextures.end();
	for(int i=0; tex_it != tex_ite; ++tex_it, ++i) {
		(*tex_it)->bind(i);
	}
*/
//		update material?
}

void Graphics::drawEnd() {
	// Draw End (generic)
//		unbind materials
//		unbind used textures
/*
	vector<Texture *>::iterator tex_it = mTextures.begin();
	vector<Texture *>::iterator tex_ite = mTextures.end();
	for(int i=0; tex_it != tex_ite; ++tex_it, ++i) {
		(*tex_it)->unbind(i);
	}
*/
}

// Buffer drawing
void Graphics::draw(const GraphicsData& v) {
	drawBegin();
		mBackend->draw(v);
	drawEnd();
}

void Graphics::draw() {
	draw(mGraphicsData);
}





/*
static void gl_draw(const GraphicsData& v){}

static void gl_clear(int attribMask){}
static void gl_clearColor(float r, float g, float b, float a){}
static void gl_loadIdentity(){}
static void gl_viewport(int x, int y, int w, int h){}

static void gl_begin(Graphics * g, int mode) {}
static void gl_end(Graphics * g) {}
static void gl_vertex(Graphics * g, double x, double y, double z) {}
static void gl_normal(Graphics * g, double x, double y, double z) {}
static void gl_color(Graphics * g, double x, double y, double z, double a) {}



bool setBackendNone(Graphics * g) {
	g->s_draw = gl_draw;
	g->s_clear = gl_clear;
	g->s_clearColor = gl_clearColor;
	g->s_loadIdentity = gl_loadIdentity;
	g->s_viewport = gl_viewport;

	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex = gl_vertex;
	g->s_normal = gl_normal;
	g->s_color = gl_color;

	g->mBackend = Backend::None;

	return true;
}


bool setBackendOpenGLES(Graphics * g) {
	if (setBackendOpenGLES2(g)) {
		return true;
	} else if (setBackendOpenGLES1(g)) {
		return true;
	} else {
		return false;
	}
}

Graphics :: Graphics(Backend::type backend) {
	setBackend(backend);
}


Graphics :: ~Graphics() {

}

bool Graphics :: setBackend(Backend::type backend) {
	if (backend == Backend::AutoDetect) {
		if (setBackendOpenGLES(this)) {
			return true;
		} else if (setBackendOpenGL(this)) {
			return true;
		} else {
			setBackendNone(this);
			return false;
		}
	}

	// set function pointers according to backend
	switch (backend) {
		case Backend::OpenGLES2:
			setBackendOpenGLES2(this);
			break;
		case Backend::OpenGLES1:
			setBackendOpenGLES1(this);
			break;
		case Backend::OpenGL:
			setBackendOpenGL(this);
			break;
		default:
			setBackendNone(this);
			return false;
	}

	return true;
}
*/
} // ::al
