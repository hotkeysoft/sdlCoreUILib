#include "stdafx.h"
#include "SDL.h"
#include "Core/Point.h"
#include "Core/Rect.h"
#include "Core/Window.h"
#include "Core/ResourceManager.h"
#include "Image.h"
#include "ToolbarItem.h"
#include <algorithm>

namespace CoreUI
{
	ToolbarItem::ToolbarItem(const char * id, RendererRef renderer, const char * label, ImageRef image, FontRef font) :
		Button(id, renderer, Rect(), label, image, font, WIN_AUTOSIZE | WIN_NOFOCUS)
	{
	}

	void ToolbarItem::Init()
	{
		Button::Init();
		m_borderWidth = 1;
	}

	ToolbarItemPtr ToolbarItem::Create(const char * id, RendererRef renderer, const char * label, ImageRef image, FontRef font)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, label, image, font);
		return std::static_pointer_cast<ToolbarItem>(ptr);
	}

	void ToolbarItem::Draw(const RectRef rect)
	{
		Button::Draw(rect);
		m_drawLocation = *rect;
	}

	HitResult ToolbarItem::HitTest(const PointRef pt)
	{
		if (m_drawLocation.PointInRect(pt))
		{
			return HitResult(HitZone::HIT_CONTROL, this);
		}

		return HitZone::HIT_NOTHING;
	}

	struct ToolbarItem::shared_enabler : public ToolbarItem
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: ToolbarItem(std::forward<Args>(args)...)
		{
		}
	};
}