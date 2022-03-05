#include "stdafx.h"
#include "SDL.h"
#include "Core/Point.h"
#include "Core/Rect.h"
#include "Core/ResourceManager.h"
#include "Core/Window.h"
#include "Util/ClipRect.h"
#include "Util/RenderTarget.h"
#include "Image.h"
#include "Label.h"
#include <algorithm>

namespace CoreUI
{
	Label::Label(const char * id, RendererRef renderer, Rect rect, const char * label, FontRef font, TextAlign align, CreationFlags flags) :
		Widget(id, renderer, nullptr, rect, label, nullptr, font, flags), m_labelAlign(align)
	{
		if ((flags & WIN_FILL) && (flags & WIN_AUTOSIZE))
		{
			throw std::invalid_argument("Incompatible creation flags");
		}

		m_backgroundColor = Color::C_TRANSPARENT;
		m_borderWidth = 1;

		m_margin = (m_flags & WIN_FILL) ? 5 : 0;
	}

	void Label::Init()
	{
		RenderLabel();
	}

	void Label::SetForegroundColor(Color color)
	{
		bool colorChanged = (color != m_foregroundColor);
		Widget::SetForegroundColor(color);

		if (colorChanged)
		{
			RenderLabel();
		}
	}

	LabelPtr Label::CreateSingle(const char * id, RendererRef renderer, Rect rect, const char * label, FontRef font, TextAlign align, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, rect, label, font, align, 0 | flags);
		return std::static_pointer_cast<Label>(ptr);
	}

	LabelPtr Label::CreateFill(const char * id, RendererRef renderer, const char * label, FontRef font, TextAlign align, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, Rect(), label, font, align, WIN_FILL | flags);
		return std::static_pointer_cast<Label>(ptr);
	}

	LabelPtr Label::CreateAutoSize(const char* id, RendererRef renderer, const char* label, FontRef font, TextAlign align, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, Rect(), label, font, align, WIN_AUTOSIZE | flags);
		return std::static_pointer_cast<Label>(ptr);
	}

	void Label::DrawBackground(const CoreUI::RectRef &rect)
	{
		if (m_backgroundColor.IsTransparent())
			return;

		Rect fillRect = m_rect;
		fillRect.h = std::max(rect->h, m_rect.h);
		fillRect.w = std::max(rect->w, m_rect.w);

		DrawFilledRect(rect, m_backgroundColor);
	}

	Rect Label::DrawFrame(const RectRef &rect)
	{
		Rect frameRect = *rect;

		if (m_flags & WIN_FILL)
		{
			DrawBackground(&m_parent->GetClientRect(false, true));
		}

		if (m_margin)
		{
			frameRect = frameRect.Deflate(m_margin);
		}

		if (!(m_flags & WIN_FILL) && !m_backgroundColor.IsTransparent())
		{
			DrawFilledRect(&frameRect, m_backgroundColor);
		}

		if (m_showBorder)
		{
			for (int i = 0; i < m_borderWidth; ++i)
			{
				DrawRect(&frameRect, m_borderColor);
				frameRect = frameRect.Deflate(1);
			}
		}

		return std::move(frameRect);
	}

	void Label::Draw()
	{
		if (m_parent == nullptr)
			return;

		Rect drawRect;
		Rect frameRect;
		if (m_flags & WIN_FILL)
		{
			frameRect = DrawFrame(&m_parent->GetClientRect(false, false));
			drawRect = m_parent->GetClientRect(false, true);
		}
		else
		{
			drawRect = GetRect(false, true);
			frameRect = DrawFrame(&drawRect);
		}

		ClipRect clip(m_renderer, &frameRect);
		if (clip)	
		{
			DrawLabel(&drawRect);
		}
	}

	void Label::Draw(const RectRef rect, bool noClip)
	{
		DrawFrame(rect);

		Rect drawFrame = rect->Deflate(GetShrinkFactor());

		if (noClip)
		{
			DrawLabel(rect);
		}
		else
		{
			ClipRect clip(m_renderer, &drawFrame);
			if (clip)
			{
				DrawLabel(rect);
			}
		}
	}

	void Label::DrawLabel(RectRef rect)
	{
		if (m_labelText)
		{
			bool hCenter = (m_labelAlign & TEXT_H_CENTER) == TEXT_H_CENTER;
			bool vCenter = (m_labelAlign & TEXT_V_CENTER) == TEXT_V_CENTER;

			Rect target = m_labelRect.CenterInTarget(rect, hCenter, vCenter);
			Rect source = { 0, 0, target.w, target.h };
			
			int deltaX = target.x - rect->x;
			if (deltaX < 0) // Clip x
			{
				target.x = rect->x;
				target.w = rect->w;

				source.x -= deltaX;
				source.w = rect->w;
			}

			int deltaY = target.y - rect->y;
			if (deltaY < 0) // Clip y
			{
				target.y = rect->y;
				target.h = rect->h;

				source.y -= deltaY;
				source.h = rect->h;
			}

			if ((m_labelAlign & TEXT_H_CENTER) == TEXT_H_RIGHT)
			{
				target.x += rect->w - (source.w + GetShrinkFactor().w);
			}
			else if ((m_labelAlign & TEXT_H_CENTER) == TEXT_H_LEFT)
			{
				target.x += GetShrinkFactor().w;
			}

			if ((m_labelAlign & TEXT_V_CENTER) == TEXT_V_BOTTOM)
			{
				target.y += rect->h - (source.h);
			}
			else if ((m_labelAlign & TEXT_V_CENTER) == TEXT_V_TOP)
			{
				target.y += GetShrinkFactor().h;
			}

			SDL_RenderCopy(m_renderer, m_labelText.get(), &source, &target);
		}
	}

	void Label::SetText(const char * label)
	{
		Widget::SetText(label);
		RenderLabel();
	}

	void Label::RenderLabel()
	{
		size_t underlinePos = -1;
		std::string toRender = m_text;
		if (m_flags & LCF_MENUITEM)
		{
			toRender = RemoveAmpersands(underlinePos);
		}

		SDL_Surface* label;
		if (toRender.find('\n') == std::string::npos)
		{
			 label = TTF_RenderText_Blended(m_font, toRender.c_str(), m_foregroundColor);
		}
		else
		{
			label = TTF_RenderText_Blended_Wrapped(m_font, toRender.c_str(), m_foregroundColor, 0);
		}

		m_labelText = SurfaceToTexture(label);

		SDL_QueryTexture(m_labelText.get(), NULL, NULL, &m_labelRect.w, &m_labelRect.h);
		m_labelRect.x = 0;
		m_labelRect.y = 0;
		
		if ((m_flags & LCF_MENUITEM) && (underlinePos != std::string::npos))
		{
			DrawUnderline(toRender, underlinePos);
		}

		if (m_flags & WIN_AUTOSIZE)
		{
			m_rect.w = m_labelRect.w + (2 * GetShrinkFactor().w);
			m_rect.h = m_labelRect.h + (2 * GetShrinkFactor().h);
		}
	}

	void Label::DrawUnderline(const std::string & str, const size_t & underlinePos)
	{
		int xPos = 0;
		if (underlinePos > 0)
		{
			TTF_SizeText(m_font, str.substr(0, underlinePos).c_str(), &xPos, nullptr);
		}
		int width = 16;
		TTF_GlyphMetrics(m_font, str[underlinePos], nullptr, &width, nullptr, nullptr, nullptr);

		// Texture from fonts are not writable
		TexturePtr clone = CloneTexture(m_labelText);
		if (clone)
		{
			RenderTarget target(m_renderer, clone.get());
			if (target)
			{
				SetDrawColor(m_foregroundColor);
				SDL_RenderDrawLine(m_renderer, xPos, m_labelRect.h - 2, xPos + width, m_labelRect.h - 2);
				m_labelText = std::move(clone);
			}
			else
			{
				std::cerr << "Unable to set render target" << std::endl;
			}
		}
		else
		{
			std::cerr << "Unable to set clone texture" << std::endl;
		}
	}

	void Label::SetAlign(uint8_t align)
	{
		// Fill in with default if H or V alignment is not specified
		if ((align & TEXT_H_CENTER) == 0)
		{
			align |= TEXT_H_CENTER;
		}

		if ((align & TEXT_V_CENTER) == 0)
		{
			align |= TEXT_V_CENTER;
		}

		m_labelAlign = (TextAlign)align;
	}

	// TODO: Allow escaping, like "This && That"
	std::string Label::RemoveAmpersands(size_t &underlinePos)
	{
		std::string ret = m_text;
		underlinePos = ret.find_first_of("&");

		if (underlinePos != std::string::npos)
		{
			ret.erase(std::remove(ret.begin(), ret.end(), '&'), ret.end());
		}

		return ret;
	}

	struct Label::shared_enabler : public Label
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: Label(std::forward<Args>(args)...)
		{
		}
	};
}