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
void MeshVBO::copyFrom(Mesh& cpy, bool _allocate){
  clear();
  ((Mesh&)(*this)) = cpy;
  if (_allocate) allocate();
}

void MeshVBO::copyFrom(MeshVBO& cpy){
  ((Mesh&)(*this)) = cpy;

  mVertId = cpy.mVertId;
  mNormalId = cpy.mNormalId;
  mColorId = cpy.mColorId;
  mColoriId = cpy.mColoriId;
  mTexCoordId = cpy.mTexCoordId;
  mIndexId = cpy.mIndexId;

  mNumVertices = cpy.mNumVertices;
  mNumIndices = cpy.mNumIndices;

  mAllocated = cpy.mAllocated;
  mBound = cpy.mBound;
  mBufferUsage = cpy.mBufferUsage;

  mHasNormals = cpy.mHasNormals;
  mHasColors = cpy.mHasColors;
  mHasColoris = cpy.mHasColoris;
  mHasTexCoord2s = cpy.mHasTexCoord2s;
  mHasTexCoord3s = cpy.mHasTexCoord3s;
  mHasIndices = cpy.mHasIndices;
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
// Allocate GPU space for buffers
/* Interleaved geometry example (not being used):
 *    0, 1, 2 = vertex_xyz
 *    3, 4, 5 = normal_xyz
 *    6, 7 = tex_coord_uv
 */
void MeshVBO::allocate(BufferDataUsage usage){
  if (!vertices().size()) {
    printf("MeshVBO::allocate(): no vertices to allocate\n");
    return;
  }
  else {
    clear();

    if (normals().size()) mHasNormals = true;
    if (colors().size()) mHasColors = true;
    if (coloris().size()) mHasColoris = true;
    if (texCoord2s().size()) mHasTexCoord2s = true;
    if (texCoord3s().size()) mHasTexCoord3s = true;
    if (indices().size()) mHasIndices = true;

    mBufferUsage = usage;

    // load vertices
    mVertStride = sizeof(vertices()[0]);
    mNumVertices = vertices().size();
    setData(vertices().elems(), &mVertId, vertices().size(), usage, GL_ARRAY_BUFFER);

    if (hasNormals()) {
      mNormalStride = sizeof(normals()[0]);
      setData(normals().elems(), &mNormalId, normals().size(), usage, GL_ARRAY_BUFFER);
    }

    if (hasColors()) {
      mColorStride = sizeof(colors()[0]);
      setData(colors().elems(), &mColorId, colors().size(), usage, GL_ARRAY_BUFFER);
    }

    if (hasColoris()) {
      mColoriStride = sizeof(coloris()[0]);
      setData(coloris().elems(), &mColoriId, coloris().size(), usage, GL_ARRAY_BUFFER);
    }

    if (hasTexCoord2s()) {
      mTexCoordStride = sizeof(texCoord2s()[0]);
      setData(texCoord2s().elems(), &mTexCoordId, texCoord2s().size(), usage, GL_ARRAY_BUFFER);
    }
    else if (hasTexCoord3s()) {
      mTexCoordStride = sizeof(texCoord3s()[0]);
      setData(texCoord3s().elems(), &mTexCoordId, texCoord3s().size(), usage, GL_ARRAY_BUFFER);
    }

    if (hasIndices()) {
      mIndexStride = sizeof(indices()[0]);
      mNumIndices = indices().size();
      setData(indices().elems(), &mIndexId, indices().size(), usage, GL_ELEMENT_ARRAY_BUFFER);
    }

    mAllocated = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Update buffers
void MeshVBO::update(){
  if (vertices().size() > mNumVertices || !isAllocated()){
    allocate();
  }
  else {
    if (normals().size()) mHasNormals = true;
    if (colors().size()) mHasColors = true;
    if (coloris().size()) mHasColoris = true;
    if (texCoord2s().size()) mHasTexCoord2s = true;
    if (texCoord3s().size()) mHasTexCoord3s = true;
    if (indices().size()) mHasIndices = true;

    mNumVertices = vertices().size();
    updateData(vertices().elems(), &mVertId, vertices().size(), GL_ARRAY_BUFFER);
    //
    if (hasNormals()) {
      if (mNormalId!=0) updateData(normals().elems(), &mNormalId, normals().size(), GL_ARRAY_BUFFER);
      else setData(normals().elems(), &mNormalId, normals().size(), mBufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasColors()) {
      if (mColorId!=0) updateData(colors().elems(), &mColorId, colors().size(), GL_ARRAY_BUFFER);
      else setData(colors().elems(), &mColorId, colors().size(), mBufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasColoris()) {
      if (mColoriId!=0) updateData(coloris().elems(), &mColoriId, coloris().size(), GL_ARRAY_BUFFER);
      else setData(coloris().elems(), &mColoriId, coloris().size(), mBufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasTexCoord2s()) {
      if (mTexCoordId!=0) updateData(texCoord2s().elems(), &mTexCoordId, texCoord2s().size(), GL_ARRAY_BUFFER);
      else setData(texCoord2s().elems(), &mTexCoordId, texCoord2s().size(), mBufferUsage, GL_ARRAY_BUFFER);
    }
    else if (hasTexCoord3s()) {
      if (mTexCoordId!=0) updateData(texCoord3s().elems(), &mTexCoordId, texCoord3s().size(), GL_ARRAY_BUFFER);
      else setData(texCoord3s().elems(), &mTexCoordId, texCoord3s().size(), mBufferUsage, GL_ARRAY_BUFFER);
    }

    if (hasIndices()) {
      mNumIndices = indices().size();
      if (mIndexId!=0) updateData(indices().elems(), &mIndexId, indices().size(), GL_ARRAY_BUFFER);
      else setData(indices().elems(), &mIndexId, indices().size(), mBufferUsage, GL_ELEMENT_ARRAY_BUFFER);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Set data, creating new buffers
template <class T>
void MeshVBO::setData(const T *src, uint32_t *bufferId, int total, BufferDataUsage usage, int bufferTarget) {
  glGenBuffers(1, bufferId);
  glBindBuffer(bufferTarget, *bufferId);
  glBufferData(bufferTarget, total * sizeof(T), src, usage);
  glBindBuffer(bufferTarget, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Update data, resuing buffers, starting at index 0 to write over preexisting data
template <class T>
void MeshVBO::updateData(const T *src, uint32_t *bufferId, int total, int bufferTarget){
  if (*bufferId!=0){
    glBindBuffer(bufferTarget, *bufferId);
    glBufferSubData(bufferTarget, 0, total * sizeof(T), src);
    glBindBuffer(bufferTarget, 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Bind buffers and get pointers
void MeshVBO::bind(){
  if (vertices().size() && !mAllocated) {
    allocate();
  }

  glBindBuffer(GL_ARRAY_BUFFER, mVertId);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  if (hasNormals()){
    glBindBuffer(GL_ARRAY_BUFFER, mNormalId);
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);
  }

  if (hasTexCoord2s()){
    glBindBuffer(GL_ARRAY_BUFFER, mTexCoordId);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
  }
  else if (hasTexCoord3s()){
    glBindBuffer(GL_ARRAY_BUFFER, mTexCoordId);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(3, GL_FLOAT, 0, 0);
  }

  if (hasColors()){
    glBindBuffer(GL_ARRAY_BUFFER, mColorId);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, 0);
  }

  if (hasColoris()){
    glBindBuffer(GL_ARRAY_BUFFER, mColoriId);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
  }

  if (hasIndices()){
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexId);
  }

  mBound = true;
}


////////////////////////////////////////////////////////////////////////////////
// Unbind
void MeshVBO::unbind(){
  glBindBuffer(GL_ARRAY_BUFFER, 0); // clean up
  if (hasIndices()) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDisableClientState(GL_VERTEX_ARRAY);
  if (hasNormals()) glDisableClientState(GL_NORMAL_ARRAY);
  if (hasColors() || hasColoris()) glDisableClientState(GL_COLOR_ARRAY);
  if (hasTexCoord2s() || hasTexCoord3s()) glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  mBound = false;

}

////////////////////////////////////////////////////////////////////////////////
// Clear
void MeshVBO::clear(){
  mVertId = 0;
  mNormalId = 0;
  mColorId = 0;
  mColoriId = 0;
  mTexCoordId = 0;
  mIndexId = 0;

  mNumVertices = 0;
  mNumIndices = 0;

  mAllocated = false;
  mBound = false;
  mBufferUsage = STREAM_DRAW;

  mHasNormals = false;
  mHasColors = false;
  mHasColoris = false;
  mHasTexCoord2s = false;
  mHasTexCoord3s = false;
  mHasIndices = false;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t MeshVBO::getVertId() {
  return mVertId;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t MeshVBO::getNormalId() {
  return mNormalId;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t MeshVBO::getColorId() {
  return mColorId;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t MeshVBO::getColoriId() {
  return mColoriId;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t MeshVBO::getTexCoordId() {
  return mTexCoordId;
}

////////////////////////////////////////////////////////////////////////////////
uint32_t MeshVBO::getIndexId() {
  return mIndexId;
}

////////////////////////////////////////////////////////////////////////////////
int MeshVBO::getNumVertices() {
  return mNumVertices;
}

////////////////////////////////////////////////////////////////////////////////
int MeshVBO::getNumIndices() {
  return mNumIndices;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::isAllocated() {
  return mAllocated;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::isBound() {
  return mBound;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::hasNormals() {
  return mHasNormals;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::hasColors() {
  return mHasColors;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::hasColoris() {
  return mHasColoris;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::hasTexCoord2s() {
  return mHasTexCoord2s;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::hasTexCoord3s() {
  return mHasTexCoord3s;
}

////////////////////////////////////////////////////////////////////////////////
bool MeshVBO::hasIndices() {
  return mHasIndices;
}

} // al::
