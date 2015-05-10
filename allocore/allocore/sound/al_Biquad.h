//  Created by Ryan McGee on 5/8/15.
//
//

#ifndef __AL_BIQUAD__
#define __AL_BIQUAD__

namespace al
{
    
/* this holds the data required to update samples thru a filter */
typedef struct {
    double a0, a1, a2, a3, a4;
    double x1, x2, y1, y2;
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
    
    BiQuad(BIQUADTYPE _type = BIQUAD_LPF, double _sampleRate = 44100);
    ~BiQuad();
    
    void set(double freq, double bandwidth = 1.9, double dbGain = 0);
    void setSampleRate(double _rate){mSampleRate = _rate;}
    void processBuffer(float *buffer, int count);
    double operator()(double sample);
    
private:
    BIQUADTYPE mType;
    BiquadData mBD;
    double mSampleRate;
};

// Cascaded Biquad to acheive steeper filters
class BiQuadNX
{
public:
    
    BiQuadNX(int _numFilters = 8, BIQUADTYPE _type = BIQUAD_LPF, double _sampleRate = 44100);
    ~BiQuadNX();
    
    void set(double freq, double bandwidth = 1.2, double dbGain = 0);
    void setSampleRate(double _rate);
    void processBuffer(float *buffer, int count);
    double operator()(double sample);
    
private:
    int numFilters;
    BiQuad *mFilters;
};

}

#endif /* defined(__AL_BIQUAD__) */
