//////////////////////////////////////////////////////////////////////////
//
// VirtualDJ v4.x and more / Numark Cue / VirtualVinyl
// Flanger
// (c)Atomix Productions 2007
//
//////////////////////////////////////////////////////////////////////////
//
// This file is just an example of flanger. This code was modified in order
// to help the understanding and to show more features of the SDK.
//  
//////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "VdjDsp.h"

#define _USE_MATH_DEFINES // for M_PI
#include <math.h>   // for the function cos() and M_PI

#include <stdio.h> // for the function sprintf()

//---------------------------------------------------------------------------
#define PLUGIN_DECK 0
#define DELAYMIN	0.1f	// 0.1 ms delay min
#define DELAYMAX	12.1f	// 12.1 ms delay max
#define LFO(x) (1.0f-cos(float(M_PI)/4.0f*x))/2.0f   // frequency f=125Hz 
                                                     //  (if x is in ms)

// ID Parameters
#define ID_INIT         -1
#define ID_BUTTON_RESET  0
#define ID_SLIDER_VOLUME 1
#define ID_SLIDER_DELAY  2
#define ID_STRING_TEXT   3

//////////////////////////////////////////////////////////////////////////
// Class definition
//////////////////////////////////////////////////////////////////////////
class CFlanger : public IVdjPluginDsp
{
public:
	// VdjPlugin
	HRESULT __stdcall OnLoad();
	HRESULT __stdcall OnGetPluginInfo(TVdjPluginInfo *infos);
	HRESULT __stdcall OnParameter(int id);
	ULONG   __stdcall Release();
	// VdjDsp
	HRESULT __stdcall OnStart(int pos,int deck);
	HRESULT __stdcall OnProcessSamples(short *buffer,int nb,int pos);
	HRESULT __stdcall OnStop();
private:
	int SliderDelay,SliderVolume;
	float Delay,Volume;
	int StartPos;
	DWORD Buff[1024]; // We need our own audio buffer of 1024 samples for the flanger
	int ButtonDown;
	char st[128];

	void UpdateDisplay();
};
//////////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////////
HRESULT __stdcall DllGetClassObject(const GUID &rclsid,const GUID &riid,void** ppObject)
{
	// This is the standard DLL loader for COM object.
	// You don't need to change anything in this function.
	if(memcmp(&rclsid,&CLSID_VdjPlugin,sizeof(GUID))!=0) return CLASS_E_CLASSNOTAVAILABLE;
	if(memcmp(&riid,&IID_IVdjPluginDsp,sizeof(GUID))!=0) return CLASS_E_CLASSNOTAVAILABLE;
	*ppObject=new CFlanger();
	return NO_ERROR;
}
//-----------------------------------------------------------------------------
HRESULT __stdcall CFlanger::OnLoad()
{	
	StartPos=-1;
	DeclareParameter(&ButtonDown,VDJPARAM_BUTTON,ID_BUTTON_RESET,"Reset",0);
	DeclareParameter(&SliderVolume,VDJPARAM_SLIDER,ID_SLIDER_VOLUME,"Volume",2731);
	DeclareParameter(&SliderDelay,VDJPARAM_SLIDER,ID_SLIDER_DELAY,"Delay",2048);
	DeclareParameter(st,VDJPARAM_STRING,ID_STRING_TEXT,"Status",sizeof(st));
	OnParameter(ID_INIT);
	return S_OK;
}
//-----------------------------------------------------------------------------
HRESULT __stdcall CFlanger::OnGetPluginInfo(TVdjPluginInfo *infos)
{
	infos->Author="Atomix Productions";
	infos->PluginName="Flanger";
	infos->Description="Add a flanger effect synced on the beat";
	infos->Bitmap=LoadBitmap(hInstance,MAKEINTRESOURCE(100)); // bitmap.bmp (64x64)
	infos->Flag=0;  // default value (see VdjDsp.h for other Flags)
	return S_OK;
}
//------------------------------------------------------------------------------
ULONG __stdcall CFlanger::Release()
{
	delete this;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// User Interface
//////////////////////////////////////////////////////////////////////////
HRESULT __stdcall CFlanger::OnParameter(int id)
{
	if(id==ID_BUTTON_RESET && ButtonDown==1)
	{
		SliderVolume=2731;
		SliderDelay=2048;
		id=ID_INIT;
	}
	
	if(id==ID_SLIDER_VOLUME || id==ID_INIT)  Volume=SliderVolume/float(4096);
	
	if(id==ID_SLIDER_DELAY || id==ID_INIT)  Delay=SliderDelay/float(4096);
	
	UpdateDisplay();
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
// Sound processing
//////////////////////////////////////////////////////////////////////////
#define SAMPLING_RATE 44100    // VirtualDJ reads 44100 samples per second
#define LEFTCHAN(v) ((short)(v&0xFFFF))
#define RIGHTCHAN(v) ((short)(v>>16))
#define MAKECHAN(l,r) ((r<<16)+(l&0xFFFF))
#define LIMITER(x) {if(x<-32768) x=-32768; else if(x>32767) x=32767;}
//-------------------------------------------------------------------------------
HRESULT __stdcall CFlanger::OnStart(int pos,int deck)
{
	StartPos=pos;
	memset(Buff,0,sizeof(Buff)); // We initialize our own audio buffer
	return S_OK;
}
//-------------------------------------------------------------------------------
HRESULT __stdcall CFlanger::OnStop()
{
	return S_OK;
}
//-------------------------------------------------------------------------------
HRESULT __stdcall CFlanger::OnProcessSamples(short *buffer,int nb,int pos)
{
	// VirtualDJ returns an audio buffer composed of 'nb' samples
	// taken from the 'pos'th sample of the song
	DWORD *stereoptr=(DWORD*)buffer;
	
	// We get the number of samples between 2 following beats
	int Bpm=SongBpm?SongBpm:22050;
	
	float mBeat,Delta;
	int DeltaPos;
	DWORD v1,v2,v;
	int l,r;

	int i;
	// We read the 'nb' samples of the VirtualDJ audio buffer
	// (from 0 to nb-1)
	for(i=0;i<nb;i++)
	{
		mBeat=(pos+i-StartPos)/float(Bpm);

		// Delta defined in ms
		Delta= DELAYMIN + (DELAYMAX-DELAYMIN) * Delay * LFO(mBeat) ;  
		
		// Delta converted in samples
		DeltaPos=(int)(Delta/1000*SAMPLING_RATE); 
		
		// We call the 'i'th sample 
		v1=stereoptr[i];
		
		// We call in our own audio buffer the sample 'DeltaPos' before v1
		v2=Buff[(pos+i-StartPos-DeltaPos)&1023];
		
		// "Song - Effect" on the left channel 
		l=(int)(LEFTCHAN(v1)-Volume*LEFTCHAN(v2));
		LIMITER(l);
		
		// "Song - Effect" on the right channel 
		r=(int)(RIGHTCHAN(v1)-Volume*RIGHTCHAN(v2));
		LIMITER(r);
		
		// We recreate the stereo with the left and the right channel
		v=MAKECHAN(l,r);
		
		// We send the sample to the VirtualDJ audio buffer
		stereoptr[i]=v;

		// We save the sample in our own audio buffer
		Buff[(pos+i-StartPos)&1023]=v;
	}
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
// Other functions used in this plugin
//////////////////////////////////////////////////////////////////////////
void CFlanger::UpdateDisplay()
{
	int Vol=int(Volume*100);
	float msDelay=DELAYMIN+Delay*(DELAYMAX-DELAYMIN);
	
	// Convert values in a string. New compilators may ask sprintf_s()
	sprintf(st,"Delay: %.2f ms | Volume: %d%%",msDelay,Vol);   
	
	// Update the inferface
	SendCommand("effect_redraw",PLUGIN_DECK);  
}