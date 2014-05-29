#include "alloutil/al_OmniApp.hpp"
using namespace al;

struct MyApp : OmniApp {
  Mesh mesh;

  MyApp() {
    mesh.primitive(Graphics::TRIANGLES);
    addCube(mesh);
//    mesh.color(Color(0, 0, 0));
//    mesh.color(Color(0, 0, 1));
//    mesh.color(Color(0, 1, 0));
//    mesh.color(Color(0, 1, 1));
//    mesh.color(Color(1, 0, 0));
//    mesh.color(Color(1, 0, 1));
//    mesh.color(Color(1, 1, 0));
//    mesh.color(Color(1, 1, 1));
    mesh.generateNormals();
  }
  // omni_face

  virtual std::string vertexCode() {
    return AL_STRINGIFY(
      varying vec4 vy_CubeMapFaceColor;
      varying vec4 color;
      varying vec3 normal, lightDir, eyeVec;
      void main() {
        vy_CubeMapFaceColor =
          (omni_face == 0) ? vec4(0, 0, 0, 1) :
          (omni_face == 1) ? vec4(0, 0, 1, 1) :
          (omni_face == 2) ? vec4(0, 1, 0, 1) :
          (omni_face == 3) ? vec4(1, 0, 0, 1) :
          (omni_face == 4) ? vec4(0, 1, 1, 1) :
          vec4(1, 1, 1, 1);
        color = gl_Color;
        vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
        normal = gl_NormalMatrix * gl_Normal;
        vec3 V = vertex.xyz;
        eyeVec = normalize(-V);
        lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = omni_render(vertex);
      }
    );
  }

  virtual std::string fragmentCode() {
    return AL_STRINGIFY(
      varying vec4 vy_CubeMapFaceColor;
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
        //gl_FragColor = mix(colorMixed, final_color, vy_CubeMapFaceColor) ;
        gl_FragColor = vy_CubeMapFaceColor;
      }
    );
  }

  virtual ~MyApp() {}

  virtual void onDraw(Graphics& g) {
    shader().uniform("lighting", 0.0);
    g.draw(mesh);
  }

  virtual void onAnimate(al_sec dt) {}
};

int main(int argc, char* argv[]) {
  MyApp().start();
  return 0;
}
