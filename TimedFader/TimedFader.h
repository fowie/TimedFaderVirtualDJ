#ifndef TIMEDFADER_H
#define TIMEDFADER_H
#include "vdjDsp8.h"

#include <cmath>
// round() is only defined in C++11 so we use this workaround
#define round(v) ((int)floor((v)+0.5f))

class CMyPlugin8 : public IVdjPluginDsp8
{
public:
	HRESULT VDJ_API OnLoad();
	HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
	ULONG VDJ_API Release();
	HRESULT VDJ_API OnStart();
	HRESULT VDJ_API OnStop();
	HRESULT VDJ_API OnProcessSamples(float *buffer, int nb);

private:
	int Bpm;
	int StartPos;

	/* if you want to work on short samples (16 bit) instead of float samples (32 bit):
	short bufferShort[8194];
	void ConvertFloat2Short(short *BufferOut, float *BufferIn, int BufferSize);
	void ConvertShort2Float(float *BufferOut, short *BufferIn, int BufferSize);
	*/
};

#endif