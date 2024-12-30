#include <cstdint>
#include "raylib.h"
#include "libs/raymath.h"

const float sampleRate = 44100.0f;

enum{
    SQUAREWAVE = 0,
    TRIANGLEWAVE = 1,
    SAWTOOTHWAVE = 2
};

struct Channel{
    bool loop = false;
    uint16_t volume = 0;
    uint16_t startAddress = 0;
    uint16_t length = 0;
    uint16_t progress = 0;
};

struct AcidAudioWave {
    // Cycles per second (hz)
    float frequency = 0.0f;

    int waveType = 0;

    // Audio frequency, for smoothing
    //float audioFrequency = 100.0f;

    // Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frequency
    //float oldFrequency = 1.0f;

    // Index for audio rendering
    float squareIndex = 0.0f;

    float triangleHeight = 0.0f;
    int triangleDirection = 1;

    float sawToothHeight = 0.0f;

    AudioStream stream;

};

AcidAudioWave VOICE0Wave;
AcidAudioWave VOICE1Wave;
AcidAudioWave VOICE2Wave;




void VOICE0InputCallback(void *buffer, unsigned int frames)
{

    float incr = VOICE0Wave.frequency/sampleRate;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        if(VOICE0Wave.waveType == SQUAREWAVE)
        {
            d[i] = ((short)VOICE0Wave.squareIndex % 2 == 0) ? 32767 : -32768;
            VOICE0Wave.squareIndex += incr;
        }
        else if(VOICE0Wave.waveType == TRIANGLEWAVE)
        {
            d[i] = (short)(VOICE0Wave.triangleHeight * 32767);
            VOICE0Wave.triangleHeight += incr * VOICE0Wave.triangleDirection;
            if(VOICE0Wave.triangleHeight < -1.0f){VOICE0Wave.triangleHeight = -1.0f;VOICE0Wave.triangleDirection = 1;}
            else if(VOICE0Wave.triangleHeight > 1.0f){VOICE0Wave.triangleHeight = 1.0f;VOICE0Wave.triangleDirection = -1;}
        }
        else if(VOICE0Wave.waveType == SAWTOOTHWAVE)
        {
            d[i] = (short)(VOICE0Wave.sawToothHeight * 32767);
            VOICE0Wave.sawToothHeight += incr;
            if(VOICE0Wave.sawToothHeight > 1.0f){VOICE0Wave.sawToothHeight = -1.0f;}
        }

    }
}

void VOICE1InputCallback(void *buffer, unsigned int frames)
{

    float incr = VOICE1Wave.frequency/sampleRate;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        if(VOICE1Wave.waveType == SQUAREWAVE)
        {
            d[i] = ((short)VOICE1Wave.squareIndex % 2 == 0) ? 32767 : -32768;
            VOICE1Wave.squareIndex += incr;
        }
        else if(VOICE1Wave.waveType == TRIANGLEWAVE)
        {
            d[i] = (short)(VOICE1Wave.triangleHeight * 32767);
            VOICE1Wave.triangleHeight += incr * VOICE1Wave.triangleDirection;
            if(VOICE1Wave.triangleHeight < -1.0f){VOICE1Wave.triangleHeight = -1.0f;VOICE1Wave.triangleDirection = 1;}
            else if(VOICE1Wave.triangleHeight > 1.0f){VOICE1Wave.triangleHeight = 1.0f;VOICE1Wave.triangleDirection = -1;}
        }
        else if(VOICE1Wave.waveType == SAWTOOTHWAVE)
        {
            d[i] = (short)(VOICE1Wave.sawToothHeight * 32767);
            VOICE1Wave.sawToothHeight += incr;
            if(VOICE1Wave.sawToothHeight > 1.0f){VOICE1Wave.sawToothHeight = -1.0f;}
        }
    }
}

void VOICE2InputCallback(void *buffer, unsigned int frames)
{
    //VOICE2Wave.audioFrequency = VOICE2Wave.frequency + (VOICE2Wave.audioFrequency - VOICE2Wave.frequency)*0.95f;

    float incr = VOICE2Wave.frequency/sampleRate;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        if(VOICE2Wave.waveType == SQUAREWAVE)
        {
            d[i] = ((short)VOICE2Wave.squareIndex % 2 == 0) ? 32767 : -32768;
            VOICE2Wave.squareIndex += incr;
        }
        else if(VOICE2Wave.waveType == TRIANGLEWAVE)
        {
            d[i] = (short)(VOICE2Wave.triangleHeight * 32767);
            VOICE2Wave.triangleHeight += incr * VOICE2Wave.triangleDirection;
            if(VOICE2Wave.triangleHeight < -1.0f){VOICE2Wave.triangleHeight = -1.0f;VOICE2Wave.triangleDirection = 1;}
            else if(VOICE2Wave.triangleHeight > 1.0f){VOICE2Wave.triangleHeight = 1.0f;VOICE2Wave.triangleDirection = -1;}
        }
        else if(VOICE2Wave.waveType == SAWTOOTHWAVE)
        {
            d[i] = (short)(VOICE2Wave.sawToothHeight * 32767);
            VOICE2Wave.sawToothHeight += incr;
            if(VOICE2Wave.sawToothHeight > 1.0f){VOICE2Wave.sawToothHeight = -1.0f;}
        }
    }
}

