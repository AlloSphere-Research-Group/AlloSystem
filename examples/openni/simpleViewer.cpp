/*
OpenNI SimpleViewer example ported to AlloSystem

Reads depth and RGB data from a supported depth sensor (i.e. Kinect, primeSense)

Tim Wood 2015
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shapes.hpp"


#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>

// openni MACRO to print error message on failure
#define CHECK_RC(rc, what)                      \
  if (rc != XN_STATUS_OK)                     \
  {                               \
    printf("%s failed: %s\n", what, xnGetStatusString(rc));   \
  }

#define DISPLAY_MODE_OVERLAY  1
#define DISPLAY_MODE_DEPTH    2
#define DISPLAY_MODE_IMAGE    3
#define DEFAULT_DISPLAY_MODE  DISPLAY_MODE_DEPTH

using namespace al;
using namespace xn;


class OpenniApp : public App{
public:

  Texture tex;

  float* pDepthHist;
  XnRGB24Pixel* pTexMap = NULL;
  unsigned int nTexMapX = 0;
  unsigned int nTexMapY = 0;
  XnDepthPixel nZRes;

  unsigned int nViewState = DEFAULT_DISPLAY_MODE;

  Context context;
  ScriptNode scriptNode;
  DepthGenerator depth;
  ImageGenerator image;
  DepthMetaData depthMD;
  ImageMetaData imageMD;

  OpenniApp(){

    // Configure the camera lens
    lens().near(0.1).far(25).fovy(45);

    // Set navigation position and orientation
    nav().pos(0,0,4);
    nav().quat().fromAxisAngle(0.*M_2PI, 0,1,0);

    // Initialize a single window; anything in App::onDraw will be rendered
    // Arguments: position/dimensions, title, frames/second
    initWindow(Window::Dim(0,0, 600,400), "SimpleViewer", 40);
    
    // Uncomment this to disable the default navigation keyboard/mouse controls
    //window().remove(navControl());


    // initialize OpenNI Context and image generators
    XnStatus nRetVal = XN_STATUS_OK;

    nRetVal = context.Init();
    CHECK_RC(nRetVal, "Initialize context");

    nRetVal = depth.Create(context);
    CHECK_RC(nRetVal, "Create depth generator");

    nRetVal = image.Create(context);
    CHECK_RC(nRetVal, "Create image generator");

    nRetVal = context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGeneratingAll");

    depth.GetMetaData(depthMD);
    image.GetMetaData(imageMD);

    // Texture map init
    nTexMapX = (((unsigned short)(depthMD.FullXRes()-1) / 512) + 1) * 512;
    nTexMapY = (((unsigned short)(depthMD.FullYRes()-1) / 512) + 1) * 512;
    // pTexMap = (XnRGB24Pixel*)malloc(nTexMapX * nTexMapY * sizeof(XnRGB24Pixel));

    nZRes = depthMD.ZRes();
    pDepthHist = (float*)malloc(nZRes * sizeof(float));

    tex.format(Graphics::RGB);
    tex.type(Graphics::UBYTE);
    tex.resize(nTexMapX,nTexMapY);
    tex.allocate();

    // pointer to texture pixel data 
    pTexMap = tex.data<XnRGB24Pixel>();

    printHelp();

  }

  virtual void onAnimate(double dt){

    XnStatus rc = XN_STATUS_OK;

    // Read a new frame
    rc = context.WaitAnyUpdateAll();
    CHECK_RC(rc, "WaitAnyUpdateAll");

    depth.GetMetaData(depthMD);
    image.GetMetaData(imageMD);

    const XnDepthPixel* pDepth = depthMD.Data();

    // Calculate the accumulative histogram (the yellow display...)
    xnOSMemSet(pDepthHist, 0, nZRes*sizeof(float));

    unsigned int nNumberOfPoints = 0;
    for (XnUInt y = 0; y < depthMD.YRes(); ++y)
    {
      for (XnUInt x = 0; x < depthMD.XRes(); ++x, ++pDepth)
      {
        if (*pDepth != 0)
        {
          pDepthHist[*pDepth]++;
          nNumberOfPoints++;
        }
      }
    }
    for (int nIndex=1; nIndex<nZRes; nIndex++)
    {
      pDepthHist[nIndex] += pDepthHist[nIndex-1];
    }
    if (nNumberOfPoints)
    {
      for (int nIndex=1; nIndex<nZRes; nIndex++)
      {
        pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (pDepthHist[nIndex] / nNumberOfPoints)));
      }
    }

    xnOSMemSet(pTexMap, 0, nTexMapX*nTexMapY*sizeof(XnRGB24Pixel));

    // check if we need to draw image frame to texture
    if (nViewState == DISPLAY_MODE_OVERLAY ||
      nViewState == DISPLAY_MODE_IMAGE)
    {
      const XnRGB24Pixel* pImageRow = imageMD.RGB24Data();
      XnRGB24Pixel* pTexRow = pTexMap + imageMD.YOffset() * nTexMapX;

      for (XnUInt y = 0; y < imageMD.YRes(); ++y)
      {
        const XnRGB24Pixel* pImage = pImageRow;
        XnRGB24Pixel* pTex = pTexRow + imageMD.XOffset();

        for (XnUInt x = 0; x < imageMD.XRes(); ++x, ++pImage, ++pTex)
        {
          *pTex = *pImage;
        }

        pImageRow += imageMD.XRes();
        pTexRow += nTexMapX;
      }
    }

    // check if we need to draw depth frame to texture
    if (nViewState == DISPLAY_MODE_OVERLAY ||
      nViewState == DISPLAY_MODE_DEPTH)
    {
      const XnDepthPixel* pDepthRow = depthMD.Data();
      XnRGB24Pixel* pTexRow = pTexMap + depthMD.YOffset() * nTexMapX;

      for (XnUInt y = 0; y < depthMD.YRes(); ++y)
      {
        const XnDepthPixel* pDepth = pDepthRow;
        XnRGB24Pixel* pTex = pTexRow + depthMD.XOffset();

        for (XnUInt x = 0; x < depthMD.XRes(); ++x, ++pDepth, ++pTex)
        {
          if (*pDepth != 0)
          {
            int nHistValue = pDepthHist[*pDepth];
            pTex->nRed = nHistValue;
            pTex->nGreen = nHistValue;
            pTex->nBlue = 0;
          }
        }

        pDepthRow += depthMD.XRes();
        pTexRow += nTexMapX;
      }
    }

    // mark our texture as dirty so it will be updated on gpu
    tex.dirty();
  }


  virtual void onDraw(Graphics& g, const Viewpoint& v){
    
    // Borrow a temporary Mesh from Graphics
    Mesh& m = g.mesh();

    m.reset();

    // Generate geometry
    m.primitive(Graphics::TRIANGLE_STRIP);
    m.vertex(-1,  1);
    m.vertex(-1, -1);
    m.vertex( 1,  1);
    m.vertex( 1, -1);

    // Add texture coordinates
    m.texCoord(0,0);
    m.texCoord(0,1);
    m.texCoord(1,0);
    m.texCoord(1,1);

    g.pushMatrix();
    g.scale( tex.width() / tex.height(), 1, 1);
    tex.bind();
      g.draw(m);
    tex.unbind();
    g.popMatrix();

  }


  virtual void onKeyDown(const ViewpointWindow& w, const Keyboard& k){
  
    switch(k.key()){

    case '1':
      nViewState = DISPLAY_MODE_OVERLAY;
      depth.GetAlternativeViewPointCap().SetViewPoint(image);
      break;
    case '2':
      nViewState = DISPLAY_MODE_DEPTH;
      depth.GetAlternativeViewPointCap().ResetViewPoint();
      break;
    case '3':
      nViewState = DISPLAY_MODE_IMAGE;
      depth.GetAlternativeViewPointCap().ResetViewPoint();
      break;
    case 'm':
      context.SetGlobalMirror(!context.GetGlobalMirror());
      break;
    }
  }

  void printHelp(){
    std::cout << "Simple OpenNI viewer \n";
    std::cout << "Keys: \n";
    std::cout << "  1 Depth overlaid on top of rgb data\n";
    std::cout << "  2 Depth Image\n";
    std::cout << "  3 RGB Image\n";
    std::cout << "  4 toggle mirroring\n";
  }

};


int main(){
  OpenniApp().start();
}
