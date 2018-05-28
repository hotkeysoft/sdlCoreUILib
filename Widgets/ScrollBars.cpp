#include "stdafx.h"
#include "SDL.h"
#include "Core\Point.h"
#include "Core\Rect.h"
#include "Core\Window.h"
#include "Core\ResourceManager.h"
#include "Image.h"
#include "Menu.h"
#include "ScrollBars.h"
#include <algorithm>

namespace CoreUI
{
	ScrollBars::ScrollBars(RendererRef renderer, WindowRef parent) :
		Widget("scroll", renderer, nullptr, Rect(), nullptr), m_parent(parent)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}
	}

	struct ScrollBars::shared_enabler : public ScrollBars
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: ScrollBars(std::forward<Args>(args)...)
		{
		}
	};

	ScrollBarsPtr ScrollBars::Create(RendererRef renderer, WindowRef parent)
	{
		return std::make_unique<shared_enabler>(renderer, parent);
	}

	void ScrollBars::DrawHScrollBar(RectRef pos)
	{		
		static ImageRef leftButton = RES().FindImage("coreUI.widget15x15", 2);
		static ImageRef rightButton = RES().FindImage("coreUI.widget15x15", 3);

		DrawButton(pos, Color::C_MED_GREY, nullptr, false);

		int scrollAreaWidth = pos->w - (2 * m_scrollBarSize);
		m_scrollState.leftButton = { pos->x, pos->y, m_scrollBarSize, m_scrollBarSize };
		m_scrollState.rightButton = { pos->x + pos->w - m_scrollBarSize, pos->y, m_scrollBarSize, m_scrollBarSize };
		m_scrollState.hScrollArea = { pos->x + m_scrollBarSize, pos->y, scrollAreaWidth, m_scrollBarSize };

		DrawButton(&m_scrollState.leftButton, Color::C_LIGHT_GREY, leftButton, !m_parent->GetPushedState(HIT_HSCROLL_LEFT));
		DrawButton(&m_scrollState.rightButton, Color::C_LIGHT_GREY, rightButton, !m_parent->GetPushedState(HIT_HSCROLL_RIGHT));

		int fullWidth = m_scrollState.hMax + pos->w;
		int sliderWidth = pos->w * scrollAreaWidth / fullWidth;
		if (sliderWidth < (m_borderWidth * 2))
		{
			sliderWidth = m_borderWidth * 2;
		}
		
		int currPos = m_parent->m_scrollPos.x * scrollAreaWidth / fullWidth;
		if (currPos + sliderWidth > scrollAreaWidth)
		{
			currPos = scrollAreaWidth - (int)sliderWidth + 1;
		}

		m_scrollState.hSlider = { pos->x + m_scrollBarSize + currPos, pos->y, (int)sliderWidth, m_scrollBarSize };

		DrawButton(&m_scrollState.hSlider, Color::C_LIGHT_GREY, nullptr, !m_parent->GetPushedState(HIT_HSCROLL_SLIDER));
	}

	void ScrollBars::DrawVScrollBar(RectRef pos)
	{
		static ImageRef upButton = RES().FindImage("coreUI.widget15x15", 0);
		static ImageRef downButton = RES().FindImage("coreUI.widget15x15", 1);

		DrawButton(pos, Color::C_MED_GREY, nullptr, false);

		int scrollAreaHeight = pos->h - (2 * m_scrollBarSize);
		m_scrollState.upButton = { pos->x, pos->y, m_scrollBarSize, m_scrollBarSize };
		m_scrollState.downButton = { pos->x, pos->y + pos->h - m_scrollBarSize, m_scrollBarSize, m_scrollBarSize };
		m_scrollState.vScrollArea = { pos->x, pos->y + m_scrollBarSize, m_scrollBarSize, scrollAreaHeight };

		DrawButton(&m_scrollState.upButton, Color::C_LIGHT_GREY, upButton, !m_parent->GetPushedState(HIT_VSCROLL_UP));
		DrawButton(&m_scrollState.downButton, Color::C_LIGHT_GREY, downButton, !m_parent->GetPushedState(HIT_VSCROLL_DOWN));
		
		int fullHeight = m_scrollState.vMax + pos->h;
		int sliderHeight = pos->h * scrollAreaHeight / fullHeight;
		if (sliderHeight < (m_borderWidth * 2))
		{
			sliderHeight = m_borderWidth * 2;
		}

		int currPos = m_parent->m_scrollPos.y * scrollAreaHeight / fullHeight;
		if (currPos + sliderHeight > scrollAreaHeight)
		{
			currPos = scrollAreaHeight - (int)sliderHeight + 1;
		}

		m_scrollState.vSlider = { pos->x, pos->y + m_scrollBarSize + currPos, m_scrollBarSize, (int)sliderHeight };

		DrawButton(&m_scrollState.vSlider, Color::C_LIGHT_GREY, nullptr, !m_parent->GetPushedState(HIT_VSCROLL_SLIDER));
	}

	void ScrollBars::CheckChildScrollStatus(WidgetRef child, RectRef thisRect, bool &showH, bool &showV)
	{
		Rect parentRect = child->GetParent()->GetClientRect(true);

		if (((thisRect->x + thisRect->w) > parentRect.w))
		{
			showH = true;
			int hMax = ((thisRect->x + thisRect->w) - parentRect.w);
			m_scrollState.hMax = std::max(m_scrollState.hMax, hMax);
		}

		if ((thisRect->y + thisRect->h) > parentRect.h)
		{
			showV = true;		
			int vMax = ((thisRect->y + thisRect->h) - parentRect.h);
			m_scrollState.vMax = std::max(m_scrollState.vMax, vMax);
		}
	}

	void ScrollBars::RefreshScrollBarStatus()
	{
		m_scrollState.hMax = 0;
		m_scrollState.vMax = 0;

		bool showH = false;
		bool showV = false;
		for (auto & child : m_parent->GetChildWindows())
		{
			if (child->GetParent() != nullptr && !(child->GetShowState() & (WindowState::WS_MAXIMIZED | WindowState::WS_MINIMIZED)))
			{
				Rect childRect = child->GetRect(true, false);
				CheckChildScrollStatus(child.get(), &childRect, showH, showV);
			}
		}
		for (auto & child : m_parent->GetControls())
		{
			CheckChildScrollStatus(child.second.get(), &child.second->GetRect(true, false), showH, showV);
		}

		m_scrollState.showH = showH || m_parent->m_scrollPos.x;
		m_scrollState.showV = showV || m_parent->m_scrollPos.y;
	}

	void ScrollBars::Draw(RectRef pos)
	{
		if (m_parent->GetFlags() & WindowFlags::WIN_NOSCROLL)
			return;

		RefreshScrollBarStatus();

		if (m_scrollState.showH)
		{
			Rect hPos = *pos;
			hPos.y += hPos.h - m_scrollBarSize;
			hPos.w -= (m_scrollState.showV ? m_scrollBarSize : 0);
			hPos.h = m_scrollBarSize;

			DrawHScrollBar(&hPos);
		}

		if (m_scrollState.showV)
		{
			Rect vPos = *pos;
			vPos.x += vPos.w - m_scrollBarSize;
			vPos.w = m_scrollBarSize;
			vPos.h -= (m_scrollState.showH ? m_scrollBarSize : 0);

			DrawVScrollBar(&vPos);
		}

		// Corner area if both bars are drawn
		if (m_scrollState.showH && m_scrollState.showV)
		{
			Rect hvPos = *pos;
			hvPos.x += hvPos.w - m_scrollBarSize;
			hvPos.y += hvPos.h - m_scrollBarSize;
			hvPos.w = m_scrollBarSize;
			hvPos.h = m_scrollBarSize;

			Draw3dFrame(&hvPos, true);
		}
	}

	HitResult ScrollBars::HitTest(const PointRef pt)
	{
		if (m_scrollState.showH)
		{
			if (m_scrollState.leftButton.PointInRect(pt))
			{
				return HitResult(HIT_HSCROLL_LEFT, this);
			}
			if (m_scrollState.rightButton.PointInRect(pt))
			{
				return HitResult(HIT_HSCROLL_RIGHT, this);
			}
			if (m_scrollState.hSlider.PointInRect(pt))
			{
				return HitResult(HIT_HSCROLL_SLIDER, this);
			}
			if (m_scrollState.hScrollArea.PointInRect(pt))
			{
				return HitResult(HIT_HSCROLL_AREA, this);
			}
		}
		if (m_scrollState.showV)
		{
			if (m_scrollState.upButton.PointInRect(pt))
			{
				return HitResult(HIT_VSCROLL_UP, this);
			}
			if (m_scrollState.downButton.PointInRect(pt))
			{
				return HitResult(HIT_VSCROLL_DOWN, this);
			}
			if (m_scrollState.vSlider.PointInRect(pt))
			{
				return HitResult(HIT_VSCROLL_SLIDER, this);
			}
			if (m_scrollState.vScrollArea.PointInRect(pt))
			{
				return HitResult(HIT_VSCROLL_AREA, this);
			}
		}
		return HitResult(HIT_NOTHING, this);
	}

	void ScrollBars::ScrollRel(PointRef pt)
	{ 
		if (m_parent->GetFlags() & WindowFlags::WIN_NOSCROLL)
			return;

		if (m_scrollState.showH)
		{
			m_parent->m_scrollPos.x = clip(m_parent->m_scrollPos.x + pt->x, 0, m_scrollState.hMax);
		}

		if (m_scrollState.showV)
		{
			m_parent->m_scrollPos.y = clip(m_parent->m_scrollPos.y + pt->y, 0, m_scrollState.vMax);
		}
	}

	void ScrollBars::ScrollTo(PointRef pt)
	{
		if (m_parent->GetFlags() & WindowFlags::WIN_NOSCROLL)
			return;

		m_parent->m_scrollPos.x = clip(pt->x, 0, m_scrollState.hMax);
		m_parent->m_scrollPos.y = clip(pt->y, 0, m_scrollState.vMax);
	}

	void ScrollBars::ClickHScrollBar(PointRef pt)
	{
		int rel = (pt->x - m_scrollState.hScrollArea.x) * m_scrollState.hMax / m_scrollState.hScrollArea.w;
		m_parent->m_scrollPos.x = clip(rel, 0, m_scrollState.hMax);
	}

	void ScrollBars::ClickVScrollBar(PointRef pt)
	{
		int rel = (pt->y - m_scrollState.vScrollArea.y) * m_scrollState.vMax / m_scrollState.vScrollArea.h;
		m_parent->m_scrollPos.y = clip(rel, 0, m_scrollState.vMax);
	}

	bool ScrollBars::HandleEvent(SDL_Event * e)
	{
		Point pt(e->button.x, e->button.y);
		HitResult hit = HitTest(&pt);

		if (e->type == SDL_MOUSEBUTTONDOWN)
		{
			if (e->button.button == SDL_BUTTON_LEFT)
			{
				WINMGR().SetActive(m_parent);
				bool capture = false;
				switch (HitZone(hit))
				{
				case HIT_HSCROLL_LEFT:
				case HIT_HSCROLL_RIGHT:
				case HIT_VSCROLL_UP:
				case HIT_VSCROLL_DOWN:
				case HIT_HSCROLL_SLIDER:
				case HIT_VSCROLL_SLIDER:
					m_parent->ToggleButtonState(hit, true);
					capture = true;
					break;
				case HIT_HSCROLL_AREA:
					ClickHScrollBar(&pt);
					capture = true;
					break;
				case HIT_VSCROLL_AREA:
					ClickVScrollBar(&pt);
					capture = true;
					break;
				case HIT_TITLEBAR:
					capture = true;
					break;
				}

				if (capture)
				{
					WINMGR().StartCapture(hit, &pt);
					return true;
				}
			}
		}
		else if (e->type == SDL_MOUSEBUTTONUP)
		{
			const CaptureInfo & capture = WINMGR().GetCapture();
			if (capture && capture.Target.target == this)
			{
				m_parent->ToggleButtonState(capture.Target, false);
				switch (HitZone(hit))
				{
				case HIT_HSCROLL_LEFT:
				case HIT_HSCROLL_RIGHT:
				case HIT_VSCROLL_UP:
				case HIT_VSCROLL_DOWN:
				case HIT_HSCROLL_SLIDER:
				case HIT_VSCROLL_SLIDER:
					if (hit == capture.Target)
					{
						m_parent->ButtonPushed(capture.Target);
					}
				}
				WINMGR().ReleaseCapture();
				return true;
			}
		}
		else if (e->type == SDL_MOUSEMOTION)
		{
			if (WINMGR().GetCapture())
			{
				const CaptureInfo & capture = WINMGR().GetCapture();
				Point newPos = pt;
				newPos.x += capture.Delta.x;
				newPos.y += capture.Delta.y;
				Point delta = { (capture.Origin.x - newPos.x) , (capture.Origin.y - newPos.y) };

				bool handled = true;
				switch ((HitZone)capture.Target)
				{
				case HIT_HSCROLL_SLIDER:
					ClickHScrollBar(&pt);
					break;
				case HIT_VSCROLL_SLIDER:
					ClickVScrollBar(&pt);
					break;
				case HIT_HSCROLL_LEFT:
				case HIT_HSCROLL_RIGHT:
				case HIT_VSCROLL_UP:
				case HIT_VSCROLL_DOWN:
					m_parent->ToggleButtonState(capture.Target, capture.Target.target->HitTest(&pt) == capture.Target);
					break;
				default:
					handled = false;
				}
				return handled;
			}
			else
			{
				if ((HitZone)hit & (HIT_BUTTON_ANY | HIT_VSCROLL_ANY | HIT_HSCROLL_ANY))
				{
					SDL_SetCursor(RES().FindCursor("default"));
					return true;
				}
			}
		}

		return false;
	}
}