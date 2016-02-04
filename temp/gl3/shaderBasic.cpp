#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_Shader.hpp"

#include <stdio.h>
#include <iostream>

using namespace al;
using namespace std;

static const char * vert = R"(
#version 410

in vec3 pos;

// uniform mat4 modelview;
// uniform mat4 proj;

void main(){
	vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0),
	                           vec4(-0.25, -0.25, 0.5, 1.0),
	                           vec4(0.25, 0.25, 0.5, 1.0));

	gl_Position = vertices[gl_VertexID];
	// gl_Position = proj * modelview * pos;
}
)";

static const char * frag = R"(
#version 410

out vec4 frag_color;

void main(){
	frag_color = vec4(1.0, 0.8, 0.8, 1.0);
}
)";

Graphics gl;

struct MyWindow : Window {

	ShaderProgram shader;
	GLuint vao;

	bool onCreate() {
		shader.compile(vert, frag);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		return true;
	}

	bool onFrame(){
		gl.clearColor(0.8, 0.9, 0.8, 0);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		shader.begin();
		glDrawArrays(GL_TRIANGLES, 0, 3);
		shader.end();
		return true;
	}

};


int main(){
	MyWindow win;
	win.append(*new StandardWindowKeyControls);
	win.create(
		Window::Dim(100, 0, 400,300),	// dimensions, in pixels
		"Window",						// title
		5,								// ideal frames/second; actual rate will vary
		Window::DEFAULT_BUF				// display mode
	);
	MainLoop::start();
	return 0;
}