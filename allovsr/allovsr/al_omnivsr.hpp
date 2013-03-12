//
//  al_omnivsr.hpp
//  allovsr
/*

    Implements Omnistereo in a Versor App
    
*/
//  Created by Pablo Colapinto on 3/9/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef allovsr_al_omnivsr_hpp
#define allovsr_al_omnivsr_hpp

#define PORT_TO_DEVICE_SERVER (12000)
#define PORT_FROM_DEVICE_SERVER (PORT_TO_DEVICE_SERVER+1)
#define DEVICE_SERVER_IP_ADDRESS "BOSSANOVA"

#include "allocore/al_Allocore.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "alloutil/al_FPS.hpp"

#include "allovsr/al_vsrInterface.hpp"
#include "alloutil/al_OmniStereo2.hpp"
#include "alloutil/al_OmniUtil.hpp"

namespace al {

    typedef OmniStereo2 Renderer;

    class OmniVsrApp  :  public Window,  public osc::PacketHandler, public FPS, public Renderer::Drawable {
    
//        typedef Renderer Rend;
        
        protected:
        
            //Has a vsr input event handler and a gui
        
            VsrInterface interface;
            VsrGui glv;

            //A Graphics Context
            Graphics mGraphics;
            
            //An Audio Interface
            AudioIO mAudioIO;
           
             //Lens Info
            al::Lens mLens;

            //Omnistereo Renderer
            Renderer mOmni;        

            //Cubemap Texture Builder
            ShaderProgram mShader;
            
            //Description
            string mDescription;
            string mName;
                
            //Messaging
            string mHostName;
            osc::Recv mOSCRecv;
            osc::Send mOSCSend;
            
    
            StandardWindowKeyControls mStdControls;
        
            bool bOmniEnable, bSlave;	

//            static void AppAudioCB(AudioIOData& io);
	

        public:

            /////////////////////
            //GETTERS AND SETTERS
            /////////////////////
            
            const string& name() const { return mName; }
            OmniVsrApp& name(const string& v){ mName=v; return *this; }
        
            //al::Window has its own mouse too so i call mine an "imouse". . .
            MouseData& imouse() { return interface.mouse; }
            VsrView& view() { return interface.view(); }
            Scene& scene() { return interface.scene(); }
            vsr::Camera& camera() { return interface.camera(); }
            Frame& model() { return interface.model(); }
            
        
            const Graphics&		graphics() const { return mGraphics; }
            Graphics&			graphics(){ return mGraphics; }
            
            ///////////////////
            // MESSAGING    ///
            ///////////////////
            
            osc::Recv&			oscRecv(){ return mOSCRecv; }
            osc::Send&			oscSend(){ return mOSCSend; }

            void sendHandshake(){
                oscSend().send("/handshake", name(), oscRecv().port());
            }

            void sendDisconnect(){
                oscSend().send("/disconnectApplication", name());
            }
        
        
        ///////////////////
        // CallBacks    ///
        ///////////////////
        
        //SUBCLASS-DEFINED
    	virtual void onDraw(Graphics& gl) {}            // Subclass Puts Draw Routines In Here
        virtual void onAnimate(al_sec dt) {}
        virtual void onSound(AudioIOData& io) {}
        virtual void onMessage(osc::Message& m){}

        
        bool onFrame(){
            
            FPS::onFrame();
            while(oscRecv().recv()) {}
    	
            onAnimate(dt);
	
            Viewport vp( width(), height() );
    
            vsr :: GL :: enablePreset();
            mGraphics.depthTesting(true);

                //Update ModelView Physics
                camera().step();
                model().step();
                
                glColor3f(1,1,1);
                
                //Call omnistereo onframe, passing in this OmniStereo::Drawable, 
                //omnistereo onframe will call this->onDrawOmni multiple times and then render the resulting cubemap texture to screen
                mOmni.onFrame( *this, mLens, Frame2Pose( camera() ), vp );
                                    
            vsr :: GL :: disablePreset();
            
            return true;
            
        }
        
        //Callback from mOmni's onFrame method (this is called multiple times)
        void onDrawOmni(Renderer& o) {
        
            graphics().error("start onDraw");
            
            mShader.begin();
            
            mOmni.uniforms(mShader);
            
            onDraw(graphics());
            
            mShader.end();
        }
       

        void modelTransform(){
            Rot t = vsr::Gen::aa( scene().model.rot() ); 
            vsr::GL::rotate( t.w() );
        }

        
        ///////////////////////////
        // DEFAULT CONSTRUCTOR  ///
        ///////////////////////////
        
        
        OmniVsrApp(bool slave = 0)
        : bSlave(slave),         
        mOSCRecv(PORT_FROM_DEVICE_SERVER),
        mOSCSend(PORT_TO_DEVICE_SERVER, DEVICE_SERVER_IP_ADDRESS)
        {
  
            mLens.fovy(60);
 //           add(new StandardWindowKeyControls);             // Standard Controls for full screen
//            add(&interface.input());
//            add(&interface.view());
            glv.parentWindow(*this);
            glv.gui.colors().back.set(.3,.3,.3);

            bOmniEnable = true;
            mHostName = Socket::hostName();
            
            initWindow();
            initOmni();
            
            Window::append(mStdControls);

            if (!bSlave) {
                //Window::append(mNavControl);	
                Window::append( interface.input() );				
                Window::append( interface.view() );				
                initAudio();
            
                oscRecv().bufferSize(32000);
                oscRecv().handler(*this);
                sendHandshake();
            }
        }
        
        virtual ~OmniVsrApp(){ if (!bSlave) sendDisconnect(); }
        
        void initOmni(string path = ""){

              if (path == "") {
                FILE *pipe = popen("echo ~", "r");
                if (pipe) {
                  char c;
                  while((c = getc(pipe)) != EOF) {
                if (c == '\r' || c == '\n')
                      break;
                path += c;
                  }
                  pclose(pipe);
                }
                path += "/calibration-current/";
              }

                mOmni.configure(path, mHostName);
                
                if (mOmni.activeStereo()) {
                    mOmni.mode(Renderer::ACTIVE).stereo(true);
                }
        }
                
        void initWindow( 
            const Window::Dim& dims = Window::Dim(800,400), 
            const std::string title = "OmniApp", 
            double fps = 60, 
            Window::DisplayMode mode = Window::DEFAULT_BUF){
            
            Window::dimensions(dims);
            Window::title(title);
            Window::fps(fps);
            Window::displayMode(mode);      
        }
        
        inline void initAudio(
            double audioRate = 44100, int audioBlockSize = 256
        ) {
            //mAudioIO.callback = AppAudioCB;
            mAudioIO.user(this);
            mAudioIO.framesPerSecond(audioRate);
            mAudioIO.framesPerBuffer(audioBlockSize);
        }

        
        bool onCreate() {
        
            //CREATE omnistereo
            mOmni.onCreate();

            //Compile CubeMap Building Shader
            Shader vert, frag;
            vert.source( GLSL::OmniCubeMap + GLSL::OmniBasicVertex, Shader::VERTEX).compile();            
            vert.printLog();
            
            frag.source( GLSL::OmniBasicFragment, Shader::FRAGMENT ).compile();
            frag.printLog();
            
            mShader.attach(vert).attach(frag).link();
            mShader.printLog();
            
            mShader.begin();
            mShader.uniform("lighting", 1.0);
            mShader.end();

            return true;
        }
        
        
       void start() {

            //Switch to Stereo Mode From Default
            if (mOmni.activeStereo()) {
                cout << "ACTIVE STEREO " << endl; 
                Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
            }

            //Window::create() method
            create();

            if (mOmni.fullScreen()) {
                fullScreen(true);
                cursorHide(true);
            }

            if (!bSlave) { 
                if(oscSend().opened()) sendHandshake();
                //mAudioIO.start();
            }

            Main::get().start();
        }


    
    };
}

#endif
