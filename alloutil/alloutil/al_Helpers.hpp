#ifndef AL_OMNIFOO_H
#define AL_OMNIFOO_H

#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "alloutil/al_FPS.hpp"
#include "alloutil/al_OmniStereo.hpp"

#define PORT_TO_DEVICE_SERVER (12000)
#define PORT_FROM_DEVICE_SERVER (PORT_TO_DEVICE_SERVER + 1)
#define DEVICE_SERVER_IP_ADDRESS "BOSSANOVA"

namespace al {

// AUDIO //

class AudioRenderer {
 public:
  AudioRenderer();
  virtual ~AudioRenderer();

  virtual void onSound(AudioIOData& io) {}

  void start();
  void initAudio(double audioRate = 44100, int audioBlockSize = 256);
  void initAudio(std::string devicename, double audioRate, int audioBlockSize,
                 int audioInputs, int audioOutputs);

  const AudioIO& audioIO() const { return mAudioIO; }
  AudioIO& audioIO() { return mAudioIO; }

 protected:
  AudioIO mAudioIO;
  static void AppAudioCB(AudioIOData& io);
};

inline void AudioRenderer::initAudio(double audioRate, int audioBlockSize) {
  mAudioIO.callback = AppAudioCB;
  mAudioIO.user(this);
  mAudioIO.framesPerSecond(audioRate);
  mAudioIO.framesPerBuffer(audioBlockSize);
}

inline void AudioRenderer::initAudio(std::string devicename, double audioRate,
                                 int audioBlockSize, int audioInputs,
                                 int audioOutputs) {
  AudioDevice indev(devicename, AudioDevice::INPUT);
  AudioDevice outdev(devicename, AudioDevice::OUTPUT);
  indev.print();
  outdev.print();
  mAudioIO.deviceIn(indev);
  mAudioIO.deviceOut(outdev);
  mAudioIO.channelsOut(audioOutputs);
  mAudioIO.channelsIn(audioInputs);
  initAudio(audioRate, audioBlockSize);
}

inline void AudioRenderer::AppAudioCB(AudioIOData& io) {
  AudioRenderer& app = io.user<AudioRenderer>();
  io.frame(0);
  app.onSound(io);
}

inline void AudioRenderer::start() {
  mAudioIO.start();
  Main::get().start();
}

inline AudioRenderer::~AudioRenderer() {}

inline AudioRenderer::AudioRenderer() { initAudio(); }

// GRAPHICS //

class GraphicsRenderer : public Window, public FPS, public OmniStereo::Drawable {
 public:
  GraphicsRenderer();
  virtual ~GraphicsRenderer();

  virtual void onDraw(Graphics& gl) {}
  virtual void onAnimate(al_sec dt) {}
  virtual bool onCreate();
  virtual bool onFrame();
  virtual void onDrawOmni(OmniStereo& omni);
  virtual std::string vertexCode();
  virtual std::string fragmentCode();

  void start();
  void initWindow(const Window::Dim& dims = Window::Dim(800, 400),
                  const std::string title = "GraphicsRenderer", double fps = 60,
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

 protected:
  OmniStereo mOmni;

  Lens mLens;
  Graphics mGraphics;

  Pose pose;

  ShaderProgram mShader;

  std::string mHostName;

  bool bOmniEnable;
};

// RENDERER //

inline void GraphicsRenderer::start() {
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

inline GraphicsRenderer::~GraphicsRenderer() {}
inline GraphicsRenderer::GraphicsRenderer() {
  bOmniEnable = true;
  mHostName = Socket::hostName();

  lens().near(0.01).far(40).eyeSep(0.03);

  initWindow();
  initOmni();

  // later, we may want to forward window controls via osc to the device server
  // or directly to the simulator.
  // XXX Window::append(mStdControls);
  // XXX Window::append(mNavControl);
}

inline void GraphicsRenderer::initOmni(std::string path) {
  mOmni.configure(path, mHostName);
  if (mOmni.activeStereo()) {
    mOmni.mode(OmniStereo::ACTIVE).stereo(true);
  }
}

inline void GraphicsRenderer::initWindow(const Window::Dim& dims,
                                     const std::string title, double fps,
                                     Window::DisplayMode mode) {
  Window::dimensions(dims);
  Window::title(title);
  Window::fps(fps);
  Window::displayMode(mode);
}

inline bool GraphicsRenderer::onCreate() {
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

inline bool GraphicsRenderer::onFrame() {
  FPS::onFrame();

  onAnimate(dt);

  Viewport vp(width(), height());

  if (bOmniEnable) {
    mOmni.onFrame(*this, lens(), pose, vp);
  } else {
    mOmni.onFrameFront(*this, lens(), pose, vp);
  }
  return true;
}

inline void GraphicsRenderer::onDrawOmni(OmniStereo& omni) {
  graphics().error("start onDraw");

  mShader.begin();
  mOmni.uniforms(mShader);

  onDraw(graphics());

  mShader.end();
}

inline std::string GraphicsRenderer::vertexCode() {
  return AL_STRINGIFY(varying vec4 color; varying vec3 normal, lightDir, eyeVec;
                      void main() {
    color = gl_Color;
    vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
    normal = gl_NormalMatrix * gl_Normal;
    vec3 V = vertex.xyz;
    eyeVec = normalize(-V);
    lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = omni_render(vertex);
  });
}

inline std::string GraphicsRenderer::fragmentCode() {
  return AL_STRINGIFY(uniform float lighting; uniform float texture;
                      uniform sampler2D texture0; varying vec4 color;
                      varying vec3 normal, lightDir, eyeVec; void main() {

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
  });
}

// SIMULATOR //

// XXX - will go in simulator
// while (oscRecv().recv()) {}
// nav().step();

class Simulator : public osc::PacketHandler {
 public:
  Simulator(std::string name = "omniapp");
  virtual ~Simulator();
  void start();

  const Nav& nav() const { return mNav; }
  Nav& nav() { return mNav; }

  const std::string& name() const { return mName; }
  Simulator& name(const std::string& v) {
    mName = v;
    return *this;
  }

  osc::Recv& oscRecv() { return mOSCRecv; }
  osc::Send& oscSend() { return mOSCSend; }

  virtual void onMessage(osc::Message& m);

  void sendHandshake();
  void sendDisconnect();

 protected:
  osc::Recv mOSCRecv;
  osc::Send mOSCSend;
  std::string mName;

  Nav mNav;
  NavInputControl mNavControl;
  // StandardWindowKeyControls mStdControls;

  double mNavSpeed, mNavTurnSpeed;
};

inline void Simulator::sendHandshake() {
  oscSend().send("/handshake", name(), oscRecv().port());
}

inline void Simulator::sendDisconnect() {
  oscSend().send("/disconnectApplication", name());
}

inline void Simulator::onMessage(osc::Message& m) {
  float x;
  if (m.addressPattern() == "/mx") {
    m >> x;
    nav().moveR(-x * mNavSpeed);

  } else if (m.addressPattern() == "/my") {
    m >> x;
    nav().moveU(x * mNavSpeed);

  } else if (m.addressPattern() == "/mz") {
    m >> x;
    nav().moveF(x * mNavSpeed);

  } else if (m.addressPattern() == "/tx") {
    m >> x;
    nav().spinR(x * -mNavTurnSpeed);

  } else if (m.addressPattern() == "/ty") {
    m >> x;
    nav().spinU(x * mNavTurnSpeed);

  } else if (m.addressPattern() == "/tz") {
    m >> x;
    nav().spinF(x * -mNavTurnSpeed);

  } else if (m.addressPattern() == "/home") {
    nav().home();

  } else if (m.addressPattern() == "/halt") {
    nav().halt();
  }
}

inline void Simulator::start() {
  if (oscSend().opened()) sendHandshake();
  Main::get().start();
}

inline Simulator::~Simulator() { sendDisconnect(); }

inline Simulator::Simulator(std::string name)
    : mOSCRecv(PORT_FROM_DEVICE_SERVER),
      mNavControl(mNav),
      mOSCSend(PORT_TO_DEVICE_SERVER, DEVICE_SERVER_IP_ADDRESS) {
  mName = name;
  oscRecv().bufferSize(32000);
  oscRecv().handler(*this);
  sendHandshake();
  mNavSpeed = 1;
  mNavTurnSpeed = 0.02;
  nav().smooth(0.8);
}

}  // al::

#endif
