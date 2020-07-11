#include "stdafx.h"
#include "Grid.h"
#include <sstream>

namespace CoreUI
{
	int Grid::RoundUp(int val)
	{
		return ((val + m_size- 1) / m_size) * m_size;
	}

	Point Grid::Snap(const PointRef pt)
	{
		if (m_size == 1 || !m_snap)
			return *pt;

		return Point(RoundUp(pt->x), RoundUp(pt->y));
	}

	std::string Grid::ToString() const
	{
		std::ostringstream os;
		os << "GRID(" << m_size << ", show=" << m_show << ", snap=" << m_snap << ")";
		return os.str();
	}
}