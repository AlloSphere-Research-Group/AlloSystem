//
//  AlloInterfaceImpl.h
//  vsr
/*
    Glue For passing Window, View, Keyboard, Mouse, state to Versor Library from AlloCore Library.
*/
//  Created by Pablo Colapinto on 3/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef al_vsrInterfaceImpl_included
#define al_vsrInterfaceImpl_included

//ABSTRACT BASE CLASS
#include "allocore/al_Allocore.hpp"
#include "allovsr/al_vsr.hpp"

#include "vsr/vsr_interface.h"
#include "vsr/vsr_gui.h"
#include "alloGLV/al_ControlGLV.hpp"


namespace al {

    using vsr::Frame;
    using vsr::Scene;
    using vsr::Interface;
    using vsr::MouseData;

    //GLUE
    struct VsrView : public vsr::Interface::ViewImpl, public al::WindowEventHandler {

        //initialize with link to an interface controller object
        VsrView(Interface * i) : Interface::ViewImpl(i) {}

        virtual bool onFrame(){
            //Copy Scene View Data (move to impl.onFrame() )
            //vsr::Mat4f t = interface -> scene().mvm();
            gfx::Mat4f t = interface -> scene().mvm();
            std::copy(t.val(), t.val() + 16, interface -> scene().xf.modelView);

//            vsr::Mat4f t2 = interface -> scene().proj();
            gfx::Mat4f t2 = interface -> scene().proj();
            std::copy(t2.val(), t2.val() + 16, interface -> scene().xf.proj);

            interface -> scene().xf.toDoubles();

            interface -> viewCalc();
            return true;
        }

        virtual bool onResize(int dw, int dh){
            interface -> vd().w = window().width();
            interface -> vd().h = window().height();
            interface -> camera().lens().width() = interface -> vd().w;
            interface -> camera().lens().height() = interface -> vd().h;

            //move this to impl.onResize . . .
            interface -> scene().xf.viewport[0] = interface -> scene().xf.viewport[1] = 0;
            interface -> scene().xf.viewport[2] = interface -> vd().w;
            interface -> scene().xf.viewport[3] = interface -> vd().h;

            fit();
            return true;
        }

    };


    struct VsrInput : public Interface::InputImpl, public al::InputEventHandler {

        VsrInput(Interface * i) : Interface::InputImpl(i)  {}

        //get data from al window
        virtual void getKeyboardData(const Keyboard &k){

            //COPY KEYBOARD DATA
            interface -> keyboard.alt      = k.alt();
            interface -> keyboard.shift     = k.shift();
            interface -> keyboard.ctrl     = k.ctrl();
            interface -> keyboard.caps     = k.caps();
            interface -> keyboard.meta     = k.meta();
            interface -> keyboard.code     = k.key();
        }

        virtual void getMouseData(const Mouse& m){

            //COPY MOUSE DATA
            interface -> mouse.x		= m.x();
            interface -> mouse.y		= m.y();

            interface -> mouse.dx		= m.dx() / interface -> vd().w;
            interface -> mouse.dy		= -m.dy() / interface -> vd().h;

        }

        //Pass events to interface controller
        virtual bool onKeyDown(const Keyboard& k ){
            getKeyboardData(k);
            interface->onKeyDown();
            return true;
        }
        virtual bool onKeyUp(const Keyboard& k){
            getKeyboardData(k);
            interface->onKeyUp();
            return true;
        }

        virtual bool onMouseMove(const Mouse& m){
            getMouseData(m);
            interface->onMouseMove();
            return true;
        }
        virtual bool onMouseDown(const Mouse& m){
            getMouseData(m);
            interface->onMouseDown();
            return true;
        }
        virtual bool onMouseUp(const Mouse& m){
            getMouseData(m);
            interface->onMouseUp();
            return true;
        }
        virtual bool onMouseDrag(const Mouse& m){
            getMouseData(m);
            interface->onMouseDrag();
            return true;
        }
    };

    class VsrInterface : public Interface {

    public:

        VsrInterface() : Interface() { init(); }

        VsrView& view() { return *(VsrView*)vimpl; }
        VsrInput& input() { return *(VsrInput*)iimpl; }

        virtual void init(){
            vimpl = new VsrView(this);
            iimpl = new VsrInput(this);

        }

    };

    //A GLV Object with built in GUI for easy prototyping
    struct VsrGui : public GLVDetachable {
        VsrGui() : GLVDetachable(), gui() {
            *this << gui;
            gui.colors().back.set(.3,.3,.3);
        }
        glv::Gui gui;
    };

    /*! Application implementation
    */
    struct VsrApp : public Window, public al::Drawable {

        string description;

        VsrInterface interface;
        VsrGui glv;

        Graphics gl;
        al::Lens lens;

        //Renderer
        Stereoscopic stereo;

        //al::Window has a mouse too so i call mine "imouse". . .
        MouseData& imouse() { return interface.mouse; }
        VsrView& view() { return interface.view(); }

        Scene& scene() { return interface.scene(); }
        vsr::Camera& camera() { return interface.camera(); }
        Frame& model() { return interface.model(); }


        VsrApp() : Window() {
            lens.fovy(60);
            add(new StandardWindowKeyControls);
            add(&interface.input());
            add(&interface.view());
            glv.parentWindow(*this);
            glv.gui.colors().back.set(.3,.3,.3);
        }


        VsrApp(Window * win) : Window() {
            lens.fovy(60);
            add(new StandardWindowKeyControls);
            add(&interface.input());
            add(&interface.view());
            glv.parentWindow(*this);
            glv.gui.colors().back.set(.3,.3,.3);
        }

        virtual void onDraw(Graphics& gl){}

        bool onFrame(){

            gfx :: GL :: enablePreset();
            gl.depthTesting(true);

            //Update Physics
            camera().step();
            model().step();

            glColor3f(1,1,1);

		//if (stereo.stereo()) printf("stereo!\n");

            stereo.draw( gl, lens, Frame2Pose( camera() ), Viewport( width(), height() ), *this );

            gfx :: GL :: disablePreset();

            return true;

        }

//        //scene descriptor
        void text(string s, int ow = 50, int oh = 100){
            //vsr::gl::draw::
            glv::draw::enter2D(width(), height());
                //glTranslated(ow,height()-oh,0);
                glColor3f(1,1,1);
                glv::draw::text( s.c_str() );
            //vsr::gl::draw::
            glv::draw::pop();
        }

        void modelTransform(){
            Rot t = vsr::Gen::aa( scene().model.rot() );
            gfx::GL::rotate( t.w() );
        }

    };


} // al::

#endif
