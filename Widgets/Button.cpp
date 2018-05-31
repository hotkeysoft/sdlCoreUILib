#include "stdafx.h"
#include "SDL.h"
#include "Core/Point.h"
#include "Core/Rect.h"
#include "Core/Window.h"
#include "Core/ResourceManager.h"
#include "Image.h"
#include "Button.h"
#include "Label.h"
#include <algorithm>

namespace CoreUI
{
	Button::Button(const char * id, RendererRef renderer, Rect rect, const char * label, ImageRef image, FontRef font, CreationFlags flags) :
		Widget(id, renderer, nullptr, rect, label, image, font, flags), m_pushed(false)
	{
		m_borderWidth = 2;
	}

	void Button::Init()
	{
		if (m_text.size())
		{
			CreateLabel();
		}

		if (m_image)
		{
			UpdateButtonSize();
		}
	}

	ButtonPtr Button::Create(const char * id, RendererRef renderer, Rect rect, const char * label, ImageRef image, FontRef font, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, rect, label, image, font, flags);
		return std::static_pointer_cast<Button>(ptr);
	}

	ButtonPtr Button::CreateAutoSize(const char * id, RendererRef renderer, const char* label, ImageRef image, FontRef font, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, Rect(), label, image, font, WIN_AUTOSIZE | flags);
		return std::static_pointer_cast<Button>(ptr);
	}

	bool Button::HandleEvent(SDL_Event *e)
	{
		Point pt(e->button.x, e->button.y);
		HitResult hit = HitTest(&pt);
		const CaptureInfo & capture = WINMGR().GetCapture();
		switch (e->type)
		{
		case SDL_MOUSEBUTTONDOWN:
			if (hit.target == this)
			{
				SetActive();
				SetFocus(this);
				m_pushed = true;
				WINMGR().StartCapture(hit, &pt);
				return true;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			m_pushed = false;
			if (capture && capture.Target.target == this)
			{
				WINMGR().ReleaseCapture();
				if (hit.target == this)
				{
					PostEvent(EVENT_BUTTON_CLICKED);
				}
			}
			return true;
		case SDL_MOUSEMOTION:
			if (hit)
			{
				SDL_SetCursor(RES().FindCursor("default"));
			}

			if (capture && capture.Target.target == this)
			{
				m_pushed = (hit.target == this);
			}
			break;
		case SDL_KEYDOWN:
			if (IsFocused())
			{
				switch (e->key.keysym.sym)
				{
				case SDLK_SPACE:
				case SDLK_RETURN:				
					m_pushed = true;
					WINMGR().StartCapture(HitResult(HIT_CLIENT, this), &pt);
					return true;
				}
			}
			break;
		case SDL_KEYUP:
			m_pushed = false;
			if (capture && capture.Target.target == this)
			{
				WINMGR().ReleaseCapture();
				PostEvent(EVENT_BUTTON_CLICKED);
			}
			return true;
		}
		return false;
	}

	HitResult Button::HitTest(const PointRef pt)
	{
		Rect parent = m_parent->GetClientRect(false, true);
		if (m_rect.Offset(&parent.Origin()).PointInRect(pt))
		{
			return HitResult(HitZone::HIT_CONTROL, this);
		}
		
		return HitZone::HIT_NOTHING;
	}

	Rect Button::GetClientRect(bool relative, bool scrolled) const
	{
		return GetRect(relative, scrolled).Deflate(m_borderWidth);
	}

	void Button::Draw()
	{
		if (m_parent == nullptr)
			return;

		Rect drawRect = GetRect(false, true);

		DrawButton(&drawRect, m_backgroundColor, nullptr, !m_pushed, m_borderWidth);

		if (m_label)
		{
			m_label->SetBorder(IsFocused());
			m_label->Draw(&drawRect.Deflate(m_borderWidth));
		}
	}

	void Button::Draw(RectRef rect)
	{
		DrawButton(rect, m_backgroundColor, nullptr, !m_pushed, m_borderWidth);
		Rect drawRect = rect->Deflate(m_borderWidth);

		if (m_image)
		{
			int imageWidth = m_image->GetRect(true, false).w ;
			m_image->Draw(&drawRect, Image::IMG_H_LEFT | Image::IMG_V_CENTER);
			drawRect.x += imageWidth;
			drawRect.w -= imageWidth;
		}

		if (m_label)
		{	
			Rect target = m_label->GetRect(true, false).CenterInTarget(&drawRect, false, true);

			m_label->SetBorder(IsFocused());
			m_label->Draw(&target);
		}
	}

	void Button::SetText(const char * label)
	{
		Widget::SetText(label);
		CreateLabel();		
	}

	void Button::CreateLabel()
	{
		if (m_flags & WIN_AUTOSIZE)
		{
			m_label = Label::CreateAutoSize("label", m_renderer, m_text.c_str(), m_font);
		}
		else
		{
			m_label = Label::CreateFill("label", m_renderer, m_text.c_str(), m_font);
		}

		m_label->SetMargin(Dimension(5,2));
		m_label->SetPadding(0);
		m_label->SetBorderColor(Color::C_MED_GREY);
		m_label->SetBorderWidth(1);
		m_label->SetBorder(true);
		m_label->SetParent(this);
		m_label->Init();
		UpdateButtonSize();
	}

	void Button::UpdateButtonSize()
	{
		if (!(m_flags & WIN_AUTOSIZE))
			return;

		bool hasImage = (m_image != nullptr);
		bool hasText = (!m_text.empty());

		int width = 0;
		int height = 0;
		if (hasImage)
		{
			Rect image = m_image->GetRect(false, false);
			width = image.w;
			height = image.h;
		}

		if (hasText)
		{
			Rect labelRect = m_label->GetRect(true, false);
			width += labelRect.w;
			height = std::max(height, labelRect.h);
		}

		m_rect = Rect(0, 0, width + (2*m_borderWidth), height + (2*m_borderWidth));
	}
	struct Button::shared_enabler : public Button
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: Button(std::forward<Args>(args)...)
		{
		}
	};
}