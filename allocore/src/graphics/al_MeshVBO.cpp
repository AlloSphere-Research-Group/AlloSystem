#include "allocore/graphics/al_MeshVBO.hpp"

namespace al{

////////////////////////////////////////////////////////////////////////////////
// Operators
void MeshVBO::operator=(const Mesh& parent)
{
  ((Mesh&)(*this)) = parent;
  initVBO();
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
  bufferUsage = parent.bufferUsage;

  hasNormals = parent.hasNormals;
  hasColors = parent.hasColors;
  hasTexCoords = parent.hasTexCoords;
  hasIndices = parent.hasIndices;
}

////////////////////////////////////////////////////////////////////////////////
// Initialize
// void MeshVBO::initVBO(BufferDataUsage usage){
void MeshVBO::initVBO(){

  if (normals().size()) hasNormals = true;
  if (colors().size()) hasColors = true;
  if (texCoord2s().size()) hasTexCoords = true;
  if (indices().size()) hasIndices = true;

  BufferDataUsage usage = STATIC_DRAW;
  bufferUsage = usage;

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

void MeshVBO::setVertexData(const Vec3f * vert0x, int total, BufferDataUsage usage, int stride){
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

void MeshVBO::setNormalData(const Vec3f * normal0x, int total, BufferDataUsage usage, int stride){
  glGenBuffers(1, &normalId);
  glBindBuffer(GL_ARRAY_BUFFER, normalId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, normal0x, usage);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::setColorData(const Color * color0x, int total, BufferDataUsage usage, int stride){
  glGenBuffers(1, &colorId);
  glBindBuffer(GL_ARRAY_BUFFER, colorId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, color0x, usage);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::setTexCoordData(const TexCoord2 * texCoord0x, int total, BufferDataUsage usage, int stride){
  glGenBuffers(1, &texCoordId);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
  glBufferData(GL_ARRAY_BUFFER, total * stride, texCoord0x, usage);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshVBO::setIndexData(const uint * index0x, int total, BufferDataUsage usage, int stride){
  num_indices = total;
  glGenBuffers(1, &indexId);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, total * stride, index0x, usage);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

////////////////////////////////////////////////////////////////////////////////
// Update buffers. Setting starting position to 0 writes over preexisting data
void MeshVBO::updateVBO(){
  //might think about states in mesh class which keep track of what has changed
  if (vertices().size() > num_verts){
    // having to do this if the vert size is bigger for some reason
    initVBO(bufferUsage);
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
      else setNormalData(normals().elems(), normals().size(), bufferUsage, normalStride);
    }

    if (hasColors) {
      if (colorId!=0) updateColorData(colors().elems(), colors().size());
      else setColorData(colors().elems(), colors().size(), bufferUsage, colorStride);
    }

    if (hasTexCoords) {
      if (texCoordId!=0) updateTexCoordData(texCoord2s().elems(), texCoord2s().size());
      else setTexCoordData(texCoord2s().elems(), texCoord2s().size(), bufferUsage, texCoordStride);
    }

    if (hasIndices) {
      if (indexId!=0) updateIndexData(indices().elems(), indices().size());
      else setIndexData(indices().elems(), indices().size(), bufferUsage, indexStride);
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
  // else cout << "Mesh_VBO.updateVertexData(): no vertId" << endl;
}

void MeshVBO::updateNormalData(const Vec3f * normal0x, int total){
  if(normalId!=0) {
    glBindBuffer(GL_ARRAY_BUFFER, normalId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * normalStride, normal0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  // else cout << "Mesh_VBO.updateVertexData(): no normalId" << endl;
}

void MeshVBO::updateColorData(const Color * color0x, int total){
  if(colorId!=0) {
    glBindBuffer(GL_ARRAY_BUFFER, colorId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * colorStride, color0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  // else cout << "Mesh_VBO.updateVertexData(): no colorId" << endl;
}

void MeshVBO::updateTexCoordData(const TexCoord2 * texCoord0x, int total){
  if(texCoordId!=0) {
    glBindBuffer(GL_ARRAY_BUFFER, texCoordId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, total * texCoordStride, texCoord0x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  // else cout << "Mesh_VBO.updateVertexData(): no texCoordId" << endl;
}

void MeshVBO::updateIndexData(const uint * index0x, int total){
  if(indexId!=0) {
    num_indices = total;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, total * indexStride, index0x);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  // else cout << "Mesh_VBO.updateVertexData(): no indexId" << endl;
}




////////////////////////////////////////////////////////////////////////////////
// Bind buffer and get necessary pointer for drawing
void MeshVBO::bindVBO(){
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
// UnbindVBO
void MeshVBO::unbindVBO(){
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
// void MeshVBO::draw(int drawMode){
//
//   if (!isBound) bind();
//
//   if (!hasIndices) glDrawArrays(drawMode, 0, num_verts);
//   else glDrawElements(drawMode, num_indices, GL_UNSIGNED_INT, NULL);
//
//   if (isBound) unbindVBO();
//
// }

////////////////////////////////////////////////////////////////////////////////
// Clear
void MeshVBO::clearVBO(){
  normalId = 0;  vertId = 0;

  colorId = 0;
  texCoordId = 0;
  indexId = 0;

  num_verts = 0;
  num_indices = 0;

  isInit = false;
  isBound = false;
  bufferUsage = STREAM_DRAW;

  hasNormals = false;
  hasColors = false;
  hasTexCoords = false;
  hasIndices = false;
}

} // al::
