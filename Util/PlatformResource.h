#pragma once

namespace PlatformUtil
{ 
	class PlatformResource
	{
	public:
		struct LibResource
		{
			LibResource() : data(nullptr), size(0) {}
			bool IsLoaded() const { return size != 0; }

			void * data;
			uint32_t size;
		};

		static LibResource LoadResource(int, const char * resourceType);

	};
}