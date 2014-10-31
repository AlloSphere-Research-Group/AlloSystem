#ifndef AL_OMNI_STEREO_GRAPHICS_RENDERER_H
#define AL_OMNI_STEREO_GRAPHICS_RENDERER_H

#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "alloutil/al_FPS.hpp"
#include "alloutil/al_OmniStereo.hpp"
#include "alloutil/al_Simulator.hpp"  // DEVICE_SERVER_PORT_CONNECTION_US

namespace al {

class OmniStereoGraphicsRenderer : public Window,
                                   public FPS,
                                   public OmniStereo::Drawable {
 public:
  OmniStereoGraphicsRenderer();
  virtual ~OmniStereoGraphicsRenderer();

  virtual void onDraw(Graphics& gl) {}
  virtual void onAnimate(al_sec dt) {}
  virtual bool onCreate();
  virtual bool onFrame();
  virtual void onDrawOmni(OmniStereo& omni);
  virtual std::string vertexCode();
  virtual std::string fragmentCode();

  void start();
  void initWindow(const Window::Dim& dims = Window::Dim(800, 400),
                  const std::string title = "OmniStereoGraphicsRenderer",
                  double fps = 60,
                  Window::DisplayMode mode = Window::DEFAULT_BUF);
  void initOmni(std::string path = "");

  const Lens& lens() const { return mLens; }
  Lens& lens() { return mLens; }

  const Graphics& graphics() const { return mGraphics; }
  Graphics& graphics() { return mGraphics; }

  ShaderProgram& shader() { return mShader; }

  OmniStereo& omni() { return mOmni; }

  const std::string& hostName() const { return mHostName; }

  bool omniEnable() const { return bOmniEnable; }
  void omniEnable(bool b) { bOmniEnable = b; }

  osc::Send& oscSend() { return mOSCSend; }

 protected:
  const Nav& nav() const { return mNav; }
  Nav& nav() { return mNav; }

  OmniStereo mOmni;

  Lens mLens;
  Graphics mGraphics;

  osc::Send mOSCSend;
  Pose pose;

  ShaderProgram mShader;

  std::string mHostName;

  bool bOmniEnable;

  Nav mNav;
  NavInputControl mNavControl;
  StandardWindowKeyControls mStdControls;
};

inline void OmniStereoGraphicsRenderer::start() {
  if (mOmni.activeStereo()) {
    Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
  }

  create();

  if (mOmni.fullScreen()) {
    fullScreen(true);
    cursorHide(true);
  }

  Main::get().start();
}

inline OmniStereoGraphicsRenderer::~OmniStereoGraphicsRenderer() {}

inline OmniStereoGraphicsRenderer::OmniStereoGraphicsRenderer()
    : mNavControl(mNav), mOSCSend(12001) {

  bOmniEnable = true;
  mHostName = Socket::hostName();

  lens().near(0.01).far(40).eyeSep(0.03);
  nav().smooth(0.8);

  initWindow();
  initOmni();

  Window::append(mStdControls);
  Window::append(mNavControl);
}

inline void OmniStereoGraphicsRenderer::initOmni(std::string path) {
  mOmni.configure(path, mHostName);
  if (mOmni.activeStereo()) {
    mOmni.mode(OmniStereo::ACTIVE).stereo(true);
  }
}

inline void OmniStereoGraphicsRenderer::initWindow(const Window::Dim& dims,
                                                   const std::string title,
                                                   double fps,
                                                   Window::DisplayMode mode) {
  Window::dimensions(dims);
  Window::title(title);
  Window::fps(fps);
  Window::displayMode(mode);
}

inline bool OmniStereoGraphicsRenderer::onCreate() {
  mOmni.onCreate();

  Shader vert, frag;
  vert.source(OmniStereo::glsl() + vertexCode(), Shader::VERTEX).compile();
  vert.printLog();
  frag.source(fragmentCode(), Shader::FRAGMENT).compile();
  frag.printLog();
  mShader.attach(vert).attach(frag).link();
  mShader.printLog();
  mShader.begin();
  mShader.uniform("lighting", 1.0);
  mShader.uniform("texture", 0.0);
  mShader.end();

  return true;
}

inline bool OmniStereoGraphicsRenderer::onFrame() {
  FPS::onFrame();

  // if running on a laptop?
  //
  nav().step();
  Vec3d v = nav().pos();
  Quatd q = nav().quat();
  oscSend().send("/pose", v.x, v.y, v.z, q.x, q.y, q.z, q.w);
  // nav().print();

  onAnimate(dt);

  Viewport vp(width(), height());

  if (bOmniEnable) {
    mOmni.onFrame(*this, lens(), pose, vp);
  } else {
    mOmni.onFrameFront(*this, lens(), pose, vp);
  }
  return true;
}

inline void OmniStereoGraphicsRenderer::onDrawOmni(OmniStereo& omni) {
  graphics().error("start onDraw");
  mShader.begin();
  mOmni.uniforms(mShader);
  onDraw(graphics());
  mShader.end();
}

inline std::string OmniStereoGraphicsRenderer::vertexCode() {
  // XXX use c++11 string literals
  return R"(
varying vec4 color;
varying vec3 normal, lightDir, eyeVec;
void main() {
  color = gl_Color;
  vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
  normal = gl_NormalMatrix * gl_Normal;
  vec3 V = vertex.xyz;
  eyeVec = normalize(-V);
  lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = omni_render(vertex);
}
)";
}

inline std::string OmniStereoGraphicsRenderer::fragmentCode() {
  return R"(
uniform float lighting;
uniform float texture;
uniform sampler2D texture0;
varying vec4 color;
varying vec3 normal, lightDir, eyeVec;
void main() {
  vec4 colorMixed;
  if (texture > 0.0) {
    vec4 textureColor = texture2D(texture0, gl_TexCoord[0].st);
    colorMixed = mix(color, textureColor, texture);
  } else {
    colorMixed = color;
  }
  vec4 final_color = colorMixed * gl_LightSource[0].ambient;
  vec3 N = normalize(normal);
  vec3 L = lightDir;
  float lambertTerm = max(dot(N, L), 0.0);
  final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
  vec3 E = eyeVec;
  vec3 R = reflect(-L, N);
  float spec = pow(max(dot(R, E), 0.0), 0.9 + 1e-20);
  final_color += gl_LightSource[0].specular * spec;
  gl_FragColor = mix(colorMixed, final_color, lighting);
}
)";
}

}  // al

#endif

