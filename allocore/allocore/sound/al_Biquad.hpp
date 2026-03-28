#ifndef INC_AL_BIQUAD_HPP
#define INC_AL_BIQUAD_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	A cross-over shelf filter that sums to an allpass

	Author(s):
	Ryan McGee, 2015
*/

namespace al{
    
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

///
/// \brief The BiQuad class
///
/// @ingroup allocore
class BiQuad
{
public:
    
    BiQuad(BIQUADTYPE _type = BIQUAD_LPF, double _sampleRate = 44100);
    ~BiQuad();
    
    void set(double freq, double bandwidth = 1.9, double dbGain = 0);
    void setSampleRate(double _rate){mSampleRate = _rate;}
    void processBuffer(float *buffer, int count);
    double operator()(double sample);
    
    void enable(bool on) {enabled = on;}
    
private:
    BIQUADTYPE mType;
    BiquadData mBD;
    double mSampleRate;
    bool enabled;
};

/// Cascaded Biquad to acheive steeper filters
/// Default params of numFilters = 8 and bandwidth = 0.26 work well for an anti-aliasing LPF filter @ 19k
///
/// @ingroup allocore
class BiQuadNX
{
public:
    
    BiQuadNX(int _numFilters = 8, BIQUADTYPE _type = BIQUAD_LPF, double _sampleRate = 44100);
    ~BiQuadNX();
    
    void set(double freq, double bandwidth = 0.26, double dbGain = 0);
    void setSampleRate(double _rate);
    void processBuffer(float *buffer, int count);
    double operator()(double sample);
    void enable(bool on);
    
private:
    int numFilters;
    BiQuad *mFilters;
};

} // al::
#endif
