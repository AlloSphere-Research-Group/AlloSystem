#ifndef INCLUDE_AL_GRAPHICS_MESH_VBO_HPP
#define INCLUDE_AL_GRAPHICS_MESH_VBO_HPP

/*  Allocore --
  Multimedia / virtual environment application class library

  Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
  Copyright (C) 2012. The Regents of the University of California.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    Neither the name of the University of California nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  File description:
  An extension of Mesh which allows for easy vertex buffer object usage

  File author:
  Kurt Kaminski, 2015, kurtoon@gmail.com

  Usage:
  VBO's require a GL context to work, so initVBO() should take place in OnCreate,
  or anytime after a window has been initialized. Adding vertices and anything else
  done with a regular Mesh may be done before initWindow().

  Q: What usage symbol should I use?
  A:  GL_STREAM_DRAW when the data will be modified once and used (i.e., drawn) at most a few times.
      GL_STATIC_DRAW when the data will be modified once and used many times.
      GL_DYNAMIC_DRAW when the data will be modified repeatedly and used many times.


  TO DO:
  [ ] support 3d textures
  [ ] test 2d texture map support
  [ ] test all manners of updating and initializing, mixing mesh and meshVBO
  [ ] hard to test performance as I think fpsActual() maxes out at display's refresh rate (~60fps)
  [ ] make a way to change usage (which would require an initVBO call)
  [ ] use mesh's "primitive" rendering mode in draw()

  QUESTIONS:
  [ ] use Graphics::Primitive or GL_[etc]?
  [ ] where to #include in allosystem?
*/

#include "allocore/graphics/al_Mesh.hpp"
#include <iostream>

using namespace std;

namespace al{

class MeshVBO : public Mesh {
public:
  
  GLuint vertId = 0;
  GLuint normalId = 0;
  GLuint colorId = 0;
  GLuint texCoordId = 0;
  GLuint indexId = 0;

  int num_verts = 0;
  int num_indices = 0;
  int vertStride = sizeof(Vec3f);
  int normalStride = sizeof(Vec3f);
  int colorStride = sizeof(Color);
  int texCoordStride = sizeof(TexCoord2);
  int indexStride = sizeof(uint);
  
  bool isInit = false;
  bool isBound = false;
  int gUsage = 0;
 
  bool hasNormals = false;
  bool hasColors = false;
  bool hasTexCoords = false;
  bool hasIndices = false;

  void operator=(const Mesh& parent);
  void operator=(const MeshVBO& parent);
  void initVBO(Mesh& mesh, int usage);
  void initVBO(MeshVBO& mesh);
  void initVBO(int usage);

  // Should I template the pointer types?
  void setVertexData(const Vec3f * vert0x, int total, int usage, int stride=0);
  void setNormalData(const Vec3f * normal0x, int total, int usage, int stride=0);
  void setColorData(const Color * color0x, int total, int usage, int stride=0);
  void setTexCoordData(const TexCoord2 * texCoord0x, int total, int usage, int stride=0);
  void setIndexData(const uint * indices, int total, int usage, int stride=0);

  void updateVBO();
  void updateVBO(Mesh& mesh);
  void updateVBO(MeshVBO& mesh);
  void updateVertexData(const Vec3f * vert0x, int total);
  void updateNormalData(const Vec3f * normal0x, int total);
  void updateColorData(const Color * color0x, int total);
  void updateTexCoordData(const TexCoord2 * texCoord0x, int total);
  void updateIndexData(const uint * indices, int total);

  // void enableColors();
  // void enableNormals();
  // void enableTexCoords();
  // void enableIndices();

  // void disableColors();
  // void disableNormals();
  // void disableTexCoords();
  // void disableIndices();

  // void draw(int drawMode, int first, int total);
  void draw(int drawMode);
  // void drawElements(int drawMode, int amt);
  
  // void drawInstanced(int drawMode, int first, int total, int primCount);
  // void drawElementsInstanced(int drawMode, int amt, int primCount);
  
  void bind();
  void unbind();

  void clear();

};

////////////////////////////////////////////////////////////////////////////////
// Operators
void MeshVBO::operator=(const Mesh& parent)
{
  ((Mesh&)(*this)) = parent;
}

void MeshVBO::operator=(const MeshVBO& parent)
{
  *this = parent;
  vertId = parent.vertId;
  normalId = parent.normalId;
  colorId = parent.colorId;
  texCoordId = parent.texCoordId;
  indexId = parent.indexId;

  num_verts = parent.num_verts;
  num_indices = parent.num_indices;
  
  isInit = parent.isInit;
  isBound = parent.isBound;
  gUsage = parent.gUsage;
  
  hasNormals = parent.hasNormals;
  hasColors = parent.hasColors;
  hasTexCoords = parent.hasTexCoords;
  hasIndices = parent.hasIndices;
}

////////////////////////////////////////////////////////////////////////////////
// Initialize
void MeshVBO::initVBO(Mesh& mesh, int usage){
  ((Mesh&)(*this)) = mesh;
  initVBO(usage);
}

void MeshVBO::initVBO(MeshVBO& mesh){
  *this = mesh;
  initVBO(this->gUsage);
}

void MeshVBO::initVBO(int usage){

  if (normals().size()) hasNormals = true;
  if (colors().size()) hasColors = true;
  if (texCoord2s().size()) hasTexCoords = true;
  if (indices().size()) hasIndices = true;

  gUsage = usage;

  // load vertices
  setVertexData(vertices().elems(), vertices().size(), usage, vertStride);

  if (hasNormals) {
    setNormalData(normals().elems(), normals().size(), usage, normalStride);
  }

  if (hasColors) {
    setColorData(colors().elems(), colors().size(), usage, colorStride);
  }

  if (hasTexCoords) {
    setTexCoordData(texCoord2s().elems(), texCoord2s().size(), usage, texCoordStride);
  }

  if (hasIndices) {
    setIndexData(indices().elems(), indices().size(), usage, indexStride);
  }

  isInit = true;
}

void MeshVBO::setVertexData(const Vec3f * vert0x, int total, int usage, int stride){
  /* Interleaved geometry example (not being used):  
   *    0, 1, 2 = vertex_xyz 
   *    3, 4, 5 = normal_xyz
   *    6, 7 = tex_coord_uv
   */
  num_verts = total;
  glGenBuffers(1, &vertId);
  glBindBuffer(GL_ARRAY_BUFFER, vertId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, vert0x, usage); 
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void MeshVBO::setNormalData(const Vec3f * normal0x, int total, int usage, int stride){
  glGenBuffers(1, &normalId);
  glBindBuffer(GL_ARRAY_BUFFER, normalId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, normal0x, usage); 
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void MeshVBO::setColorData(const Color * color0x, int total, int usage, int stride){
  glGenBuffers(1, &colorId);
  glBindBuffer(GL_ARRAY_BUFFER, colorId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, color0x, usage); 
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void MeshVBO::setTexCoordData(const TexCoord2 * texCoord0x, int total, int usage, int stride){
  glGenBuffers(1, &texCoordId);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, texCoord0x, usage); 
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void MeshVBO::setIndexData(const uint * index0x, int total, int usage, int stride){
  num_indices = total;
  glGenBuffers(1, &indexId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, total * stride, index0x, usage);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

////////////////////////////////////////////////////////////////////////////////
// Update buffers using preexisting Mesh data
void MeshVBO::updateVBO(Mesh& mesh){
  ((Mesh&)(*this)) = mesh;
  updateVBO();
}

////////////////////////////////////////////////////////////////////////////////
// Update buffers using preexisting VBO data
void MeshVBO::updateVBO(MeshVBO& mesh){
  *this = mesh;
  updateVBO();
}

////////////////////////////////////////////////////////////////////////////////
// Update buffers. Setting starting position to 0 writes over preexisting data
void MeshVBO::updateVBO(){
  //might think about states in mesh class which keep track of what has changed
  if (vertices().size() > num_verts){
    // having to do this if the vert size is bigger for some reason
    initVBO(gUsage);
  }
  else {
    if (normals().size()) hasNormals = true;
    if (colors().size()) hasColors = true;
    if (texCoord2s().size()) hasTexCoords = true;
    if (indices().size()) hasIndices = true;

    num_verts = vertices().size();
    updateVertexData(vertices().elems(), vertices().size());

    if (hasNormals) {
      if (vertId!=0) updateNormalData(normals().elems(), normals().size());
      else setNormalData(normals().elems(), normals().size(), gUsage, normalStride);
    }

    if (hasColors) {
      if (colorId!=0) updateColorData(colors().elems(), colors().size());
      else setColorData(colors().elems(), colors().size(), gUsage, colorStride);
    } 

    if (hasTexCoords) {  
      if (texCoordId!=0) updateTexCoordData(texCoord2s().elems(), texCoord2s().size());
      else setTexCoordData(texCoord2s().elems(), texCoord2s().size(), gUsage, texCoordStride);
    }

    if (hasIndices) {
      if (indexId!=0) updateIndexData(indices().elems(), indices().size());
      else setIndexData(indices().elems(), indices().size(), gUsage, indexStride);
    }
  }
}

void MeshVBO::updateVertexData(const Vec3f * vert0x, int total){
  if (vertId!=0){
    num_verts = total;
    glBindBuffer(GL_ARRAY_BUFFER, vertId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * vertStride, vert0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  else cout << "Mesh_VBO.updateVertexData(): no vertId" << endl;
}

void MeshVBO::updateNormalData(const Vec3f * normal0x, int total){
  if(normalId!=0) {
    glBindBuffer(GL_ARRAY_BUFFER, normalId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * normalStride, normal0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  else cout << "Mesh_VBO.updateVertexData(): no normalId" << endl;
}

void MeshVBO::updateColorData(const Color * color0x, int total){
  if(colorId!=0) {
    glBindBuffer(GL_ARRAY_BUFFER, colorId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * colorStride, color0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  else cout << "Mesh_VBO.updateVertexData(): no colorId" << endl;
}

void MeshVBO::updateTexCoordData(const TexCoord2 * texCoord0x, int total){
  if(texCoordId!=0) {
    glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * texCoordStride, texCoord0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  else cout << "Mesh_VBO.updateVertexData(): no texCoordId" << endl;
}

void MeshVBO::updateIndexData(const uint * index0x, int total){
  if(indexId!=0) {
    num_indices = total;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, total * indexStride, index0x);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  else cout << "Mesh_VBO.updateVertexData(): no indexId" << endl;
}




////////////////////////////////////////////////////////////////////////////////
// Bind buffer and get necessary pointer for drawing
void MeshVBO::bind(){
  glBindBuffer(GL_ARRAY_BUFFER, vertId);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, vertStride, 0);

  if (hasNormals){
    glBindBuffer(GL_ARRAY_BUFFER, normalId);
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, normalStride, 0);
  }

  if (hasTexCoords){
    glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(3, GL_FLOAT, texCoordStride, 0);
  }

  if (hasColors){
    glBindBuffer(GL_ARRAY_BUFFER, colorId);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, colorStride, 0);
  }

  if (hasIndices){
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
  }

  isBound = true;
}


////////////////////////////////////////////////////////////////////////////////
// Unbind
void MeshVBO::unbind(){
  glBindBuffer(GL_ARRAY_BUFFER, 0); // clean up
  if (hasIndices) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

  glDisableClientState(GL_VERTEX_ARRAY);
  if (hasNormals) glDisableClientState(GL_NORMAL_ARRAY);
  if (hasColors) glDisableClientState(GL_COLOR_ARRAY);
  if (hasTexCoords) glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  isBound = false;
}


////////////////////////////////////////////////////////////////////////////////
// Draw
void MeshVBO::draw(int drawMode){

  if (!isBound) bind();
  
  if (!hasIndices) glDrawArrays(drawMode, 0, num_verts);
  else glDrawElements(drawMode, num_indices, GL_UNSIGNED_INT, NULL);

  if (isBound) unbind();

}

////////////////////////////////////////////////////////////////////////////////
// Clear
void MeshVBO::clear(){
  vertId = 0;
  normalId = 0;
  colorId = 0;
  texCoordId = 0;
  indexId = 0;

  num_verts = 0;
  num_indices = 0;
  
  isInit = false;
  isBound = false;
  gUsage = 0;
  
  hasNormals = false;
  hasColors = false;
  hasTexCoords = false;
  hasIndices = false;
}

} // al::


#endif
