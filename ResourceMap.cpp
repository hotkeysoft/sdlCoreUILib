#include "stdafx.h"
#include "ResourceMap.h"

namespace ResourceMap
{
	// Load fonts first because image / imagemaps are widgets that need the default font
	std::vector<ResourceInfo> g_ResourceMap =
	{
		// Fonts
		{ IDR_TTF_DEFAULT,		"TTF",	"coreUI.default",		RES_FONT, 14 },
		{ IDR_TTF_DEFAULT_BOLD,	"TTF",	"coreUI.defaultBold",	RES_FONT, 14 },
		{ IDR_TTF_MONO,			"TTF",	"coreUI.mono",			RES_FONT, 14 },

		// Images

		// Image Lists
		{ IDB_PNG_WIDGET8x12,	"PNG",	"coreUI.widget8x12",	RES_IMAGEMAP, 8, 12 },
		{ IDB_PNG_WIDGET15x15,	"PNG",	"coreUI.widget15x15",	RES_IMAGEMAP, 15, 15 },
		{ IDB_PNG_WIDGET24x24,	"PNG",	"coreUI.widget24x24",	RES_IMAGEMAP, 24, 24 },
	};
}
