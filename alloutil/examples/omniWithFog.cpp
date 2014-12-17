#include "alloutil/al_OmniApp.hpp"
using namespace al;

struct MyApp : OmniApp {
  Mesh mesh;
  Light light;

  MyApp() {
    mesh.primitive(Graphics::TRIANGLES);
    addSphere(mesh, 1.0, 32, 32);
    for (int i = 0; i < mesh.vertices().size(); ++i) {
      float f = (float)i / mesh.vertices().size();
      mesh.color(Color(HSV(f, 1 - f, 1), 1));
    }
    mesh.generateNormals();
    light.ambient(Color(0.4, 0.4, 0.4, 1.0));
    light.pos(5, 5, 5);
  }

  virtual ~MyApp() {}

  virtual void onDraw(Graphics& g) {
    light();
    // say how much lighting you want
    shader().uniform("lighting", 1.0);

/*!*/ g.fog(lens().far(), lens().near()+2, Color(0, 0, 0, 1));
/*!*/ shader().uniform("fogCurve", 30.0);

    g.draw(mesh);
  }

  virtual void onAnimate(al_sec dt) {
    //light.pos(nav().pos());
    //std::cout << dt << std::endl;
  }

  virtual void onSound(AudioIOData& io) {
    while (io()) {
      io.out(0) = rnd::uniformS() * 0.05;
    }
  }

  virtual void onMessage(osc::Message& m) {
    OmniApp::onMessage(m);
  }

  virtual bool onKeyDown(const Keyboard& k){
    return true;
  }

  std::string vertexCode() {
    return AL_STRINGIFY(
      varying vec4 color;
      varying vec3 normal, lightDir, eyeVec;

/*!*/ uniform float fogCurve;
/*!*/ varying float fogFactor;

      void main(){
        color = gl_Color;
        vec4 vertex = gl_ModelViewMatrix * gl_Vertex;

        normal = gl_NormalMatrix * gl_Normal;
        vec3 V = vertex.xyz;
        eyeVec = normalize(-V);
        lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = omni_render(vertex);

/*!*/   float z = gl_Position.z;
/*!*/   fogFactor = (z - gl_Fog.start) * gl_Fog.scale;
/*!*/   fogFactor = clamp(fogFactor, 0., 1.);
/*!*/   if(fogCurve != 0.){
/*!*/     fogFactor = (1. - exp(-fogCurve*fogFactor))/(1. - exp(-fogCurve));
/*!*/   }
      }
    );
  }

  std::string fragmentCode() {
    return AL_STRINGIFY(
      uniform float lighting;
      uniform float texture;
      uniform sampler2D texture0;
      varying vec4 color;
      varying vec3 normal, lightDir, eyeVec;

/*!*/ varying float fogFactor;

      void main() {
        vec4 colorMixed;
        if( texture > 0.0){
          vec4 textureColor = texture2D(texture0, gl_TexCoord[0].st);
          colorMixed = mix(color, textureColor, texture);
        }else{
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
/*!*/   gl_FragColor = mix(mix(colorMixed, final_color, lighting), gl_Fog.color, fogFactor);
      }
    );
  }
};

int main(int argc, char * argv[]) {
  MyApp().start();
  return 0;
}
