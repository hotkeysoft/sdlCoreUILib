#include "stdafx.h"
#include "Rect.h"
#include "SDL_rect.h"
#include <sstream>

namespace CoreUI
{
	bool Rect::IsEmpty() const
	{
		return SDL_RectEmpty(this);
	}

	bool Rect::IsEqual(const RectRef other) const
	{
		return SDL_RectEquals(this, other);
	}

	Rect Rect::IntersectRect(const RectRef otherRect)
	{
		Rect ret;
		SDL_IntersectRect(this, otherRect, &ret);
		return ret;
	}

	bool Rect::PointInRect(const PointRef pt, const RectRef rect)
	{
		return SDL_PointInRect(pt, rect);
	}

	bool Rect::IntersectRect(const RectRef r1, const RectRef r2, const RectRef out)
	{
		return SDL_IntersectRect(r1, r2, out);
	}

	Rect Rect::CenterInTarget(const RectRef target, bool hCenter, bool vCenter)
	{
		int xOffset = hCenter ? ((target->w - w) / 2) : 0;
		int yOffset = vCenter ? ((target->h - h) / 2) : 0;
		Rect centered(
			target->x + xOffset, 
			target->y + yOffset,
			w, h);

		return centered;
	}

	std::string Rect::ToString() const
	{
		std::ostringstream os;
		os << "RECT(xy[" << this->x << "," << this->y << "],wh[" << this->w << "," << this->h << "])";
		return os.str();
	}
}