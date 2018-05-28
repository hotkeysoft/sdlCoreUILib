#include "stdafx.h"
#include "WinResource.h"
#include <Windows.h>
#include "resource.h"

namespace WinUtil
{
	HMODULE g_hModDll;

	WinResource::DllResource WinResource::LoadResource(int id, const char * resourceType)
	{
		DllResource ret;

		HRSRC hRscr = ::FindResource(g_hModDll, MAKEINTRESOURCE(id), resourceType);
		if (hRscr) 
		{
			HGLOBAL hgRscr = ::LoadResource(g_hModDll, hRscr);
			if (hgRscr) 
			{
				ret.data = ::LockResource(hgRscr);
				ret.size = ::SizeofResource(g_hModDll, hRscr);
			}
		}

		return ret;
	}
}

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	WinUtil::g_hModDll = hinstDLL;
	// Perform actions based on the reason for calling.

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}