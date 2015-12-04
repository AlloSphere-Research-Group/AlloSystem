#include "allocore/graphics/al_MeshVBO.hpp"

namespace al{

////////////////////////////////////////////////////////////////////////////////
// Constructors
MeshVBO::MeshVBO(){};

// Copy
MeshVBO::MeshVBO(Mesh& cpy){
  copyFrom(cpy);
}

////////////////////////////////////////////////////////////////////////////////
// Copy from Mesh or MeshVBO
void MeshVBO::copyFrom(Mesh& cpy){
  clear();
  ((Mesh&)(*this)) = cpy;
  init();
}

void MeshVBO::copyFrom(MeshVBO& cpy){
  ((Mesh&)(*this)) = cpy;

  vertId = cpy.vertId;
  normalId = cpy.normalId;
  colorId = cpy.colorId;
  texCoordId = cpy.texCoordId;
  indexId = cpy.indexId;

  num_vertices = cpy.num_vertices;
  num_indices = cpy.num_indices;

  isInit = cpy.isInit;
  isBound = cpy.isBound;
  bufferUsage = cpy.bufferUsage;

  hasNormals = cpy.hasNormals;
  hasColors = cpy.hasColors;
  hasTexCoords = cpy.hasTexCoords;
  hasIndices = cpy.hasIndices;
}

////////////////////////////////////////////////////////////////////////////////
// Operators
void MeshVBO::operator=(Mesh& cpy) {
  copyFrom(cpy);
}

void MeshVBO::operator=(MeshVBO& cpy) {
  copyFrom(cpy);
}

////////////////////////////////////////////////////////////////////////////////
// Initialize
/* Interleaved geometry example (not being used):
 *    0, 1, 2 = vertex_xyz
 *    3, 4, 5 = normal_xyz
 *    6, 7 = tex_coord_uv
 */
void MeshVBO::init(BufferDataUsage usage){
  if (normals().size()) hasNormals = true;
  if (colors().size()) hasColors = true;
  if (texCoord2s().size()) hasTexCoords = true;
  if (indices().size()) hasIndices = true;

  bufferUsage = usage;

  // load vertices
  vertStride = sizeof(vertices()[0]);
  num_vertices = vertices().size();
  setData(vertices().elems(), &vertId, vertices().size(), usage, GL_ARRAY_BUFFER);

  if (hasNormals) {
    normalStride = sizeof(normals()[0]);
    setData(normals().elems(), &normalId, normals().size(), usage, GL_ARRAY_BUFFER);
  }

  if (hasColors) {
    colorStride = sizeof(colors()[0]);
    setData(colors().elems(), &colorId, colors().size(), usage, GL_ARRAY_BUFFER);
  }

  if (hasTexCoords) {
    texCoordStride = sizeof(texCoord2s()[0]);
    setData(texCoord2s().elems(), &texCoordId, texCoord2s().size(), usage, GL_ARRAY_BUFFER);
  }

  if (hasIndices) {
    indexStride = sizeof(indices()[0]);
    num_indices = indices().size();
    setData(indices().elems(), &indexId, indices().size(), usage, GL_ELEMENT_ARRAY_BUFFER);
  }

  isInit = true;
}

template <class T>
void MeshVBO::setData(const T * src, uint * bufferId, int total, BufferDataUsage usage, int bufferTarget) {
  glGenBuffers(1, bufferId);
  glBindBuffer(bufferTarget, *bufferId);
  glBufferData(bufferTarget, total * sizeof(T), src, usage);
  glBindBuffer(bufferTarget, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Update buffers. Setting starting position to 0 writes over preexisting data
void MeshVBO::update(){
  if (vertices().size() > num_vertices){
    // having to do this if the vert size is bigger for some reason
    init();
  }
  else {
    if (normals().size()) hasNormals = true;
    if (colors().size()) hasColors = true;
    if (texCoord2s().size()) hasTexCoords = true;
    if (indices().size()) hasIndices = true;

    num_vertices = vertices().size();
    updateData(vertices().elems(), &vertId, vertices().size(), GL_ARRAY_BUFFER);
    //
    if (hasNormals) {
      if (normalId!=0) updateData(normals().elems(), &normalId, normals().size(), GL_ARRAY_BUFFER);
      else setData(normals().elems(), &normalId, normals().size(), bufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasColors) {
      if (colorId!=0) updateData(colors().elems(), &colorId, colors().size(), GL_ARRAY_BUFFER);
      else setData(colors().elems(), &colorId, colors().size(), bufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasTexCoords) {
      if (texCoordId!=0) updateData(texCoord2s().elems(), &texCoordId, texCoord2s().size(), GL_ARRAY_BUFFER);
      else setData(texCoord2s().elems(), &texCoordId, texCoord2s().size(), bufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasIndices) {
      num_indices = indices().size();
      if (indexId!=0) updateData(indices().elems(), &indexId, indices().size(), GL_ARRAY_BUFFER);
      else setData(indices().elems(), &indexId, indices().size(), bufferUsage, GL_ELEMENT_ARRAY_BUFFER);
    }
  }
}

template <class T>
void MeshVBO::updateData(const T * src, uint * bufferId, int total, int bufferTarget){
  if (*bufferId!=0){
    glBindBuffer(bufferTarget, *bufferId);
    glBufferSubData(bufferTarget, 0, total * sizeof(T), src);
    glBindBuffer(bufferTarget, 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Bind buffers and get pointers
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
  if (hasIndices) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDisableClientState(GL_VERTEX_ARRAY);
  if (hasNormals) glDisableClientState(GL_NORMAL_ARRAY);
  if (hasColors) glDisableClientState(GL_COLOR_ARRAY);
  if (hasTexCoords) glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  isBound = false;

}

////////////////////////////////////////////////////////////////////////////////
// Clear
void MeshVBO::clear(){
  vertId = 0;
  normalId = 0;
  colorId = 0;
  texCoordId = 0;
  indexId = 0;

  num_vertices = 0;
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
