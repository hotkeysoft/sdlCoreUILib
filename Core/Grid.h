#pragma once
#include "CoreUI.h"
#include "Rect.h"
#include <string>
#include <ostream>

namespace CoreUI
{
	class DllExport Grid
	{
	public:
		Grid() : m_size(1), m_show(false), m_snap(false) {}

		int GetSize() { return m_size; }
		void SetSize(int size) { m_size = (std::max)(1, size); }

		bool IsVisible() { return m_show; }
		void Show(bool show) { m_show = show; }

		bool IsSnap() { return m_snap; }
		void Snap(bool snap) { m_snap = snap; }
		Point Snap(const PointRef);

		std::string ToString() const;

	private:
		int RoundUp(int);

		int m_size;
		bool m_show;
		bool m_snap;
	};

	inline std::ostream & operator << (std::ostream & os, const Grid& grid)
	{
		os << grid.ToString();
		return os;
	}
}