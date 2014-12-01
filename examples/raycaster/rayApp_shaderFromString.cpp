#include "alloutil/al_RayApp.hpp"

using namespace al;

struct MyApp : public RayApp {
  
  MyApp()
  {
    nav().pos(0,0,3.0);
  }
  
  virtual ~MyApp() {
  }
  
  virtual void onAnimate(al_sec dt) {
  }
  
  virtual void onSound(AudioIOData& io) {
    while (io()) {
      io.out(0) = rnd::uniformS() * 0.02;
    }
  }
  
  virtual void sendUniforms(ShaderProgram& shaderProgram) {
    shaderProgram.uniform("eyesep", lens().eyeSep());
    shaderProgram.uniform("pos", nav().pos());
    shaderProgram.uniform("quat", nav().quat());
  }
  
  std::string vertexCode() {
    // you can use R"()"; instead of AL_STRINGIFY if using c++11
    return AL_STRINGIFY(
    varying vec2 T;
    void main(void) {
      // pass through the texture coordinate (normalized pixel):
      T = vec2(gl_MultiTexCoord0);
      gl_Position = vec4(T*2.-1., 0, 1);
    }
    );
  }
  
  std::string fragmentCode() {
    return AL_STRINGIFY(
    uniform sampler2D pixelMap;
    uniform sampler2D alphaMap;
    uniform vec4 quat;
    uniform vec3 pos;
    uniform float eyesep;
    varying vec2 T;
        
    // q must be a normalized quaternion
    vec3 quat_rotate(in vec4 q, in vec3 v) {
      // return quat_mul(quat_mul(q, vec4(v, 0)), quat_conj(q)).xyz;
      // reduced:
      vec4 p = vec4(
                    q.w*v.x + q.y*v.z - q.z*v.y,  // x
                    q.w*v.y + q.z*v.x - q.x*v.z,  // y
                    q.w*v.z + q.x*v.y - q.y*v.x,  // z
                    -q.x*v.x - q.y*v.y - q.z*v.z   // w
                    );
      return vec3(
                  -p.w*q.x + p.x*q.w - p.y*q.z + p.z*q.y,  // x
                  -p.w*q.y + p.y*q.w - p.z*q.x + p.x*q.z,  // y
                  -p.w*q.z + p.z*q.w - p.x*q.y + p.y*q.x   // z
                  );
    }
    
    bool findIntersection(in vec3 orig, in vec3 dir, inout vec3 hitpoint, inout vec3 normal) {
      // orig, dir is normalized
      vec3 sphere_center = vec3(0,0,0);
      float sphere_radius = 1.0;
      
      float B,C,D,t;
      
      vec3 new_orig = orig - sphere_center;
      
      B = dot(dir, new_orig);
      C = dot(new_orig, new_orig) - sphere_radius * sphere_radius;
      
      D = B*B - C;
      
      if (D < 0.0) return false;
      
      D = sqrt(D);
      t = -B - D;
      if (t < 0.0) {
        t = -B + D;
        if (t < 0.0) return false;
      }
      
      hitpoint = orig + dir * t;
      normal = normalize(hitpoint - sphere_center);
      return true;
    }
    
    void main(){
      // lighting:
      vec3 color = vec3(0, 0, 0);
      vec3 light_pos = vec3(3, 1, 5);
      vec3 material_color = vec3(0.3, 0.3, 1.0);
      vec3 ambient_color = vec3(0.1, 0.1, 0.1);
      
      // pixel location (calibration space):
      vec3 v = normalize(texture2D(pixelMap, T).rgb);
      // ray direction (world space);
      vec3 rd = quat_rotate(quat, v);
      
      // stereo offset:
      // should reduce to zero as the nv becomes close to (0, 1, 0)
      // take the vector of nv in the XZ plane
      // and rotate it 90' around Y:
      vec3 up = vec3(0, 1, 0);
      vec3 rdx = cross(normalize(rd), up);
      
      //vec3 rdx = projection_on_plane(rd, up);
      vec3 eye = rdx * eyesep * 10.0;
      
      // ray origin (world space)
      vec3 ro = pos + eye;
      
      // calculate new ray direction for positive parallax
      v -= eye;
      rd = quat_rotate(quat, v);
      rd = normalize(rd);
      
      // find object intersection:
      vec3 p = ro;
      vec3 normal;
      
      if(findIntersection(ro, rd, p, normal)) {
        // compute ray to light source:
        vec3 light_dir = normalize(light_pos - p);
        
        // abs for bidirectional surfaces
        float light_normal = max(0.,dot(light_dir, normal));
        
        color = ambient_color + material_color * light_normal;
      }
      
      color *= texture2D(alphaMap, T).rgb;
      
      gl_FragColor = vec4(color, 1);
    }
    );
  }
  
  virtual void onMessage(osc::Message& m) {
  }
  
  virtual bool onKeyDown(const Keyboard& k){
    if(k.ctrl()) {
      switch(k.key()) {
        case '1': mOmni.mode(RayStereo::MONO); return false;
        case '2': mOmni.mode(RayStereo::SEQUENTIAL); return false;
        case '3': mOmni.mode(RayStereo::ACTIVE); return false;
        case '4': mOmni.mode(RayStereo::DUAL); return false;
        case '5': mOmni.mode(RayStereo::ANAGLYPH); return false;
        case '6': mOmni.mode(RayStereo::LEFT_EYE); return false;
        case '7': mOmni.mode(RayStereo::RIGHT_EYE); return false;
      }
    } else {
      if(k.key() == '1') {
        nav().pos().set(0.000000, 0.000000, 3.000000);
        nav().quat().set(1.000000, 0.000000, 0.000000, 0.000000);
      }
      else if(k.key() == '2') {
        nav().pos().set(0.647900, -0.344193, 0.861521);
        nav().quat().set(0.977944, -0.119045, -0.164871, 0.047663);
      }
      else if(k.key() == '3') {
        nav().pos().set(0.119933, 0.338736, 0.335348);
        nav().quat().set(0.899555, -0.436596, 0.001055, -0.013571);
      }
      else if(k.key() == '4') {
        nav().pos().set(-0.464406, -0.093565, -0.479541);
        nav().quat().set(0.624573, -0.730013, -0.073427, -0.267580);
      }
    }
    
    return true;
  }
};

int main(int argc, char * argv[]) {
  MyApp().start();
  return 0;
}