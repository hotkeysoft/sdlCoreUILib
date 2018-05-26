#include "stdafx.h"
#include <SDL.h>
#include "Util\ClipRect.h"
#include "Menu.h"
#include "Label.h"
#include "MenuItem.h"

namespace CoreUI
{
	Menu::Menu(RendererRef renderer, const char * id) : 
		Widget(id, renderer, nullptr, Rect(), nullptr), 
		m_lineHeight(0),
		m_active(nullptr)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}

		m_padding = Dimension(2, 2);
		m_margin = 0;
	}

	MenuPtr Menu::Create(RendererRef renderer, const char * id)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer, id);
		return std::static_pointer_cast<Menu>(ptr);
	}

	void Menu::Init()
	{
		if (m_lineHeight == 0)
		{
			// Fallback
			m_lineHeight = TTF_FontLineSkip(m_font);
		}
	}

	int Menu::GetHeight(int clientWidth) const
	{ 
		clientWidth -= (2 * GetShrinkFactor().w);
		int currX = 0;
		int lineCount = 1;
		for (auto & item : m_items)
		{
			int itemWidth = item->m_label->GetRect(true, false).w;
			currX += itemWidth;
			if (currX > clientWidth)
			{
				++lineCount;
				currX = itemWidth;
			}
		}

		return (m_lineHeight * lineCount) + (2 * GetShrinkFactor().h); 
	}

	void Menu::Draw(const RectRef rect)
	{
		Rect drawRect = *rect;
		drawRect.h = GetHeight(rect->w);
		Draw3dFrame(&drawRect, true);

		drawRect = drawRect.Deflate(GetShrinkFactor());
		m_rect = drawRect;
	
		MenuItemPtr openedMenu = nullptr;

		ClipRect clip(m_renderer, &drawRect);
		if (clip)
		{
			int currX = 0;
			int originalX = drawRect.x;
			drawRect.h = m_lineHeight;

			for (auto & item : m_items)
			{
				Rect labelRect = item->GetLabel()->GetRect(true, false);
				drawRect.w = labelRect.w;

				currX += labelRect.w;
				if (currX > m_rect.w)
				{
					currX = labelRect.w;
					drawRect.x = originalX;
					drawRect.y += m_lineHeight;
				}

				item->m_label->Draw(&drawRect);
				
				item->m_labelRect = drawRect;

				if (item->IsOpened())
				{
					Draw3dFrame(&item->m_labelRect, false);
					openedMenu = item;
				}

				drawRect.x += labelRect.w;
			}
		}

		DrawOpenedMenu(openedMenu);

	}

	void Menu::DrawOpenedMenu(CoreUI::MenuItemPtr & item)
	{
		// Menu is on top of everything so no need for bounding rect
		//TODO: Get main window size
		ClipRect noclip(m_renderer, &Rect(0, 0, 2000, 2000), false);
		if (noclip)
		{
			if (item)
			{
				//Draw3dFrame(&item->m_labelRect, false);
				Point menuPos(item->m_labelRect.Origin());
				menuPos.y += (m_lineHeight);

				item->Draw(&menuPos);
			}

			if (m_active)
			{
				DrawActiveFrame(m_active);
			}
		}
	}

	void Menu::DrawActiveFrame(MenuItemRef item)
	{
		MenuItemRef parent = item->GetParentMenuItem();
		if (parent)
		{
			Rect highlight = item->m_rect.Offset(&parent->m_renderedMenuRect.Origin());
			Draw3dFrame(&highlight, false);
			DrawActiveFrame(parent);
		}
	}

	MenuItemPtr Menu::AddMenuItem(const char * id, const char * name, SDL_Keycode hotkey)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		MenuItemPtr item = MenuItem::Create(m_renderer, id, name, nullptr);
		item->SetParent(this);
		item->Init();
		m_lineHeight = item->m_label->GetRect(true, false).h;

		m_items.push_back(item);

		if (hotkey != SDLK_UNKNOWN)
		{
			m_hotkeys[hotkey] = item;
		}

		return item;
	}

	HitResult Menu::HitTest(const PointRef pt)
	{		
		if (m_rect.PointInRect(pt))
		{
			return HitResult(HitZone::HIT_MENU, this);
		}

		for (auto & item : m_items)
		{
			if (item->Hit(pt))
			{
				return HitResult(HitZone::HIT_MENU_ITEM, item.get());
			}
		}

		return HitZone::HIT_NOTHING;
	}

	MenuItemPtr Menu::ItemAt(PointRef pt)
	{
		auto it = std::find_if(m_items.begin(), m_items.end(), [pt](MenuItemPtr item) { return item->Hit(pt); });
		if (it != m_items.end())
		{
			return (*it)->ItemAt(pt);
		}

		return nullptr;
	}

	bool Menu::HandleEvent(SDL_Event * e)
	{
		Point pt(e->button.x, e->button.y);
		HitResult hit = HitTest(&pt);
		const CaptureInfo & capture = WINMGR().GetCapture();
		switch (e->type)
		{
		case SDL_MOUSEMOTION:
			if (hit)
			{
				SDL_SetCursor(RES().FindCursor("default"));
				if (capture)
				{
					MenuItemPtr item = ItemAt(&pt);
					if (item && (hit & (HIT_MENU | HIT_MENU_ITEM)))
					{
						m_active = item.get();
						OpenMenu(m_active);
					}
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		{
			MenuItemPtr item = ItemAt(&pt);

			if (capture)
			{
				if (item && item->GetParentMenuItem() && item->IsOpened() && item->HasSubMenu())
				{
					return true;
				}
				CloseMenu();
				WINMGR().ReleaseCapture();
				if (item && hit == HIT_MENU_ITEM)
				{
					PostEvent(EVENT_MENU_SELECTED, item.get());
				}
				// Eat the click anyway so user doesn't accidentally activate a control outside the menu area
				return true;
			}

			SetActive();
			SetFocus(this);

			if (item)
			{
				m_active = item.get();
				OpenMenu(m_active);
				WINMGR().StartCapture(hit, &pt);
			}

			return item != nullptr;
		}
		case SDL_KEYDOWN:
		{
			if (capture)
			{
				switch (e->key.keysym.sym)
				{
				case SDLK_ESCAPE:
					CloseMenu();
					WINMGR().ReleaseCapture();
					return true;
				case SDLK_LEFT:
					MoveLeft();
					return true;
				case SDLK_RIGHT:
					MoveRight();
					return true;
				case SDLK_UP:
					MoveUp();
					return true;
				case SDLK_DOWN:
					MoveDown();
					return true;
				case SDLK_RETURN:
					if (m_active)
					{
						WINMGR().ReleaseCapture();
						PostEvent(EVENT_MENU_SELECTED, m_active);
						CloseMenu();
						return true;
					}
				}
			}
			// Hotkeys
			{
				// ALT - main menu
				if (SDL_GetModState() & KMOD_ALT)
				{
					auto it = m_hotkeys.find(e->key.keysym.sym);
					if (it != m_hotkeys.end())
					{
						SetActive();
						SetFocus(this);
						m_active = it->second.get();
						OpenMenu(m_active);
						WINMGR().StartCapture(HitResult(HIT_MENU, this), &pt);
						return true;
					}
				} 
				// Plain key - sub menu
				else if (m_active)
				{
					MenuItemRef parent = m_active->GetParentMenuItem();
					if (parent == nullptr)
						parent = m_active;

					auto it = parent->m_hotkeys.find(e->key.keysym.sym);
					if (it != parent->m_hotkeys.end())
					{
						SetActive();
						SetFocus(this);
						m_active = it->second.get();
						if (m_active->HasSubMenu())
						{
							OpenMenu(m_active);
							MoveRight();
							WINMGR().StartCapture(HitResult(HIT_MENU, this), &pt);
						}
						else
						{
							PostEvent(EVENT_MENU_SELECTED, m_active);
							CloseMenu();
						}

						return true;
					}
				}
			}

			return false;
		}
		default:
			return false;
		}

		return true;
	}

	void Menu::OpenMenu(MenuItemRef item)
	{
		if (!item)
			return;

		// Close other menus on same level
		MenuItemRef parent = item->GetParentMenuItem();
		if (parent)
		{
			for (auto & it : parent->m_items)
			{
				if (it.get() != item)
				{
					CloseMenuItem(it.get());
				}
			}
		}
		else
		{
			for (auto & it : m_items)
			{
				if (it.get() != item)
				{
					CloseMenuItem(it.get());
				}
			}
		}

		item->m_opened = true;
	}

	void Menu::CloseMenuItem(MenuItemRef item)
	{
		if (!item)
			return;

		// Close menu item and all children
		item->m_opened = false;
		for (auto & it : item->m_items)
		{
			if (it)
			{
				CloseMenuItem(it.get());
			}
		}
	}

	void Menu::CloseMenu()
	{
		m_active = nullptr;
		for (auto & item : m_items)
		{
			if (item)
			{
				item->m_opened = false;
				for (auto & it : item->m_items)
				{
					if (it)
					{
						CloseMenuItem(it.get());
					}
				}
			}
		}
	}
	
	Menu::MenuItems::const_iterator Menu::FindMenuItem(MenuItemRef item, MenuItemRef parent) const
	{
		return std::find_if(
			parent ? parent->m_items.begin() : m_items.begin(),
			parent ? parent->m_items.end() : m_items.end(),
			[item](MenuItemPtr it) { return it.get() == item; });
	}

	void Menu::MoveRight()
	{
		if (!m_active)
			return;

		// Top level
		if (m_active->GetParentMenuItem() == nullptr)
		{
			auto item = FindMenuItem(m_active);
			if (item == m_items.end())
				return;

			item++;
			if (item == m_items.end())
			{
				item = m_items.begin();
			}
			m_active = item->get();
			OpenMenu(m_active);
		}
		else
		{
			OpenMenu(m_active);
			if (m_active->HasSubMenu())
			{
				m_active = m_active->m_items.begin()->get();
			}
		}
	}

	void Menu::MoveLeft()
	{
		if (!m_active)
			return;

		// Top level, move left right
		if (m_active->GetParentMenuItem() == nullptr)
		{
			auto item = FindMenuItem(m_active);
			if (item == m_items.end())
				return;

			item = (item == m_items.begin()) ? --m_items.end() : --item;
			m_active = item->get();
			OpenMenu(m_active);
		}
		else if (m_active->IsOpened() && m_active->HasSubMenu()) // submenu, close if open
		{
			CloseMenuItem(m_active);
		}
		else
		{
			m_active = m_active->GetParentMenuItem();
			if (m_active->GetParentMenuItem())
			{
				CloseMenuItem(m_active);
			}
		}
	}

	void Menu::MoveUp()
	{
		if (!m_active)
			return;

		MenuItemRef parent = m_active->GetParentMenuItem();
		if (parent == nullptr)
		{
			if (!m_active->m_items.empty())
			{
				m_active = (--m_active->m_items.end())->get();
				OpenMenu(m_active);
			}
		}
		else
		{

			auto item = FindMenuItem(m_active, parent);
			if (item == parent->m_items.end())
				return;

			item = (item == parent->m_items.begin()) ? --parent->m_items.end() : --item;
			if (*item == nullptr)
			{
				item = (item == parent->m_items.begin()) ? --parent->m_items.end() : --item;
			}

			m_active = item->get();
			OpenMenu(m_active);
		}
	}

	void Menu::MoveDown()
	{
		if (!m_active)
			return;

		MenuItemRef parent = m_active->GetParentMenuItem();
		if (parent == nullptr)
		{ 
			if (!m_active->m_items.empty())
			{
				m_active = m_active->m_items.begin()->get();
				OpenMenu(m_active);
			}
		}
		else
		{
			auto item = FindMenuItem(m_active, parent);
			if (item == parent->m_items.end())
				return;

			++item;
			if (item == parent->m_items.end())
			{
				item = parent->m_items.begin();
			}
			if (*item == nullptr)
			{
				++item;
				if (item == parent->m_items.end())
				{
					item = parent->m_items.begin();
				}
			}

			m_active = item->get();
			OpenMenu(m_active);
		}
	}

	struct Menu::shared_enabler : public Menu
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: Menu(std::forward<Args>(args)...)
		{
		}
	};
}