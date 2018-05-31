#include "stdafx.h"
#include "Point.h"
#include "SDL_rect.h"
#include <sstream>

namespace CoreUI
{
	bool Point::IsEqual(const PointRef other) const
	{
		return (other != nullptr) &&
			(this->x == other->x) &&
			(this->y == other->y);
	}

	std::string Point::ToString() const
	{
		std::ostringstream os;
		os << "P(" << this->x << "," << this->y << ")";
		return os.str();
	}
}