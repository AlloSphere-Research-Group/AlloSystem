

#include "allocore/al_Allocore.hpp"
#include "allocore/sound/al_Vbap.hpp"
//#include "allocore/system/al_Parameter.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "alloutil/al_AlloSphereSpeakerLayout.hpp"
#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Font.hpp"

using namespace al;

#define BLOCK_SIZE (2048)
#define NUM_SOURCES 1
#define NUM_SPEAKS 8

enum{
    RING_LAYOUT,
    THREE_D_LAYOUT,
    ALLO_LAYOUT,
    TEST_LAYOUT
};


bool perSampProcessing = true;
float speedMult = 0.02;
int layout = TEST_LAYOUT;

SpeakerLayout speakerLayout;
Vbap* panner;
AudioScene scene(BLOCK_SIZE);
Listener * listener;
SoundSource srcs[NUM_SOURCES];

float srcElev = 1.f;

Vec3d srcpos(0.0,0.0,0.0);


//Currently bypasses al_AudioScene
inline void perSample(AudioIOData& io){

    static unsigned int t = 0;

    int numFrames = io.framesPerBuffer();

    for(int j=0; j<numFrames; j++){
        double sec = (t / io.fps());
        float fac = sec*2.0*M_PI;

        for(int i = 0; i < NUM_SOURCES; i++){

            //Calculate source position
            float tta = sec*speedMult*2.0*M_PI + (2.0*M_PI/NUM_SOURCES)*(i+1.0);
            float x = 12.0*cos(tta);
            float y = 12.0*sin(tta);

            //Calculate source sample
            float smp = sin(fac*(880*((i/3.0)+1)))*(1.0/(i+1));

            Vec3d pos(x,y,srcElev);
            srcpos = pos;
            smp = srcs[i].attenuation(pos.mag())*smp;
            panner->perform(io,srcs[i],pos,BLOCK_SIZE,j,smp);
        }
        ++t;
    }
}

inline void perBuffer(AudioIOData& io){

    static unsigned int t = 0;
    int numFrames = io.framesPerBuffer();

    for(int j=0; j<numFrames; j++){

        double sec = (t / io.fps());

        for(int i = 0; i < NUM_SOURCES; i++){

            float smp = sin(sec*(220*(i+1))*2*M_PI)*(1.0/(i+1));
            float x = 1.5*cos(sec*(speedMult)*2.0*M_PI+ ((2.0*M_PI/NUM_SOURCES)*(i+1.0)));
            float y = 1.5*sin(sec*(speedMult)*2.0*M_PI + ((2.0*M_PI/NUM_SOURCES)*(i+1.0)));
            srcs[i].pos(x,y,0.0);
            srcs[i].writeSample(smp);

        }
        ++t;
    }
    scene.render(io);
}

void audioCB(AudioIOData& io){
    if(perSampProcessing){
        perSample(io);
    } else{
        perBuffer(io);
    }
}

//Converts spatial coordinate system to Allocore's OpenGL coordinate system
Vec3d convertCoords(Vec3d v){
    Vec3d returnVector;
    returnVector.x = v.x;
    returnVector.y = v.z;
    returnVector.z = -v.y;
    return returnVector;
}

class MyApp : public App
{
    Font font;
    Mesh text;

    Mesh speakerMesh;
    std::vector<Mesh> mVec;

    std::vector<Vec3d> sCoords;
    std::vector<int>  sChannels;

    Light light;

public:
    MyApp(std::vector<SpeakerTriple> sts): font("allocore/share/fonts/VeraMoIt.ttf", 20) {

        light.pos(0,6,0);

        //Setup the triangles
        for(int j = 0; j < sts.size();++j){

            SpeakerTriple s = sts[j];
            Mesh* mesh = new Mesh();
            mesh->primitive(Graphics::TRIANGLES);

            for(int i = 0; i < 3; ++i){
                Vec3d coords = convertCoords(s.vec[i]);
                mesh->vertex(coords.x,coords.y,coords.z);
            }

            Color c(rnd::uniform(1.f,0.1f), rnd::uniform(), rnd::uniform(),1.0);
            for(int i = 0;i < 3;++i){
                mesh->color(c);
            }

            mVec.push_back(*mesh);

        }

        addSurface(text,0,4);

        Speakers sp = speakerLayout.speakers();
        addSphere(speakerMesh);
        for(int i = 0; i <speakerLayout.speakers().size();i++ ){
            Speaker s = sp[i];
            sChannels.push_back(s.deviceChannel);
            sCoords.push_back(convertCoords(s.vec()));
        }

        for(int i = 0; i < mVec.size(); ++i){
            mVec[i].decompress();
            mVec[i].generateNormals();
        }

        nav().pos(0,0,0);
        initWindow();
    }


    void onDraw(Graphics& g){
        g.blendAdd();

        //Draw the triangles
        for(int i = 0; i < mVec.size(); ++i){
            g.draw(mVec[i]);
        }

        //Draw one of the sources
        g.pushMatrix();
        g.translate(convertCoords(srcpos));
        g.scale(0.1);
        g.draw(speakerMesh);
        g.popMatrix();

        //Draw the speakers
        for(int i = 0; i < sCoords.size(); ++i){

            g.pushMatrix();

            g.translate(sCoords[i]);
            g.scale(0.06);

            int chan = sChannels[i];
            font.write(text,std::to_string(chan));
            font.texture().bind();
            g.draw(text);
            font.texture().unbind();

            g.draw(speakerMesh);
            g.popMatrix();
        }
    }
};



int main (int argc, char * argv[]){

    if(layout == RING_LAYOUT){
        speakerLayout = SpeakerRingLayout<NUM_SPEAKS>(0.f,0.f,10.f);
    }


    //Creates three rings for testing different numbers o
    if(layout == TEST_LAYOUT){

                //Speakers per ring
                int r1 = 30;
                int r2 = 31;
                int r3 = 32;

                float el1 = 30.0f;
                float el2 = 0.0f;
                float el3 = -40.f;

                float rad = 20.0f;

                int spkInc = 0;

                for(int i = 0; i < r1;i++ ){
                    speakerLayout.addSpeaker(Speaker(spkInc,(360.f/r1)*i,el1,rad));
                    spkInc++;
                }

                for(int i = 0; i < r2;i++ ){
                    speakerLayout.addSpeaker(Speaker(spkInc,(360.f/r2)*i,el2,rad));
                    spkInc++;
                }
                for(int i = 0; i < r3;i++ ){
                    speakerLayout.addSpeaker(Speaker(spkInc,(360.f/r3)*i,el3,rad));
                    spkInc++;
                }
    }

    if(layout == ALLO_LAYOUT){
        speakerLayout= AlloSphereSpeakerLayout();
    }


    panner = new Vbap(speakerLayout);
    panner->setIs3D(layout==RING_LAYOUT ? false:true);
    listener = scene.createListener(panner);

    for(int i = 0; i < NUM_SOURCES; i++){
        srcs[i].usePerSampleProcessing(perSampProcessing);
        srcs[i].dopplerType(DOPPLER_NONE);
        scene.addSource(srcs[i]);
    }


    scene.usePerSampleProcessing(perSampProcessing);
    panner->setEnabled(true);
    panner->print();

    //Determine number of output channels
    Speakers allSpeakers = speakerLayout.speakers();
    int highestChannel = 0;
    for(Speaker s:allSpeakers){
        if(s.deviceChannel > highestChannel){
            highestChannel = s.deviceChannel;
        }
    }

    int outputChannels = highestChannel + 1;
    AudioIO audioIO(BLOCK_SIZE, 44100, audioCB, NULL, outputChannels, 0);
    audioIO.start();

    std::vector<SpeakerTriple> speakerTriangles = panner->triplets() ;

    MyApp(speakerTriangles).start();
    getchar();

    return 0;
}



