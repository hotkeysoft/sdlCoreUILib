#include "stdafx.h"
#include "PlatformResource.h"
#include "resource.h"

extern unsigned char _binary_Resources_Oxygen_Regular_ttf_start;
extern unsigned char _binary_Resources_Oxygen_Regular_ttf_end;

extern unsigned char _binary_Resources_Oxygen_Bold_ttf_start;
extern unsigned char _binary_Resources_Oxygen_Bold_ttf_end;

extern unsigned char _binary_Resources_FiraMono_Regular_ttf_start;
extern unsigned char _binary_Resources_FiraMono_Regular_ttf_end;

extern unsigned char _binary_Resources_widget8x12_png_start;
extern unsigned char _binary_Resources_widget8x12_png_end;

extern unsigned char _binary_Resources_widget15x15_png_start;
extern unsigned char _binary_Resources_widget15x15_png_end;

extern unsigned char _binary_Resources_widget24x24_png_start;
extern unsigned char _binary_Resources_widget24x24_png_end;

namespace PlatformUtil
{
	PlatformResource::LibResource PlatformResource::LoadResource(int id, const char * resourceType)
	{
		LibResource ret;
		unsigned char* start = nullptr;
		unsigned char* end = nullptr;
	
		switch(id)
		{
		case IDR_TTF_DEFAULT:
			start =	&_binary_Resources_Oxygen_Regular_ttf_start;
			end = 	&_binary_Resources_Oxygen_Regular_ttf_end;
		break;
		case IDR_TTF_DEFAULT_BOLD:
			start =	&_binary_Resources_Oxygen_Bold_ttf_start;
			end = 	&_binary_Resources_Oxygen_Bold_ttf_end;		
		break;
		case IDR_TTF_MONO:
			start =	&_binary_Resources_FiraMono_Regular_ttf_start;
			end = 	&_binary_Resources_FiraMono_Regular_ttf_end;
		break;
		
		case IDB_PNG_WIDGET8x12:
			start =	&_binary_Resources_widget8x12_png_start;
			end = 	&_binary_Resources_widget8x12_png_end;
		break;
		case IDB_PNG_WIDGET15x15:
			start =	&_binary_Resources_widget15x15_png_start;
			end = 	&_binary_Resources_widget15x15_png_end;
		break;
		case IDB_PNG_WIDGET24x24:
			start =	&_binary_Resources_widget24x24_png_start;
			end = 	&_binary_Resources_widget24x24_png_end;
		break;
		}
		
		ret.data = start;
		ret.size = end-start;
		return ret;
	}
}
