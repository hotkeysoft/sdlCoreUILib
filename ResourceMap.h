#pragma once

#include "resource.h"
#include <string>
#include <vector>

namespace ResourceMap
{
	enum ResourceType
	{
		RES_IMAGE,
		RES_IMAGEMAP,
		RES_CURSOR,
		RES_FONT,
	};

	struct ResourceInfo
	{
		int winResId;
		const char * winResType;
		const char * id;
		ResourceType type;

		int i1; // tileWidth / tileHeight for imagemap, size for fonts, etc.
		int i2;
	};

	extern std::vector<ResourceInfo> g_ResourceMap;
}
