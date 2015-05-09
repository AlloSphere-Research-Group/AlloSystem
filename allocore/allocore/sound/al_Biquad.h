//  Created by Ryan McGee on 5/8/15.
//
//

#ifndef __AL_BIQUAD__
#define __AL_BIQUAD__

namespace al
{
    
/* this holds the data required to update samples thru a filter */
typedef struct {
    float a0, a1, a2, a3, a4;
    float x1, x2, y1, y2;
}
BiquadData;

/* filter types */
enum BIQUADTYPE {
    BIQUAD_LPF, /* low pass filter */
    BIQUAD_HPF, /* High pass filter */
    BIQUAD_BPF, /* band pass filter */
    BIQUAD_NOTCH, /* Notch Filter */
    BIQUAD_PEQ, /* Peaking band EQ filter */
    BIQUAD_LSH, /* Low shelf filter */
    BIQUAD_HSH /* High shelf filter */
};

class BiQuad
{
public:
    
    BiQuad(BIQUADTYPE _type = BIQUAD_LPF, float _sampleRate = 44100);
    ~BiQuad();
    
    void set(float freq, float bandwidth = 1.9, float dbGain = 0);
    void setSampleRate(float _rate){mSampleRate = _rate;}
    void processBuffer(float *buffer, int count);
    
private:
    BIQUADTYPE mType;
    BiquadData mBD;
    float mSampleRate;
    float filter(float sample);
};

// Cascaded Biquad to acheive steeper filters
class BiQuadNX
{
public:
    
    BiQuadNX(int _numFilters = 4, BIQUADTYPE _type = BIQUAD_LPF, float _sampleRate = 44100);
    ~BiQuadNX();
    
    void set(float freq, float bandwidth = 1.9, float dbGain = 0);
    void setSampleRate(float _rate);
    void processBuffer(float *buffer, int count);
    
private:
    int numFilters;
    BiQuad *mFilters;
    float filter(float sample);
};

}

#endif /* defined(__AL_BIQUAD__) */
