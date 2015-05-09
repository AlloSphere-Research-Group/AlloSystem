//
//  UAFilters.cpp
//  KineticV2
//
//  Created by Ryan McGee on 5/8/15.
//
//
#include "al_Biquad.h"
#include <stdlib.h>
#include <cmath>

using namespace al;

BiQuad::BiQuad(BIQUADTYPE _type, float _sampleRate)
:
mType(_type),
mSampleRate(_sampleRate)
{
    mBD.x1 = mBD.x2 = 0;
    mBD.y1 = mBD.y2 = 0;
    
    set(mSampleRate/2-1, 1.9, 0);
}

BiQuad::~BiQuad()
{
    
}

void BiQuad::set(float freq, float bandwidth, float dbGain)
{
    //TODO all the way to fs/2, range
    if(freq > 10000) freq = 10000;
    if(freq <= 100) freq = 100;
    
    float A, omega, sn, cs, alpha, beta;
    float a0, a1, a2, b0, b1, b2;
    
    /* setup variables */
    A = pow(10, dbGain /40);
    omega = 2 * M_PI * freq /mSampleRate;
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn * sinh(M_LN2 /2 * bandwidth * omega /sn);
    beta = sqrt(A + A);
    
    switch (mType) {
        case BIQUAD_LPF:
            b0 = (1 - cs) /2;
            b1 = 1 - cs;
            b2 = (1 - cs) /2;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        case BIQUAD_HPF:
            b0 = (1 + cs) /2;
            b1 = -(1 + cs);
            b2 = (1 + cs) /2;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        case BIQUAD_BPF:
            b0 = alpha;
            b1 = 0;
            b2 = -alpha;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        case BIQUAD_NOTCH:
            b0 = 1;
            b1 = -2 * cs;
            b2 = 1;
            a0 = 1 + alpha;
            a1 = -2 * cs;
            a2 = 1 - alpha;
            break;
        case BIQUAD_PEQ:
            b0 = 1 + (alpha * A);
            b1 = -2 * cs;
            b2 = 1 - (alpha * A);
            a0 = 1 + (alpha /A);
            a1 = -2 * cs;
            a2 = 1 - (alpha /A);
            break;
        case BIQUAD_LSH:
            b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
            b1 = 2 * A * ((A - 1) - (A + 1) * cs);
            b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
            a0 = (A + 1) + (A - 1) * cs + beta * sn;
            a1 = -2 * ((A - 1) + (A + 1) * cs);
            a2 = (A + 1) + (A - 1) * cs - beta * sn;
            break;
        case BIQUAD_HSH:
            b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
            b1 = -2 * A * ((A - 1) + (A + 1) * cs);
            b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
            a0 = (A + 1) - (A - 1) * cs + beta * sn;
            a1 = 2 * ((A - 1) - (A + 1) * cs);
            a2 = (A + 1) - (A - 1) * cs - beta * sn;
            break;
        default:
            return;
    }
    
    /* precompute the coefficients */
    mBD.a0 = b0 /a0;
    mBD.a1 = b1 /a0;
    mBD.a2 = b2 /a0;
    mBD.a3 = a1 /a0;
    mBD.a4 = a2 /a0;
}

void BiQuad::processBuffer(float *buffer, int count)
{
    for (int i=0; i<count; i++)
    {
        float sample = buffer[i];
        sample = (*this)(sample);
        buffer[i] = sample;
    }
}

float BiQuad::operator()(float sample)
{
    float result;
    
    /* compute result */
    result = mBD.a0 * sample + mBD.a1 * mBD.x1 + mBD.a2 * mBD.x2 -
    mBD.a3 * mBD.y1 - mBD.a4 * mBD.y2;
    
    /* shift x1 to x2, sample to x1 */
    mBD.x2 = mBD.x1;
    mBD.x1 = sample;
    
    /* shift y1 to y2, result to y1 */
    mBD.y2 = mBD.y1;
    mBD.y1 = result;
    
    return result;
}

////////////////////////////////////////////////////////////////////////////

BiQuadNX::BiQuadNX(int _numFilters, BIQUADTYPE _type, float _sampleRate)
:
numFilters(_numFilters)
{
    mFilters = (BiQuad*)malloc(sizeof(BiQuad)*numFilters);
    for(int i = 0; i < numFilters; i++)
        mFilters[i] = BiQuad(_type, _sampleRate);
}
BiQuadNX::~BiQuadNX()
{
    free(mFilters);
}

void BiQuadNX::set(float freq, float bandwidth, float dbGain)
{
    for(int i = 0; i < numFilters; i++)
        mFilters[i].set(freq, bandwidth, dbGain);
}
void BiQuadNX::setSampleRate(float _rate)
{
    for(int i = 0; i < numFilters; i++)
        mFilters[i].setSampleRate(_rate);
}
void BiQuadNX::processBuffer(float *buffer, int count)
{
    for(int i = 0; i < numFilters; i++)
        mFilters[i].processBuffer(buffer, count);
}

float BiQuadNX::operator()(float sample)
{
    float result = sample;
    //for(int i = numFilters-1; i >= 0; i--)
    for(int i = 0; i < numFilters; i++)
    {
        result = mFilters[i](result);
    }
    return result;
}
