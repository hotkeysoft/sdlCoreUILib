#pragma once

namespace WinUtil
{ 
	class WinResource
	{
	public:
		struct DllResource
		{
			DllResource() : data(nullptr), size(0) {}
			bool IsLoaded() const { return size != 0; }

			void * data;
			uint32_t size;
		};

		static DllResource LoadResource(int, const char * resourceType);

	};
}