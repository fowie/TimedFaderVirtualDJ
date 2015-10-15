#ifndef TIMEDFADER_H
#define TIMEDFADER_H
#include "vdjDsp8.h"

#include <cmath>
#include <stdio.h>
#include <float.h>

// round() is only defined in C++11 so we use this workaround
#define round(v) ((int)floor((v)+0.5f))

#define STATE_IDLE 0
#define STATE_PLAYING 1
#define STATE_FADING 2
#define STATE_DONE 3

#define MAX_PLAY_MIN_DURATION 4  // 3 minutes
#define MAX_PLAY_SEC_DURATION 59
#define MAX_FADE_DURATION 15   // 30 seconds

// ID Parameters
#define ID_INIT         -1  //useful so I can call UpdateDisplay the first time without actually updating a particular GUI element
#define ID_SLIDER_PLAY_MINUTES 1
#define ID_SLIDER_PLAY_SECONDS 2
#define ID_SLIDER_FADE  3
#define ID_STRING_PLAY   4
#define ID_STRING_FADE   5


class TimedFader : public IVdjPluginDsp8
{
public:
	HRESULT VDJ_API OnLoad();
	HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
	ULONG VDJ_API Release();
	HRESULT VDJ_API OnStart();
	HRESULT VDJ_API OnStop();
	HRESULT VDJ_API OnParameter(int id);
	HRESULT VDJ_API OnProcessSamples(float *buffer, int nb);

private:
	void UpdateDisplay();
	float GetNextFaderStep();

	float fPlayMinDurationParameter;
	float fPlaySecDurationParameter;
	float fFadeDurationParameter;
	float fPlayDuration;
	float fFadeDuration;
	float fPlaySecondsRemaining;
	float fFadeSecondsRemaining;
	float muteGain;

	int currentFaderFrameCount; // current step (out of the total # of frames)
	float currentFaderMultiplier; // amount to fade in each step
	int faderSteps; // number of steps
	float fOutput;

	int state; // 0 = none, 1 = playing, 2 = fading

	float fSecondsPerFrame;
	float fFadeDurationInFrames;
	char fadeDurationString[128];
	char playDurationString[128];
};

#endif