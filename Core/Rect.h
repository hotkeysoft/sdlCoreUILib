#pragma once
#include "Common.h"
#include "Point.h"
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
	struct Dimension
	{
		enum Dim
		{
			DIM_W = 1,
			DIM_H = 2,
			DIM_ALL = DIM_W | DIM_H
		};

		Dimension(uint8_t all = 0) : w(all), h(all) {}
		Dimension(uint8_t w, uint8_t h) : w(w), h(h) {}

		void Set(uint8_t dim, Dim set = DIM_ALL)
		{
			if (set & DIM_W)
				w = dim;
			if (set & DIM_H)
				h = dim;
		}

		Dimension operator+(const Dimension& rhs) const { return Dimension(w + rhs.w, h + rhs.h); }

		explicit operator bool() const { return w || h; }

		uint8_t w;
		uint8_t h;
	};

	class DllExport Rect : public SDL_Rect
	{
	public:
		Rect() : SDL_Rect({ 0, 0, 0, 0 }) {}
		Rect(int x, int y, int w, int h) : SDL_Rect({ x, y, w, h }) {};

		void Clear() { x = y = w = h = 0; }

		bool IsEmpty() const;
		bool IsEqual(const RectRef other) const;

		bool PointInRect(const PointRef pt) { return SDL_PointInRect(pt, this); }
		Rect IntersectRect(const RectRef);

		Point Origin() const { return Point(x, y); }
		Point Size() const { return Point(w, h); }

		Rect Offset(const PointRef pt) const { return Rect(x + pt->x, y + pt->y, w, h); }
		Rect OffsetNeg(const PointRef pt) const { return Rect(x - pt->x, y - pt->y, w, h); }

		Rect Deflate(int size) const { return Rect(x + size, y + size, w - (2 * size), h - (2 * size)); }
		Rect Deflate(Dimension dim) const { return Rect(x + dim.w, y + dim.h, w - (2 * dim.w), h - (2 * dim.h)); }

		Rect CenterInTarget(const RectRef, bool hCenter = true, bool vCenter = true);

		static bool PointInRect(const PointRef, const RectRef);
		static bool IntersectRect(const RectRef r1, const RectRef r2, const RectRef out);

		std::string ToString() const;
	};

	inline std::ostream & operator << (std::ostream & os, const Rect& rect)
	{
		os << rect.ToString();
		return os;
	}
}