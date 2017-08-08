#pragma once

/*  Order Independent Transparency (OIT)
        Original algorithm by Morgan McGuire
        AlloSystem version written by Keehong Youn
        younkeehong@gmail.com
        
        - Uses OpenGL compatibility profile
        - Provides transparency without sorting of the objects
        - By default uses flat non-lighting shader
            Override "vertCode" & "fragCode" to use custom shader

    use:
        OITFbo oit;
        oit.init(w, h);

        oit.beginOpaque();
        // render opaque things
        oit.endOpaque();

        oit.beginAccum();
        // render transparent things
        oit.endAccum();

        oit.beginReveal();
        // render transparent things
        oit.endReveal();

        oit.composite();

        // now use "oit.result()" to get texture with rendered result
    
    Reference:
        - http://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html
        - http://casual-effects.blogspot.com/2015/03/colored-blended-order-independent.html
*/

#include "allocore/graphics/al_OpenGL.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_FBO.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"

#include <string>

namespace al {

class OITFbo {
public:
// private:
    // three render target: opaque, accum, and reveal
    // four shader pass: opaque, accum, reveal, and composite
    static const int opaque = 0;
    static const int accum = 1;
    static const int reveal = 2;
    static const int comp = 3;

    int w, h;

    FBO fbo; // one FBO with
    Texture tex[3]; // three color attachment textures and
    Texture tex_depth; // one common depth texture

    ShaderProgram shader[4];
    Graphics g;

public:
    OITFbo() {};
    ~OITFbo() {};

    void init(int _w, int _h) {
        w = _w;
        h = _h;

        tex_depth = Texture(w, h, Graphics::DEPTH_COMPONENT, Graphics::FLOAT);
        tex_depth.submit(); // this actually makes texture registered at gpu
        fbo.attachTexture2D(tex_depth.id(), FBO::DEPTH_ATTACHMENT);

        for (int i = 0; i < 3; i++) {
          tex[i] = Texture(w, h, Graphics::RGBA, Graphics::FLOAT);
          tex[i].submit();
        }

        // would be nice if could FBO::COLOR_ATTACHMENT0 + i, but can't
        fbo.attachTexture2D(tex[opaque].id(), FBO::COLOR_ATTACHMENT0);
        fbo.attachTexture2D(tex[accum].id(), FBO::COLOR_ATTACHMENT1);
        fbo.attachTexture2D(tex[reveal].id(), FBO::COLOR_ATTACHMENT2);

        shader[opaque].compile(vertRenderCode().c_str(), fragOpaqueCode().c_str());
        shader[accum].compile(vertRenderCode().c_str(), fragAccumCode().c_str());
        shader[reveal].compile(vertRenderCode().c_str(), fragRevealCode().c_str());
        shader[comp].compile(vertCompositeCode().c_str(), fragCompsiteCode().c_str());

        ShaderProgram& c = shader[comp];
        c.begin();
        c.uniform("tex_accum", 0);
        c.uniform("tex_reveal", 1);
        c.end();
    }

    void beginOpaque() {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        bindFBO();
        renderToOpaque();
        clearDepth();
        shader[opaque].begin();
    }
    
    void endOpaque() {
        shader[opaque].end();
        unbindFBO();
    }

    void beginAccum() {
        glDepthMask(GL_FALSE); // depth test enabled but not masking
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // src, dst
        bindFBO();
        renderToAccum();
        clearColor(); // but don't clear depth from opaque rendering
        shader[accum].begin();
    }

    void endAccum() {
        shader[accum].end();
        unbindFBO();
    }

    void beginReveal() {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        bindFBO();
        renderToReveal();
        clearColor();
        shader[reveal].begin();
    }

    void endReveal() {
        shader[reveal].end();
        unbindFBO();
    }

    void composite() {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        bindFBO();
        renderToOpaque();
        shader[comp].begin();
        tex[accum].bind(0);
        tex[reveal].bind(1);
        quadViewport(); // push pixels for entire screen
        tex[accum].unbind(0); // it is important to unbind
        tex[reveal].unbind(1);
        shader[comp].end();
        unbindFBO();
    }

    ShaderProgram& opaqueShader() {
        return shader[opaque];
    }

    ShaderProgram& accumShader() {
        return shader[accum];
    }

    ShaderProgram& revealShader() {
        return shader[reveal];
    }

    ShaderProgram& compShader() {
        return shader[comp];
    }

    Texture& result() {
        return tex[opaque];
    }

    virtual std::string vertRenderCode();
    virtual std::string fragOpaqueCode();
    virtual std::string fragAccumCode();
    virtual std::string fragRevealCode();

    virtual std::string vertCompositeCode();
    virtual std::string fragCompsiteCode();

private:
    void bindFBO() { fbo.begin(); }
    void unbindFBO() { fbo.end(); }
    void renderToOpaque() { glDrawBuffer(GL_COLOR_ATTACHMENT0); }
    void renderToAccum() { glDrawBuffer(GL_COLOR_ATTACHMENT1); }
    void renderToReveal() { glDrawBuffer(GL_COLOR_ATTACHMENT2); }

    void quadViewport();
    void clearColor(float r=0, float g=0, float b=0, float a=0);
    void clearDepth(float d=1);
};

inline std::string OITFbo::vertRenderCode() {
    return R"(
varying vec4 color;
void main(){
    color = gl_Color;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";
}

inline std::string OITFbo::fragOpaqueCode() {
    return R"(
varying vec4 color;
void main(){
    gl_FragColor = color;
}
)";
}

inline std::string OITFbo::fragAccumCode() {
    return R"(
varying vec4 color;
void main() {
    vec4 premult = vec4(color.rgb * color.a, color.a);
    vec3 transmit = color.rgb * (1.0 - color.a);
    premult.a *= 1.0 - clamp((transmit.r + transmit.g + transmit.b) / 3.0, 0.0, 1.0);
    float tmp = (min(premult.a, 1.0) * 8.0 + 0.01) * (1.0 - gl_FragCoord.z * 0.95);
    float w = clamp(tmp * tmp * tmp * 1e3, 1e-2, 3e2);
    gl_FragColor = premult * w / 100.0; // divide by 100 so we write a val less than 1.0
}
)";
}

inline std::string OITFbo::fragRevealCode() {
    return R"(
varying vec4 color;
void main() {
    vec4 premult = vec4(color.rgb * color.a, color.a);
    vec3 transmit = color.rgb * (1.0 - color.a);
    premult.a *= 1.0 - clamp((transmit.r + transmit.g + transmit.b) / 3.0, 0.0, 1.0);
    // all ouput we want is one value but we gotta write to 4 channels
    gl_FragColor = vec4(premult.a);
}
)";
}

inline std::string OITFbo::vertCompositeCode() {
    return R"(
void main(){
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";
}

inline std::string OITFbo::fragCompsiteCode() {
    return R"(
uniform sampler2D tex_accum;
uniform sampler2D tex_reveal;
void main(){
    vec4 accum = texture2D(tex_accum, gl_TexCoord[0].st) * 100.0;
    float revealage = texture2D(tex_reveal, gl_TexCoord[0].st).a;
    gl_FragColor = vec4(accum.rgb / max(accum.a, 0.00001), revealage);
}
)";
}

inline void OITFbo::quadViewport() {
    g.pushMatrix(g.PROJECTION);
    g.loadIdentity();
    g.pushMatrix(g.MODELVIEW);
    g.loadIdentity();
    g.depthMask(0); // write only to color buffer
    al::Mesh& m = g.mesh();
    m.reset();
    m.primitive(g.TRIANGLE_STRIP);
    m.vertex(-1, -1, 0);
    m.texCoord(0, 0);
    m.vertex(1, -1, 0);
    m.texCoord(1, 0);
    m.vertex(-1, 1, 0);
    m.texCoord(0, 1);
    m.vertex(1, 1, 0);
    m.texCoord(1, 1);
    g.draw(m);
    g.depthMask(1);
    g.popMatrix(g.PROJECTION);
    g.popMatrix(g.MODELVIEW);
}

inline void OITFbo::clearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

inline void OITFbo::clearDepth(float d) {
    glClearDepth(d);
    glClear(GL_DEPTH_BUFFER_BIT);
}

}