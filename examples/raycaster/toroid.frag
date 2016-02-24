uniform sampler2D pixelMap;
uniform sampler2D alphaMap;
uniform vec4 quat;
uniform vec3 pos;
uniform float eyesep;
uniform float time;
varying vec2 T;
// layout(location = 0) out vec3 testcolor;

float accuracy = 1e-3;
float stepsize = 0.02;
float maxval = 10.0;
float normalEps = 1e-5;
float m_2pi = 6.28318530718;

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

float evalAt(vec3 at) {
  // A torus. To be used with ray marching.
  // See http://www.freigeist.cc/gallery.html .
  float R = 1.0;
  float r = 0.5;
  
  R *= R;
  r *= r;
  
  float t = dot(at, at) + R - r;
  
  return t * t - 4.0 * R * dot(at.xy, at.xy);
}

bool findIntersection(in vec3 orig, in vec3 dir, inout vec3 hitpoint, inout vec3 normal) {
  // Raymarching with fixed initial step size and final bisection.
  // The object has to define evalAt().
  float cstep = stepsize;
  float alpha = cstep;
  
  vec3 at = orig + alpha * dir;
  float val = evalAt(at);
  bool sit = (val < 0.0);
  
  alpha += cstep;
  
  bool sitStart = sit;
  
  while (alpha < maxval)
  {
    at = orig + alpha * dir;
    val = evalAt(at);
    sit = (val < 0.0);
    
    // Situation changed, start bisection.
    if (sit != sitStart)
    {
      float a1 = alpha - stepsize;
      
      while (cstep > accuracy)
      {
        cstep *= 0.5;
        alpha = a1 + cstep;
        
        at = orig + alpha * dir;
        val = evalAt(at);
        sit = (val < 0.0);
        
        if (sit == sitStart)
          a1 = alpha;
      }
      
      hitpoint = at;
      
      // "Finite difference thing". :)
      normal.x = evalAt(at + vec3(normalEps, 0, 0));
      normal.y = evalAt(at + vec3(0, normalEps, 0));
      normal.z = evalAt(at + vec3(0, 0, normalEps));
      normal -= val;
      normal = normalize(normal);
      
      return true;
    }
    
    alpha += cstep;
  }
  
  return false;
}

void main(){
  vec3 light1 = pos + vec3(0.2 * sin(m_2pi * time + 0.3),
                           0.3 * sin(m_2pi * time),
                           0.5 * cos(m_2pi * time + 0.6));//pos + vec3(0.0, 0.5, 1.0);
  vec3 light2 = pos + vec3(1.0, 0.0, 0.0);
  vec3 color1 = vec3(1.0, 1.0, 1.0);
  vec3 color2 = vec3(0.3 + 0.1 * sin(m_2pi * time),
                     0.3 + 0.1 * sin(m_2pi * time + 1.0),
                     1.0);
  vec3 ambient = vec3(0.3, 0.3, 0.3);
  
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
  vec3 eye = rdx * eyesep;// * 0.02;
  
  // ray origin (world space)
  vec3 ro = pos + eye;
  
  // calculate new ray direction for positive parallax
  v -= eye;
  rd = quat_rotate(quat, v);
  rd = normalize(rd);
  
  // initial eye-ray to find object intersection:
  float mindt = 0.01;	// how close to a surface we can get
  float mint = mindt;
  float maxt = 50.;
  float t=mint;
  float h = maxt;
  
  // find object intersection:
  vec3 p = ro + mint*rd;
  vec3 normal;
  if(!findIntersection(ro, rd, p, normal)) {
    t = maxt;
  }
  
  // lighting:
  vec3 color = vec3(0, 0, 0);
  
  if (t<maxt) {
    // compute ray to light source:
    vec3 ldir1 = normalize(light1 - p);
    vec3 ldir2 = normalize(light2 - p);
    
    // abs for bidirectional surfaces
    float ln1 = max(0.,dot(ldir1, normal));
    float ln2 = max(0.,dot(ldir2, normal));
    
    color = ambient + color1 * ln1 + color2 * ln2;
  }
  
  color *= texture2D(alphaMap, T).rgb;
  // color *= vec3(1.0, 0.0, 0.0);
  
  gl_FragColor = vec4(color, 1);
}
