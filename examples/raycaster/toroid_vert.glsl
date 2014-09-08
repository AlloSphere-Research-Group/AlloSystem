varying vec2 T;
void main(void) {
  // pass through the texture coordinate (normalized pixel):
  T = vec2(gl_MultiTexCoord0);
  gl_Position = vec4(T*2.-1., 0, 1);
}