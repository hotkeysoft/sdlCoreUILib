#include "stdafx.h"
#include <SDL.h>
#include "Core/WindowManager.h"
#include "Util/ClipRect.h"
#include "Toolbar.h"
#include "Image.h"
#include "ToolbarItem.h"

namespace CoreUI
{
	Toolbar::Toolbar(RendererRef renderer, const char * id, int height, CreationFlags flags) :
		Widget(id, renderer, nullptr, Rect(), nullptr, nullptr, nullptr, flags), m_height(height)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}

		if (((flags & WIN_AUTOSIZE) == 0) && (height < 8 || height > 128))
		{
			throw std::out_of_range("toolbar height out of range [8-128]");
		}	
	}

	ToolbarPtr Toolbar::Create(RendererRef renderer, const char * id, int height, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer, id, height, flags);
		return std::static_pointer_cast<Toolbar>(ptr);
	}

	ToolbarPtr Toolbar::CreateAutoSize(RendererRef renderer, const char * id, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer, id, 8, flags | WIN_AUTOSIZE);
		return std::static_pointer_cast<Toolbar>(ptr);
	}

	void Toolbar::Init()
	{
	}

	int Toolbar::GetHeight(int clientWidth) const
	{ 
		return m_height;
	}

	void Toolbar::Draw(const RectRef rect)
	{
		Rect drawRect = *rect;
		drawRect.h = GetHeight(rect->w);
		DrawFilledRect(rect, m_backgroundColor);
		Draw3dFrame(&drawRect, true, m_backgroundColor);

		drawRect = drawRect.Deflate(GetShrinkFactor());
		m_rect = drawRect;

		ClipRect clip(m_renderer, &drawRect);
		if (clip)
		{			
			for (auto & item : m_items)
			{
				if (item)
				{
					Rect itemRect = item->GetRect(true, false);
					drawRect.w = itemRect.w;
					item->Draw(&drawRect);
					drawRect.x += itemRect.w;
				}
				else
				{
					drawRect.x += m_borderWidth;
				}
			}
		}
	}

	ToolbarItemPtr Toolbar::AddToolbarItem(const char * id, ImageRef image, const char * name)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		if (FindByID(id) != m_items.end())
		{
			throw std::invalid_argument("tool bar item id already used: " + std::string(id));
		}

		if (image && !(m_flags & WIN_AUTOSIZE))
		{
			Rect imageRect = image->GetRect(false, false);
			if ((imageRect.h + (2*m_borderWidth)) > m_height || imageRect.w > 128)
			{
				throw std::invalid_argument("image too large for toolbar");
			}
		}

		ToolbarItemPtr item = ToolbarItem::Create(id, m_renderer, name, image);
		item->SetParent(this);
		item->Init();
		
		m_items.push_back(item);

		UpdateSize(item);

		return item;
	}

	Toolbar::ToolbarItems::const_iterator Toolbar::FindByID(const char * id) const
	{
		return std::find_if(m_items.begin(), m_items.end(),
			[id](ToolbarItemPtr it) { return it && it->GetId() == id; });
	}

	void Toolbar::AddSeparator()
	{
		m_items.push_back(nullptr);
	}

	void Toolbar::UpdateSize(ToolbarItemPtr item)
	{
		if (item && (m_flags & WIN_AUTOSIZE))
		{
			Rect itemRect = item->GetRect(true, false);
			m_height = std::max(itemRect.h, m_height);

			// Clip to sensible value
			m_height = std::min(128, m_height);
		}
	}

	HitResult Toolbar::HitTest(const PointRef pt)
	{		
		if (m_rect.PointInRect(pt))
		{
			return HitResult(HitZone::HIT_TOOLBAR, this);
		}

		return HitZone::HIT_NOTHING;
	}

	ToolbarItemPtr Toolbar::ItemAt(PointRef pt)
	{
		for (auto & item : m_items)
		{
			if (item && item->HitTest(pt))
			{
				return item;
			}
		}

		return nullptr;
	}

	bool Toolbar::HandleEvent(SDL_Event * e)
	{
		Point pt(e->button.x, e->button.y);
		HitResult hit = HitTest(&pt);
		if (hit)
		{
			ToolbarItemPtr item = ItemAt(&pt);
			if (item)
			{
				return item->HandleEvent(e);
			}
		}
		return false;
	}

	struct Toolbar::shared_enabler : public Toolbar
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: Toolbar(std::forward<Args>(args)...)
		{
		}
	};
}