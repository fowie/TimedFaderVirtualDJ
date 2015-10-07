//////////////////////////////////////////////////////////////////////////
//
// VirtualDJ / Cue / VirtualVinyl / PCDJ VJ
// Plugin SDK for Windows / MAC OS
// (c)Atomix Productions 2006-2008
// File Version: 1.3
//
//////////////////////////////////////////////////////////////////////////
//
// This file defines the sound effect plugins.
// In addition to all the elements supported from the base IVdjPlugin class,
// it defines additional DSP-specific functions and variables:
//
//////////////////////////////////////////////////////////////////////////


#ifndef VdjDsp_H
#define VdjDsp_H

#include "VdjPlugin.h"

//////////////////////////////////////////////////////////////////////////
// DSP plugin class

class IVdjPluginDsp : public IVdjPlugin
{
public:
	// called when the plugin is started or stopped
	virtual HRESULT VDJ_API OnStart(int pos,int deck) {return S_OK;}
	virtual HRESULT VDJ_API OnStop() {return S_OK;}

	// This function will be called each time VirtualDJ needs your plugin
	// to be applied on a new sound buffer
	// NOTE: samples are stereo so composed of 2 mono samples, 
	// consequently you need to process up to buffer[2*nb]
	virtual HRESULT VDJ_API OnProcessSamples(short *buffer,int nb,int pos)=0;
	// This function let you take specific action once a particular point has been reached
	virtual HRESULT VDJ_API OnPosition(int actualpos) {return S_OK;}

	// If your plugin needs to access an other sound buffer than the one at the
	// position provided, you can get it with this function
	HRESULT (VDJ_API *GetSongSamples)(int pos,int nb,short **buffer);
	// Call this function if you need to stop your plugin automatically
	HRESULT (VDJ_API *Stop)();
	// Call this function if you want to change the current reading position displayed
	HRESULT (VDJ_API *ChangePosition)(int newpos);

	// Some useful variables
	int SongBpm; // number of samples between two consecutive beats
	int SongPhase; // number of samples between the start of the song and the first beat
};

//////////////////////////////////////////////////////////////////////////
// Macros for sound processing
#define SAMPLING_RATE 44100.0f

// (a mono sample = 16 bits [= 65536 values] => short) 
// (a stereo sample = 16 bits + 16 bits = 32 bits => DWORD)
// stereo:          1                 2                 3                       nb
//         |-------- --------|-------- --------|-------- --------| ... |-------- --------|
//               (32bits)          (32bits)         (32bits)                (32bits)
//
// mono:       1L       1R       2L       2R       3L       3R            nbL      nbR
//         |--------|--------|--------|--------|--------|--------| ... |--------|--------|
//          (16bits) (16bits) (16bits) (16bits) (16bits) (16bits)       (16bits) (16bits)
//
// buffer[i]:  0         1        2        3        4       5             2nb-2    2nb-1
//
#define LEFTCHAN(v) ((short)(v&0xFFFF))
#define RIGHTCHAN(v) ((short)(v>>16))
#define MAKECHAN(l,r) ((DWORD)((r<<16)+(l&0xFFFF)))
#define LIMITER(s) {if(s<-32768) s=-32768; else if(s>32767) s=32767;} // max{short}=32767 & min{short}=-32768

//////////////////////////////////////////////////////////////////////////
// flags used in OnGetPluginInfo()
#define VDJPLUGINFLAG_INPLACE			0x00	// normal behavior
#define VDJPLUGINFLAG_NOTINPLACE		0x01	// set if you don't need buffer to contain song's data on input (if you call GetSongSamples)

#define VDJPLUGINFLAG_CACHE				0x00	// if set, OnProcessSamples will be called as linearly as possible (default behavior)
#define VDJPLUGINFLAG_NOCACHE			0x02	// if set, OnProcessSamples might be called in every direction, and with nb as low as 1
#define VDJPLUGINFLAG_FIXED512			0x04	// if set, OnPorcessSamples will received only 512 samples buffers

#define VDJPLUGINFLAG_PROCESSSONG		0x00	// apply the plugin on the song (default behavior)
#define VDJPLUGINFLAG_PROCESSSCRATCH	0x08	// apply the plugin on the desk output (useful for scratch plugins)
#define VDJPLUGINFLAG_PROCESSMASTER		0x10	// apply the plugin on the master output


//////////////////////////////////////////////////////////////////////////
// GUID definitions

#ifndef VDJDSPGUID_DEFINED
#define VDJDSPGUID_DEFINED
	static const GUID IID_IVdjPluginDsp = { 0x41dbff5, 0x55d4, 0x47ee, { 0x9d, 0x32, 0xd3, 0xc8, 0xa2, 0x0, 0x61, 0xff } };
#else
	extern static const GUID IID_IVdjPluginDsp;
#endif

//////////////////////////////////////////////////////////////////////////

#endif /* VdjDsp_H */
