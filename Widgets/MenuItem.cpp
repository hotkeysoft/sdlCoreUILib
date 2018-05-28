#include "stdafx.h"
#include <SDL.h>
#include "MenuItem.h"
#include "Label.h"
#include "Image.h"
#include "Util\RenderTarget.h"

#define MENUICONSIZE 16

namespace CoreUI
{
	MenuItem::MenuItem(RendererRef renderer, const char * id, const char * name, ImageRef image, MenuItemRef parent) : 
		Widget(id, renderer, parent, Rect(), name, image), m_opened(false)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}
		
		m_margin = 0;
		m_padding = 4;
		m_showBorder = false;
		m_borderWidth = 1;
	}

	MenuItemPtr MenuItem::Create(RendererRef renderer, const char * id, const char * name, ImageRef image, MenuItemRef parent)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer, id, name, image, parent);
		return std::static_pointer_cast<MenuItem>(ptr);
	}

	void MenuItem::Init()
	{
		m_label = Label::CreateAutoSize("l", m_renderer, m_text.c_str(), nullptr, Label::TEXT_AUTOSIZE_DEFAULT, LCF_MENUITEM);
		m_label->SetBackgroundColor(m_backgroundColor);
		m_label->SetForegroundColor(m_foregroundColor);
		m_label->SetPadding(Dimension(8, 2));
		m_label->Init();

		m_activeLabel = Label::CreateAutoSize("la", m_renderer, m_text.c_str(), nullptr, Label::TEXT_AUTOSIZE_DEFAULT, LCF_MENUITEM);
		m_activeLabel->SetBackgroundColor(m_selectedBgColor);
		m_activeLabel->SetForegroundColor(m_selectedFgColor);
		m_activeLabel->SetPadding(Dimension(8, 2));
		m_activeLabel->Init();
		
		m_renderedMenu = nullptr;
		m_renderedMenuRect = Rect();
	}

	void MenuItem::Draw(const PointRef pos)
	{
		if (!m_renderedMenu)
		{
			Render(false);
			Render(true);
		}

		if (m_renderedMenu)
		{
			m_renderedMenuRect.x = pos->x;
			m_renderedMenuRect.y = pos->y;
			SDL_RenderCopy(m_renderer, m_renderedMenu.get(), nullptr, &m_renderedMenuRect);

			if (HasSubMenu())
			{
				for (auto & item : m_items)
				{
					Point subMenuPos = *pos;
					if (item && item->IsOpened() && item->HasSubMenu())
					{
						subMenuPos.y = item->m_rect.y + m_renderedMenuRect.y;
						subMenuPos.x += m_renderedMenuRect.w;
						item->Draw(&subMenuPos);
					}
				}
			}
		}
	}

	bool MenuItem::Hit(PointRef pt)
	{
 		if (m_labelRect.PointInRect(pt))
			return true;

		if (IsOpened())
		{
			if (m_renderedMenuRect.PointInRect(pt))
			{
				return true;
			}

			for (auto & item : m_items)
			{
				if (item && item->IsOpened() && item->HasSubMenu() && item->Hit(pt))
				{
					return true;
				}
			}		
		}

		return false;
	}

	MenuItemPtr MenuItem::ItemAt(PointRef pt)
	{
		if (m_labelRect.PointInRect(pt))
		{
			return shared_from_this();
		}

		if (IsOpened())
		{
			Point rel(pt->x - m_renderedMenuRect.x, pt->y - m_renderedMenuRect.y);

			for (auto & item : m_items)
			{
				if (item)
				{
					Rect & itemRect = item->m_rect;
					if (itemRect.PointInRect(&rel))
					{
						return item;
					}

					if (item->IsOpened() && item->HasSubMenu() && item->Hit(pt))
					{
						return item->ItemAt(pt);
					}
				}
			}
		}

		return nullptr;
	}

	MenuItemPtr MenuItem::AddMenuItem(const char * id, const char * name, SDL_Keycode hotkey)
	{
		return AddMenuItem(id, name, nullptr, hotkey);
	}

	MenuItemPtr MenuItem::AddMenuItem(const char * id, const char * name, ImageRef image, SDL_Keycode hotkey)
	{
		m_renderedMenu = nullptr; // Force render next time menu is drawn

		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		MenuItemPtr item = MenuItem::Create(m_renderer, id, name, image, this);
		item->Init();

		m_items.push_back(item);

		if (hotkey != SDLK_UNKNOWN)
		{
			m_hotkeys[hotkey] = item;
		}

		return item;
	}

	void MenuItem::AddSeparator()
	{
		m_renderedMenu = nullptr; // Force render next time menu is drawn

		if (m_items.empty())
		{
			throw std::invalid_argument("Can't start menu with separator");
		}

		if (*m_items.rbegin() == nullptr)
		{
			throw std::invalid_argument("Can't have two consecutive separators");
		}

		m_items.push_back(nullptr);
	}

	// TODO: Sooo many constants...
	void MenuItem::Render(bool active)
	{
		m_renderedMenuRect = Rect();

		m_renderedMenuRect.w = m_label->GetRect(true, false).w;
		for (auto & item : m_items)
		{
			if (item)
			{
				item->Init();
				Rect labelRect = item->m_label->GetRect(true, false);
				labelRect.w += MENUICONSIZE + 8; // Space for icon + right arrow for submenus
				m_renderedMenuRect.w = std::max(labelRect.w, m_renderedMenuRect.w);
				m_renderedMenuRect.h += labelRect.h;
			}
			else
			{
				m_renderedMenuRect.h += 4;
			}
		}
		m_renderedMenuRect.h = std::max(m_renderedMenuRect.h, 0);
		m_renderedMenuRect.w += GetShrinkFactor().w * 2;
		m_renderedMenuRect.h += GetShrinkFactor().h * 2;

		TextureRef texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_renderedMenuRect.w, m_renderedMenuRect.h);
		if (texture)
		{
			RenderTarget target(m_renderer, texture);
			if (target)
			{			
				DrawFilledRect(&m_renderedMenuRect, active ? m_selectedBgColor : m_backgroundColor);
				// Icon area
				Rect iconArea = m_renderedMenuRect;
				iconArea.w = MENUICONSIZE + GetShrinkFactor().w*2 + 2;
				SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
				DrawFilledRect(&iconArea, Color(255, 255, 255, 45));
				SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
				Draw3dFrame(&m_renderedMenuRect, true);

				Rect target = m_renderedMenuRect.Deflate(GetShrinkFactor());

				for (auto & item : m_items)
				{
					if (item)
					{
						Rect labelRect = item->m_label->GetRect(true, false);
						target.h = labelRect.h;

						if (item->m_image)
						{
							Rect imageRect = target;
							imageRect.x += 2;
							imageRect.w = MENUICONSIZE + 2;
							item->m_image->Draw(&imageRect, Image::IMG_V_CENTER | Image::IMG_H_CENTER);
						}

						{
							labelRect = Rect(target.x + MENUICONSIZE + 6, target.y, target.w - (MENUICONSIZE + 6), target.h);
							LabelPtr & label = active ? item->m_activeLabel : item->m_label;

							label->Draw(&labelRect, true);
							item->m_rect = target;
						}
						if (item->HasSubMenu())
						{
							static ImageRef image = RES().FindImage("coreUI.widget8x12", 1);
							if (image)
							{
								image->Draw(&target.Deflate(1), Image::IMG_H_RIGHT | Image::IMG_V_CENTER);
							}
							else
							{
								std::cerr << "Image map not found: coreUI.widget8x12" << std::endl;
							}
						}

						target.y += labelRect.h;
					}
					else
					{
						Rect separator(target.x, target.y+1, target.w, 2);
						Draw3dFrame(&separator, false);
						target.y += 4;
					}
				}

				if (active)
				{
					m_renderedActiveMenu = TexturePtr(texture, sdl_deleter());
				}
				else
				{
					m_renderedMenu = TexturePtr(texture, sdl_deleter());
				}
				
			}
			else
			{
				std::cerr << "Unable to set render target" << std::endl;
			}
		}
		else
		{
			std::cerr << "Unable to create texture target" << std::endl;
		}
	}

	struct MenuItem::shared_enabler : public MenuItem
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: MenuItem(std::forward<Args>(args)...)
		{
		}
	};
}