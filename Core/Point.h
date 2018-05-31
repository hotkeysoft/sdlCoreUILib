#pragma once
#include "Common.h"
#include "SDL_rect.h"
#include <string>
#include <ostream>

namespace CoreUI
{
	class DllExport Point : public SDL_Point
	{
	public:
		Point() : SDL_Point({ 0, 0 }) {}
		Point(int x, int y) : SDL_Point({ x, y }) {};

		bool IsEqual(const PointRef other) const;

		Point* operator&() { return this; }

		std::string ToString() const;
	};

	inline std::ostream & operator << (std::ostream & os, const Point& pt)
	{
		os << pt.ToString();
		return os;
	}
}