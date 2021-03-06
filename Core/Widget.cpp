#include "stdafx.h"
#include "Color.h"
#include "Point.h"
#include "Rect.h"
#include "Widget.h"
#include "WindowManager.h"
#include "Tooltip.h"
#include "Widgets/Image.h"

namespace CoreUI
{
	Widget::Widget(const char* id) :
		m_id(id ? id : ""), m_eventClassId(Uint32(-1))
	{
	}

	Widget::Widget(const char* id, RendererRef renderer, WidgetRef parent,
		Rect rect, const char* text, ImageRef image, FontRef font, CreationFlags flags) :
		m_id(id ? id : ""),
		m_renderer(renderer),
		m_parent(parent),
		m_rect(rect),
		m_text(text ? text : ""),
		m_image(image),
		m_showBorder(false),
		m_foregroundColor(Color::C_BLACK),
		m_backgroundColor(Color::C_LIGHT_GREY),
		m_borderColor(Color::C_BLACK),
		m_selectedBgColor(Color(90, 128, 128)),
		m_selectedFgColor(Color::C_WHITE),
		m_padding(0),
		m_margin(0),
		m_minSize(1),
		m_borderWidth(4),
		m_focused(false),
		m_flags(flags),
		m_eventClassId(Uint32(-1))
	{
		m_tag.o = nullptr;
		SetFont(font);
	}

	void Widget::SetParent(WidgetRef parent)
	{
		if (m_parent)
		{
			throw std::invalid_argument("widget already has parent");
		}
		m_parent = parent;
	}

	void Widget::SetFocus(WidgetRef focus, WidgetRef parent)
	{
		if (m_flags & WIN_NOFOCUS)
			return;

		if (parent == nullptr)
		{
			m_focused = true;
		}

		if (m_parent)
		{
			// Bubble up until we hit a window
			m_parent->SetFocus(focus, this);
		}
	}

	void Widget::ClearFocus()
	{
		m_focused = false;
	}

	void Widget::SetFont(FontRef font)
	{
		if (font == nullptr)
		{
			m_font = RES().FindFont("coreUI.default");
			if (m_font == nullptr)
			{
				throw std::runtime_error("Unable to load default font");
			}
		}
		else
		{
			m_font = font;
		}
	}

	void Widget::SetDrawColor(const CoreUI::Color & col)
	{
		SDL_SetRenderDrawColor(m_renderer, col.r, col.g, col.b, col.a);
	}

	void Widget::Draw3dFrame(const RectRef pos, bool raised, const CoreUI::Color & col)
	{
		// Relief effect
		Point points[5] = { { pos->x, pos->y + pos->h - 1 },{ pos->x + pos->w - 1, pos->y + pos->h - 1 },
		{ pos->x + pos->w - 1, pos->y },{ pos->x, pos->y },{ pos->x, pos->y + pos->h - 2 } };

		SetDrawColor(raised ? col.Darken() : Color::C_WHITE);
		SDL_RenderDrawLines(m_renderer, points + 0, 3);

		SetDrawColor(raised ? Color::C_WHITE : col.Darken());
		SDL_RenderDrawLines(m_renderer, points + 2, 3);
	}

	void Widget::DrawRect(const RectRef pos, const CoreUI::Color & col, int borderWidth)
	{
		SetDrawColor(col);

		SDL_RenderDrawRect(m_renderer, pos);

		if (borderWidth > 1)
		{
			Rect innerRect = *pos;
			for (int i = 1; i < borderWidth; ++i)
			{
				innerRect = innerRect.Deflate(1);
				SDL_RenderDrawRect(m_renderer, &innerRect);
			}
		}
	}

	void Widget::DrawFilledRect(const RectRef pos, const CoreUI::Color & col)
	{
		SetDrawColor(col);

		SDL_RenderFillRect(m_renderer, pos);
	}

	void Widget::DrawReliefBox(const RectRef pos, const CoreUI::Color & col, bool raised)
	{
		SetDrawColor(col);

		// Render rect
		SDL_RenderFillRect(m_renderer, pos);

		Draw3dFrame(pos, !raised);

		if (m_borderWidth > 1)
		{
			int offset = m_borderWidth - 1;
			Rect frame(pos->x + offset,
				pos->y + offset,
				pos->w - (2 * offset),
				pos->h - (2 * offset));
			Draw3dFrame(&frame, raised);
		}
	}

	void Widget::DrawButton(const RectRef pos, const CoreUI::Color & col, ImageRef image, bool raised, int thickness)
	{
		SetDrawColor(col);

		// Render rect
		SDL_RenderFillRect(m_renderer, pos);

		for (int i = 0; i < thickness; ++i)
		{
			Draw3dFrame(&pos->Deflate(i), raised);
		}
		
		if (image && image->IsSet())
		{
			image->Draw(&Point(pos->x + 1, pos->y + 1));
		}
	}

	TexturePtr Widget::SurfaceToTexture(SDL_Surface* surf, bool writable)
	{
		TexturePtr texture = TexturePtr(SDL_CreateTextureFromSurface(m_renderer, surf), sdl_deleter());

		SDL_FreeSurface(surf);

		return writable ? std::move(CloneTexture(texture)) : std::move(texture);
	}

	TexturePtr Widget::CloneTexture(TexturePtr source, Color background)
	{
		Uint32 format;
		Rect rect;

		SDL_QueryTexture(source.get(), &format, NULL, &rect.w, &rect.h);
		TextureRef clone = SDL_CreateTexture(m_renderer, format, SDL_TEXTUREACCESS_TARGET, rect.w, rect.h);
		if (clone && (SDL_SetRenderTarget(m_renderer, clone) == 0))
		{
			if (background.IsTransparent())
			{
				SDL_SetTextureBlendMode(clone, SDL_BLENDMODE_BLEND);
			}
			SetDrawColor(m_backgroundColor);

			if (SDL_SetRenderTarget(m_renderer, clone) == 0)
			{
				SDL_RenderClear(m_renderer); // Needed? transparent vs opaque bg
				SDL_RenderCopy(m_renderer, source.get(), &rect, &rect);
				SDL_SetRenderTarget(m_renderer, nullptr);
			}
		}
		return std::move(TexturePtr(clone, sdl_deleter()));
	}


	Rect Widget::GetRect(bool relative, bool scrolled) const
	{
		if (m_parent == nullptr || (relative && !scrolled))
		{
			return m_rect;
		}

		Rect parent = m_parent->GetClientRect(relative, scrolled);
		return m_rect.Offset(&parent.Origin());
	}

	Rect Widget::GetClientRect(bool relative, bool scrolled) const
	{
		return GetRect(relative, scrolled);
	}

	bool Widget::MovePos(PointRef pos)
	{
		bool clip = false;

		m_rect.x = pos->x;
		m_rect.y = pos->y;

		if (m_rect.x < 0)
		{
			clip = true;
			m_rect.x = 0;
		}

		if (m_rect.y < 0)
		{
			clip = true;
			m_rect.y = 0;
		}

		return !clip;
	}

	bool Widget::MoveRel(PointRef rel)
	{
		bool clip = false;

		m_rect.x += rel->x;
		m_rect.y += rel->y;

		if (m_rect.x < 0)
		{
			clip = true;
			m_rect.x = 0;
		}

		if (m_rect.y < 0)
		{
			clip = true;
			m_rect.y = 0;
		}

		return !clip;
	}

	bool Widget::ResizeRel(PointRef rel)
	{
		bool clip = false;

		m_rect.w += rel->x;
		m_rect.h += rel->y;

		if (m_rect.w < m_minSize.w)
		{
			clip = true;
			m_rect.w = m_minSize.w;
		}

		if (m_rect.h < m_minSize.h)
		{
			clip = true;
			m_rect.h = m_minSize.h;
		}

		return !clip;
	}

	bool Widget::Resize(PointRef size)
	{
		bool clip = false;

		m_rect.w = size->x;
		m_rect.h = size->y;

		if (m_rect.w < m_minSize.w)
		{
			clip = true;
			m_rect.w = m_minSize.w;
		}

		if (m_rect.h < m_minSize.h)
		{
			clip = true;
			m_rect.h = m_minSize.h;
		}

		return !clip;
	}

	bool Widget::MoveRect(RectRef rect)
	{
		{
			Point origin = m_rect.Origin();
			m_rect.x = rect->x;
			m_rect.y = rect->y;

			if (m_rect.x < 0)
			{
				m_rect.x = 0;
				m_rect.w = rect->w + rect->x;
			}
			else
			{
				m_rect.w = rect->w;
			}

			if (m_rect.y < 0)
			{
				m_rect.y = 0;
				m_rect.h = rect->h + rect->y;
			}
			else
			{
				m_rect.h = rect->h;
			}

			if (m_rect.w < m_minSize.w)
			{
				m_rect.w = m_minSize.w;
				m_rect.x = origin.x;
			}

			if (m_rect.h < m_minSize.h)
			{
				m_rect.h = m_minSize.h;
				m_rect.y = origin.y;
			}
		}
		return false;
	}

	Uint32 Widget::GetEventClassId()
	{
		if (m_eventClassId == (Uint32)-1)
		{
			m_eventClassId = WINMGR().GetEventType(GetClassName());
		}
		return m_eventClassId;
	}

	void Widget::PostEvent(EventCode code, void * data2)
	{
		SDL_Event toPost;
		SDL_zero(toPost);

		toPost.type = GetEventClassId();
		if (toPost.type == (Uint32)-1)
		{
			throw std::invalid_argument("class not registered");
		}

		toPost.user.code = code;
		toPost.user.data1 = this;
		toPost.user.data2 = data2;
		
		SDL_PushEvent(&toPost);
	}

	bool Widget::HandleEvent(SDL_Event* e)
	{
		static Uint32 timerEventID = WINMGR().GetEventType(Timer::EventClassName());

		// Tooltip processing
		if ((e->type == SDL_MOUSEMOTION) && m_tooltip.size() && (m_tooltipTimer == (Uint32)-1))
		{
			Point pt(e->button.x, e->button.y);
			HitResult hit = HitTest(&pt);
			if (hit)
			{
				// TODO: dynamic delay
				m_tooltipTimer = WINMGR().AddTimer(300, false, this);
			}
		}
		else if (e->type == timerEventID && e->user.code == m_tooltipTimer)
		{
			Point pt;
			SDL_GetMouseState(&pt.x, &pt.y);

			HitResult hit = HitTest(&pt);
			if (hit)
			{
				TOOLTIP().Show(this, pt, m_tooltip.c_str());
			}
			else
			{
				WINMGR().DeleteTimer(m_tooltipTimer);
				m_tooltipTimer = (Uint32)-1;
				TOOLTIP().Hide(this);
			}
		}

		return false;
	}
}