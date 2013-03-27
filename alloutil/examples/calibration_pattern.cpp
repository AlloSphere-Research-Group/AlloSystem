#include "alloutil/al_OmniApp.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace al;

static const double translationScale = 0.005;
static const double rotationScale = 0.001;
static const std::string textFileName("registration_pose.txt");

static const al::Color backgroundColor(1,1,1);
static const al::Color innerSphereColor(RGB(0.3,0.3,0.3));
static const al::Color outerSphereColor(RGB(0,1,1));

static const double majorLineWidth = 3.0;
static const double minorLineWidth = 1.0;

// static const std::string calibrationPath("/alloshare/img");

struct MyApp : OmniApp {
public:
    MyApp(std::string name) : OmniApp(name) {
		
        double radius = 10;
		double smallRadius = 1;
		
        CreateCircles(radius,smallRadius);
    
        CreateSphericalCage(1.001*radius,mInnerCage,innerSphereColor);
        //CreateSphericalCage(2.0*radius,mOuterCage,outerSphereColor);
		
        mLight.ambient(Color(1.0, 1.0, 1.0, 1.0));
        mLight.pos(0, 0, 0);
		
	//        initOmni(calibrationPath);
        //mOmni.mode(OmniStereo::MONO);
        mOmni.mode(OmniStereo::ACTIVE);
    }
    
    virtual ~MyApp() {}
    
    //equal latitude and longitude
    void CreateSphericalCage(double radius, Mesh & mesh, al::Color const & color){
        //slices should be divisible by 4 and stacks by 2
        mesh.primitive(al::Graphics::LINES);
        al::addSphere(mesh, radius, 128, 64); //mesh,radius,slices,stacks
        mesh.color(color);
        mesh.generateNormals();
    }
    void CreateCircles(double radius, double smallRadius){
        
        {
            //3 most important circles
            const int segments = 256;
            
            mXZCircle.primitive(Graphics::LINE_LOOP); //equator
            mXYCircle.primitive(Graphics::LINE_LOOP);
            mYZCircle.primitive(Graphics::LINE_LOOP);
            
            for (int i = 0; i < segments; ++i) {
                double t = 2.0*M_PI*i/segments;
                mXZCircle.vertex(   radius*sin(t),      0.0,        radius*cos(t)   );
                mXYCircle.vertex(   radius*cos(t),  radius*sin(t),      0.0         );
                mYZCircle.vertex(       0.0,        radius*cos(t),  radius*sin(t)   );
            }
            
            mXZCircle.color(RGB(1,0,0));
            mXYCircle.color(RGB(0,1,0));
            mYZCircle.color(RGB(0,0,1));
            
            mXZCircle.generateNormals();
            mXYCircle.generateNormals();
            mYZCircle.generateNormals();
        }
        {
            //projected directly in front and ahead.  will appear elliptical if space is scaled non-uniformly
            const int segments = 128;
            
            mBackCircle.primitive(Graphics::LINE_LOOP);
            mTopCircle.primitive(Graphics::LINE_LOOP);
            mRightCircle.primitive(Graphics::LINE_LOOP);
            
            for (int i = 0; i < segments; ++i){
                double t = 2.0*M_PI*i/segments;
		double doorwayFudgeFactor = 2.0;
                mBackCircle.vertex(smallRadius*cos(t), smallRadius*sin(t), sqrt(radius*radius - smallRadius*smallRadius));
                mTopCircle.vertex(smallRadius*cos(t), sqrt(radius*radius - smallRadius*smallRadius), smallRadius*sin(t));
                mRightCircle.vertex(sqrt(radius*radius - smallRadius*smallRadius)*(1/doorwayFudgeFactor), 
				    smallRadius*sin(t)*doorwayFudgeFactor, smallRadius*cos(t)*doorwayFudgeFactor);
            }
            mRightCircle.color(RGB(1,0,0)); // In positive X = R color
            mTopCircle.color(RGB(0,1,0));   // In positive Y = G color
            mBackCircle.color(RGB(0,0,1));  // In positive Z = B color
            
            mRightCircle.generateNormals();
            mTopCircle.generateNormals();
            mBackCircle.generateNormals();
        }
    }
    
    virtual void onDraw(Graphics& g) {
        g.clearColor(backgroundColor);
        g.clear(al::Graphics::COLOR_BUFFER_BIT);
        mLight();
	g.lineWidth(majorLineWidth);
	g.disable(al::Graphics::LIGHTING);
        g.draw(mXZCircle);
	g.draw(mXYCircle);
        g.draw(mYZCircle);
        g.draw(mBackCircle);
	g.draw(mTopCircle);
	g.draw(mRightCircle);
        g.lineWidth(minorLineWidth);
        g.draw(mInnerCage);
        //g.draw(mOuterCage);
		g.enable(al::Graphics::LIGHTING);
    }
    
    virtual void onAnimate(al_sec dt) {
		osc::Packet p;
		p.beginMessage("/nav");
		p << nav().pos().x << nav().pos().y << nav().pos().z << nav().quat().x << nav().quat().y << nav().quat().z << nav().quat().w;
		p.endMessage();
		
		osc::Send(12001, "192.168.0.26").send(p);
		osc::Send(12001, "192.168.0.27").send(p);
		osc::Send(12001, "192.168.0.28").send(p);
		osc::Send(12001, "192.168.0.29").send(p);
    }
    std::string vertexCode() {
		return AL_STRINGIFY(
                            varying vec4 color;
                            void main() {
                                color = gl_Color;
                                vec4 vertex = gl_ModelViewMatrix * gl_Vertex;
                                gl_Position = omni_render(vertex);
                            }
                            );
	}
	
	std::string fragmentCode() {
		return AL_STRINGIFY(
                            varying vec4 color;
                            void main() {
                                gl_FragColor = color;
                            }
                            );
	}
    virtual void onSound(AudioIOData& io) {
    }
    
    virtual bool onKeyDown(const Keyboard& k){
        
        if(k.key() == '0') {
			nav().halt();
			nav().home();
		}
        else if(k.key() == ' ') {
            //std::cout << "writing pose data... " << std::flush;
            std::ofstream out(textFileName.c_str());
            if (out.is_open()) {
                if (!out.good()) {
                    std::cout << "Can't write to " << textFileName << std::endl;
                    return true;
                }
                out << std::setprecision(10);
                out << "x,y,z  w,x,y,z" << std::endl;
                out << nav().pos()[0] << " " << nav().pos()[1] << " " << nav().pos()[2] << " ";
                out << nav().quat()[0] << " " << nav().quat()[1] << " " << nav().quat()[2] << " " << nav().quat()[3];
                out.close();
                std::cout << "Pose written to " << textFileName << "." << std::endl;
            }
            else {
                std::cout << "Can't create  " << textFileName << std::endl;
                //return true;
            }
        }
		return true;
	}
	
	virtual void onMessage(osc::Message& message) {
		message.print();
		
        
        if(Socket::hostName() == "photon") {
            
            float value;
            
            if (message.addressPattern() == "/mx") {
				message >> value;
				nav().moveR(-value * translationScale);
            }
            else if (message.addressPattern() == "/my") {
				message >> value;
				nav().moveU(value * translationScale);
			}
            else if (message.addressPattern() == "/mz") {
				message >> value;
				nav().moveF(value * translationScale);
			}
            else if (message.addressPattern() == "/tx") {
				message >> value;
				nav().spinR(-value * rotationScale);
			}
            else if (message.addressPattern() == "/ty") {
				message >> value;
				nav().spinU(value * rotationScale);
			}
            else if (message.addressPattern() == "/tz") {
				message >> value;
				nav().spinF(-value * rotationScale);
			}
            else if (message.addressPattern() == "/h") {
				nav().home();
			}
		}
		else {
			double px, py, pz, qx, qy, qz, qw;
			if (message.addressPattern() == "/nav") {
				message >> px >> py >> pz >> qx >> qy >> qz >> qw;
				nav().pos().set(px, py, pz);
				nav().quat().set(qw, qx, qy, qz);
			}
		}
	}
private:
    Mesh mXZCircle, mXYCircle, mYZCircle; //major circles
    Mesh mBackCircle, mTopCircle, mRightCircle;;
    Mesh mInnerCage;
    Mesh mOuterCage;
	
    Light mLight;
};

int main(int argc, char * argv[]) {
    MyApp("calibration_pattern").start();
    return 0;
}
