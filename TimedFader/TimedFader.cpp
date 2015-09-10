#include "TimedFader.h"

//-----------------------------------------------------------------------------
HRESULT VDJ_API TimedFader::OnLoad()
{
	//inits here!
	fOutput = 1.0f;
	faderSteps = 15;
	currentFaderFrameCount = 1;

	fPlayDuration = 10.0f;
	fFadeDuration = 3.0f;
	fPlaySecondsRemaining = fPlayDuration;
	fFadeSecondsRemaining = fFadeDuration;
	fFadeDurationInFrames = fFadeDuration*(float)SampleRate;

	state = STATE_DONE;

	muteGain = (float)pow(10.0f, 0.05f * -60.0f);
	muteGain *= 0.01f;

	fSecondsPerFrame = 1.0f / (float)SampleRate;

	return S_OK;
}
//-----------------------------------------------------------------------------
HRESULT VDJ_API TimedFader::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->PluginName = "MyPlugin8";
	infos->Author = "Atomix Productions";
	infos->Description = "My first VirtualDJ 8 plugin";
	infos->Version = "1.0";
	infos->Flags = 0x00; // you could also use VDJFLAG_PROCESSAFTERSTOP if you want to process sound when the deck is stopped
	infos->Bitmap = NULL;

	return S_OK;
}
//---------------------------------------------------------------------------
ULONG VDJ_API TimedFader::Release()
{
	delete this;
	return 0;
}
//---------------------------------------------------------------------------
HRESULT VDJ_API TimedFader::OnStart()
{
	// ADD YOUR CODE HERE WHEN THE AUDIO PLUGIN IS STARTED
	state = STATE_IDLE;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT VDJ_API TimedFader::OnStop()
{
	// ADD YOUR CODE HERE WHEN THE AUDIO PLUGIN IS STOPPED
	fFadeSecondsRemaining = fFadeDuration;
	fPlaySecondsRemaining = fPlayDuration;
	state = STATE_IDLE;
	currentFaderFrameCount = 1;
	faderSteps = 15;

	return S_OK;
}
//---------------------------------------------------------------------------
HRESULT VDJ_API TimedFader::OnProcessSamples(float *buffer, int nb)
{
	// ADD YOUR AUDIO TREATMENT CODE HERE USING THE AUDIO BUFFER *buffer OF 2*nb FLOAT SAMPLES (STEREO SIGNAL)
	float out[2], in[2];

	float max1 = 0;
	float max2 = 0;

	bool audioIsPlaying = true;

	// Check the current buffer for any audio, if none, audio is not playing
	for (int i = 0; i<nb; i++)
	{
		in[0] = buffer[2 * i];
		in[1] = buffer[2 * i + 1];

		if (fabs(in[0]) > max1)
		{
			max1 = fabs(in[0]);
		}
		if (fabs(in[1]) > max2)
		{
			max2 = fabs(in[1]);
		}
	}
	if (max1 < 0.000001 && max2 < 0.000001)
	{
		audioIsPlaying = false;
	}

	// In any state, if no audio is playing, go to Idle
	if (!audioIsPlaying)
	{
		state = STATE_IDLE;
	}

	int index = 0;
	switch (state)
	{
	case STATE_PLAYING:
		// Do nothing else, just let audio play
		while (fPlaySecondsRemaining > 0.0 && index < nb)
		{
			/*in[0] = buffer[2 * index];
			in[1] = buffer[2 * index + 1];
			*/	
			fPlaySecondsRemaining -= fSecondsPerFrame;
			// let audio pass through
			/*
			buffer[2 * index] = in[0] * 1.0;
			buffer[2 * index + 1] = in[1] * 1.0;
			*/
			index++;
		}

		if (fPlaySecondsRemaining <= 0.0)
		{
			state = STATE_FADING;
		}
		//faderDisplay->setParameter(PARAM_PlaySecondsRemaining, fPlaySecondsRemaining);
		break;
	case STATE_FADING:
		while (fFadeSecondsRemaining > 0.0 && index < nb)
		{
			currentFaderFrameCount++;
			fFadeSecondsRemaining -= fSecondsPerFrame;
			// Fade based on current fade tap
			currentFaderMultiplier = GetNextFaderStep();			
			in[0] = buffer[2 * index];
			in[1] = buffer[2 * index + 1];
			out[0] = in[0] * currentFaderMultiplier;
			out[1] = in[1] * currentFaderMultiplier;

			buffer[2 * index] = out[0];
			buffer[2 * index + 1] = out[1];
			index++;
		}

		if (fFadeSecondsRemaining <= 0.0)
		{
			// silence out the rest of the sample
			for (int i = 0; i < nb; i++)
			{
				buffer[2 * i] *= muteGain;
				buffer[2 * i + 1] *= muteGain;
			}
			//faderDisplay->setParameter(PARAM_FaderLight, 1);
			state = STATE_DONE;
		}
		//faderDisplay->setParameter(PARAM_FadeSecondsRemaining, fFadeSecondsRemaining);
		break;
	case STATE_DONE:
		fPlaySecondsRemaining = fPlayDuration;
		fFadeSecondsRemaining = fFadeDuration;
		// make sure the audio stays muted
		// and wait here until all audio stops playing, then move back to Idle
		for (int i = 0; i < nb; i++)
		{
			buffer[2 * i] *= muteGain;
			buffer[2 * i + 1] *= muteGain;
		}
		break;
	case STATE_IDLE:
		// Set these here again in case someone changed the durations while we are in the IDLE state
		currentFaderFrameCount = 1;
		fPlaySecondsRemaining = fPlayDuration;
		fFadeSecondsRemaining = fFadeDuration;
		//faderDisplay->setParameter(PARAM_PlaySecondsRemaining, fPlayDuration);
		//faderDisplay->setParameter(PARAM_FadeSecondsRemaining, fFadeDuration);
		//faderDisplay->setParameter(PARAM_FaderLight, 0);

		// Instead, messy hack:
		//if (fabs(buffer[0]) > 0.0000001 && fabs(buffer[1]) > 0.0000001)
		if (audioIsPlaying)
		{
			// Audio is coming through, start the timer...
			state = STATE_PLAYING;
		}
		break;
	}

	return S_OK;
}
// Gives a nice logarithmic curve [0,1.0] based on the total number of frames in the fade duration and the current frame we're on
float TimedFader::GetNextFaderStep()
{
	// Logarithmic work
	// put x in the range [0,1.0]
	float x = currentFaderFrameCount / fFadeDurationInFrames;
	float step = 0.0f;

	/* inverse log (1/log(x))
	// put x in the range [0,0.9999]  (don't go to 1 to avoid a /0 error)
	x = (x <= 0) ? 0.0001f : (x >= 1) ? 0.9f : x;
	float maxLog = fabs(1/log(0.9f));
	step = ((1 / log(x)) + maxLog)/maxLog;
	*/

	/* 2^x
	// range of [0,7.2]
	x = 7.2f*x;
	step = 1 - pow(2, x);
	// step is now in the range of [0,-150], convert this to [0,1] and invert [1,0]
	step = step / -150;
	step = 1 - step; */

	/* Linear */
	step = 1 - x;

	// Just some sanity checks
	// never amplify
	if (step > 1.0)
	{
		step = 1.0;
	}
	// never go below -60dB
	if (step < muteGain)
	{
		step = muteGain;
	}

	return step;
}
