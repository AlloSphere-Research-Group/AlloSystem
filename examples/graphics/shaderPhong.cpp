#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"

using namespace al;

//static const char * vLight ="\
//varying vec3 N, v;\
//void main(){\
//	v = vec3(gl_ModelViewMatrix * gl_Vertex);\
//	N = normalize(gl_NormalMatrix * gl_Normal);\
//	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\
//}";
//
//static const char * fLight ="\
//varying vec3 N, v;\
//void main(){\
//	vec3 L = normalize(gl_LightSource[0].position.xyz - v);\ 
//	vec3 E = normalize(-v);\
//	vec3 R = normalize(-reflect(L,N));\
//	vec4 Iamb = gl_FrontLightProduct[0].ambient;\  
//	vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N,L), 0.0);\
//	Idiff = clamp(Idiff, 0.0, 1.0);\
//	vec4 Ispec = gl_FrontLightProduct[0].specular\
//				* pow(max(dot(R,E),0.0),0.3*gl_FrontMaterial.shininess);\
//	Ispec = clamp(Ispec, 0.0, 1.0);\
//	gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;\   
//}";

static const char * vLight ="\
varying vec3 normal, lightDir, eyeVec;\
void main(){\
	normal = gl_NormalMatrix * gl_Normal;\
	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);\
	lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);\
	eyeVec = -vVertex;\
	gl_Position = ftransform();\
}";

static const char * fLight ="\
varying vec3 normal, lightDir, eyeVec;\
void main(){\
	vec4 final_color = \
	(gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + \
	(gl_LightSource[0].ambient * gl_FrontMaterial.ambient);\
	vec3 N = normalize(normal);\
	vec3 L = normalize(lightDir);\
	float lambertTerm = dot(N,L);\
	if(lambertTerm > 0.0){\
		final_color += gl_LightSource[0].diffuse * \
		               gl_FrontMaterial.diffuse * \
					   lambertTerm;	\
		vec3 E = normalize(eyeVec);\
		vec3 R = reflect(-L, N);\
		float specular = pow( max(dot(R, E), 0.0), \
		                 gl_FrontMaterial.shininess );\
		final_color += gl_LightSource[0].specular * \
		               gl_FrontMaterial.specular * \
					   specular;\
	}\
	gl_FragColor = final_color;\
}";


struct MyWindow : Window{

	ShaderProgram shaderP;
	Shader shaderV, shaderF;
	Mesh mesh;
	GraphicsGL gl;
	Light light;
	Material material;
	double angle;

	bool onCreate(){
		shaderV.source(vLight, Shader::VERTEX);
		shaderF.source(fLight, Shader::FRAGMENT);
		shaderF.compile();
		shaderV.compile();
		shaderP.attach(shaderV);
		shaderP.attach(shaderF);
		shaderP.link();
		
		printf("%s", shaderV.log());
		printf("%s", shaderF.log());
		printf("%s", shaderP.log());
		
		mesh.primitive(gl.TRIANGLES);
		addIcosahedron(mesh);
		//addDodecahedron(mesh);
		mesh.decompress();
		mesh.generateNormals();
		
		return true;
	}

	bool onFrame(){

		gl.depthTesting(true);
		gl.clearColor(1,1,1,1);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.viewport(0,0, width(), height());

		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-3), Vec3d(0,0,0), Vec3d(0,1,0)));

		shaderP.begin();

			material.shininess(32);

			light.ambient(Color(0.1,0,0));
			light.diffuse(Color(0.9,0,0));
			light.specular(Color(0,0.5,0));

			material();
			light();
		
			if((angle+=0.3)>=360) angle=0;
			gl.rotate(angle, 0,1,0);
			gl.rotate(angle*2, 0,0,1);

			gl.draw(mesh);

		shaderP.end();

		return true;
	}
};

int main(){
	MyWindow win1;
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
