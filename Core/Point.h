#pragma once
#include "Common.h"
#include "SDL_rect.h"
#include <string>
#include <ostream>

#ifdef  COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#else
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#endif

namespace CoreUI
{
	class DllExport Point : public SDL_Point
	{
	public:
		Point() : SDL_Point({ 0, 0 }) {}
		Point(int x, int y) : SDL_Point({ x, y }) {};

		bool IsEqual(const PointRef other) const;

		std::string ToString() const;
	};

	inline std::ostream & operator << (std::ostream & os, const Point& pt)
	{
		os << pt.ToString();
		return os;
	}
}