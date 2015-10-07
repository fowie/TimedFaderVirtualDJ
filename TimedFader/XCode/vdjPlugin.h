//////////////////////////////////////////////////////////////////////////
//
// VirtualDJ / Cue / VirtualVinyl / PCDJ VJ
// Plugin SDK for Windows / MAC OS
// (c)Atomix Productions 2006-2008
// File Version: 1.3
//
//////////////////////////////////////////////////////////////////////////
//
// This file defines the basic functions that are used in all plugins.
// It defines the functions and variables needed to:
// - load and unload a plugin
// - give the infos about the plugin (name, picture, etc)
// - get the parameters automatically saved and restored between loads
// - interact with VirtualDJ (ask queries or send commands)
// - implement a custom interface
//
// Other functions specific to particular types of plugin can be found
// in their respective header file
//
//////////////////////////////////////////////////////////////////////////


#ifndef VdjPlugin_H
#define VdjPlugin_H

//////////////////////////////////////////////////////////////////////////
// Platform specific defines for compatibility Mac/Windows

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32_))
	#ifndef WIN32_LEAN_AND_MEAN        /* For old plugins compatibility */
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#endif
	#define VDJ_EXPORT		__declspec( dllexport )
    #define VDJ_API         __stdcall
	#define VDJ_BITMAP		HBITMAP
	#define VDJ_HINSTANCE	HINSTANCE
    #define VDJ_WINDOW      HWND
    #define PLUGIN_BITMAP   LoadBitmap(hInstance,MAKEINTRESOURCE(100)) /* For TVdjPluginInfo->Bitmap */
#elif (defined(__APPLE__) || defined(MACOSX) || defined(__MACOSX__))
	#define VDJ_EXPORT		__attribute__ ((visibility ("default")))
    #define VDJ_API
	#define VDJ_BITMAP		char *
	#define VDJ_HINSTANCE	void *
    #define VDJ_WINDOW      WindowRef
    #define PLUGIN_BITMAP   "bitmap.bmp"  /* For TVdjPluginInfo->Bitmap */
	#define S_OK               ((HRESULT)0x00000000L)
	#define S_FALSE            ((HRESULT)0x00000001L)
	#define E_NOTIMPL          ((HRESULT)0x80004001L)
	#define MAKEINTRESOURCE(i) (LPSTR)((ULONG_PTR)((WORD)(i)))
	#define CLASS_E_CLASSNOTAVAILABLE -1
	#define NO_ERROR 0
	#define wsprintf sprintf
    #define __stdcall          /* For old plugins compatibility */
	typedef long HRESULT;
	typedef struct _GUID {
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[ 8 ];
	} GUID;
	typedef unsigned long ULONG;
	typedef unsigned long DWORD;
#endif


//////////////////////////////////////////////////////////////////////////
// Standard structures and defines

struct TVdjPluginInfo
{
	char *PluginName;
	char *Author;
	char *Description;
	VDJ_BITMAP Bitmap;
	DWORD Flag;
};

// Plugin Interface
#define VDJPARAM_BUTTON	0
#define VDJPARAM_SLIDER	1
#define VDJPARAM_SWITCH	2
#define VDJPARAM_STRING	3
#define VDJPARAM_CUSTOM	4
#define VDJPARAM_RADIO	5

// Deck ID
#define PLUGIN_DECK  0
#define   LEFT_DECK  1
#define  RIGHT_DECK  2

//////////////////////////////////////////////////////////////////////////
// Base class

class IVdjPlugin
{
public:
	// Initialization
	virtual HRESULT VDJ_API OnLoad() {return S_OK;}
	virtual HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo *infos) {return E_NOTIMPL;}
	virtual ULONG VDJ_API Release() {delete this;return 0;}
	virtual ~IVdjPlugin() {}

	// callback functions to communicate with VirtualDJ
	HRESULT (VDJ_API *SendCommand)(char *command,int deck); // send a command to VirtualDJ
	HRESULT (VDJ_API *GetInfo)(char *query,void *result); // get infos from VirtualDJ

	// parameters stuff
	// call DeclareParameter() for all your variables during OnLoad()
	// if type=VDJPARAM_CUSTOM or VDJPARAM_STRING, defaultvalue must be set to sizeof(*parameter)
	HRESULT (VDJ_API *DeclareParameter)(void *parameter,int type,int id,char *name,int defaultvalue);
	// OnParameter will be called each time a parameter is changed from within VirtualDJ
	virtual HRESULT VDJ_API OnParameter(int id) {return S_OK;}

	// Custom user-interface
	// Create a Window using CreateWindow() or CreateDialog(), and send back the HWND.
	// If you return E_NOTIMPL, the default interface will be used.
	virtual HRESULT VDJ_API OnGetUserInterface(VDJ_WINDOW *hWnd) {return E_NOTIMPL;}
	VDJ_HINSTANCE hInstance;
	int Width,Height;
};

//////////////////////////////////////////////////////////////////////////
// GUID definitions

#ifndef VDJCLASSGUID_DEFINED
#define VDJCLASSGUID_DEFINED
	static const GUID CLSID_VdjPlugin = { 0x2e1480fe, 0x4ff4, 0x4539, { 0x90, 0xb3, 0x64, 0x5f, 0x5d, 0x86, 0xf9, 0x3b } };
#else
	extern static const GUID CLSID_VdjPlugin;
#endif

//////////////////////////////////////////////////////////////////////////
// DLL export function

#ifndef NODLLEXPORT
#ifdef __cplusplus
extern "C" {
#endif
	VDJ_EXPORT HRESULT VDJ_API DllGetClassObject(const GUID &rclsid,const GUID &riid,void** ppObject);
#ifdef __cplusplus
}
#endif
#endif

//////////////////////////////////////////////////////////////////////////

#endif /* VdjPlugin_H */
