#include "stdafx.h"
#include "SDL.h"
#include "Point.h"
#include "Rect.h"
#include "Window.h"
#include "Util/ClipRect.h"
#include "Widgets/Image.h"
#include "Widgets/Menu.h"
#include "Widgets/Toolbar.h"
#include "ResourceManager.h"
#include <algorithm>

namespace CoreUI
{
	const Color Window::m_activeTitleBarColor = Color(90, 128, 128);

	Window Window::m_nullWnd;

	Window::Window() : Widget("null"), m_showState(), m_pushedState(HIT_NOTHING)
	{
	}

	Window::Window(const char* id, RendererRef renderer, WindowRef parent, FontRef font, Rect rect, CreationFlags flags) :
		Widget(id, renderer, parent, rect, nullptr, nullptr, font, flags),
		m_showState(WST_VISIBLE), m_pushedState(HIT_NOTHING)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}
		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		if (m_flags & WindowFlags::WIN_MINMAX && m_parent == nullptr)
		{
			throw std::invalid_argument("WIN_MINMAX flag needs parent window");
		}

		m_scrollBars = ScrollBars::Create(renderer, this);

		m_backgroundColor = Color::C_LIGHT_GREY;

		m_minSize = 120;
	}

	WindowPtr Window::Create(const char* id, RendererRef renderer, WindowRef parent, FontRef font, Rect rect, CreationFlags flags)
	{
		auto ptr = std::make_shared<shared_enabler>(id, renderer, parent, font, rect, flags);
		return std::static_pointer_cast<Window>(ptr);
	}

	void Window::SetActive()
	{
		WINMGR().SetActive(this);
	}

	void Window::SetFocus(WidgetRef focus, WidgetRef parent)
	{
		// Remove focus from other controls
		for (auto & control : m_controls)
		{
			if (control.second.get() != parent)
			{
				control.second->ClearFocus();
			}
		}
	}
	
	void Window::SetText(const char * title)
	{
		Widget::SetText(title);
		RenderTitle();
	}

	void Window::Show(bool show)
	{
		m_showState = show ? WindowState(m_showState | WST_VISIBLE) : WindowState(m_showState & ~WST_VISIBLE);
	}

	void Window::DrawCloseButton(Rect pos, const CoreUI::Color & col)
	{
		static ImageRef restoreButton = RES().FindImage("coreUI.widget24x24", 3);

		DrawButton(&GetCloseButtonRect(pos), col, restoreButton, !(m_pushedState & HIT_CLOSEBUTTON));
	}

	void Window::DrawMinMaxButtons(Rect pos, const CoreUI::Color & col)
	{
		static ImageRef minimizeButton = RES().FindImage("coreUI.widget24x24", 0);
		static ImageRef maximizeButton = RES().FindImage("coreUI.widget24x24", 1);
		static ImageRef restoreButton = RES().FindImage("coreUI.widget24x24", 2);

		DrawButton(&GetMinimizeButtonRect(pos), col, (m_showState & WST_MINIMIZED) ? restoreButton : minimizeButton, !(m_pushedState & HIT_MINBUTTON));
		DrawButton(&GetMaximizeButtonRect(pos), col, (m_showState & WST_MAXIMIZED) ? restoreButton : maximizeButton, !(m_pushedState & HIT_MAXBUTTON));
	}

	WindowManager::WindowList Window::GetChildWindows()
	{
		return WINMGR().GetWindowList(this);
	}

	void Window::AddControl(WidgetPtr widget)
	{
		if (widget == nullptr)
		{
			throw std::invalid_argument("widget is null");
		}

		if (FindControl(widget->GetId().c_str()) != nullptr)
		{
			throw std::invalid_argument("control already exists:" + widget->GetId());
		}
		widget->SetParent(this);
		widget->Init();

		m_controls[widget->GetId()] = widget;
	}

	WidgetPtr Window::FindControl(const char * id) const
	{
		if (id == nullptr)
			return nullptr;

		auto ctrl = m_controls.find(id);
		if (ctrl != m_controls.end())
		{
			return ctrl->second;
		}

		return nullptr;
	}

	Rect Window::GetRect(bool relative, bool scrolled) const
	{
		Rect rect = m_rect;

		if ((m_parent == nullptr) && (m_flags & WIN_FILL))
		{
			rect = WINMGR().GetWindowSize();
		}

		if (m_showState & WST_MAXIMIZED)
		{
			rect = GetParent()->GetClientRect(true, false);
		}
		else if (m_showState & WST_MINIMIZED)
		{
			int minimizedHeight = (m_buttonSize + 2) + (2 * m_borderWidth) + 2;
			rect = GetParent()->GetClientRect(true, false);
			rect.w = 200;
			rect.x = GetParentWnd()->GetMinimizedChildIndex(const_cast<WindowRef>(this)) * 200;
			rect.y = (rect.h - minimizedHeight);
			rect.h = minimizedHeight;
		}

		if (m_parent != nullptr)
		{
			if (!relative)
			{
				Rect parentClient = GetParent()->GetClientRect(false, false);
				rect.x += parentClient.x;
				rect.y += parentClient.y;
			}

			if (scrolled)
			{
				PointRef scrollOffset = GetParentWnd()->GetScrollPos();
				rect.x -= scrollOffset->x;
				rect.y -= scrollOffset->y;
			}
		}

		return rect;
	}

	Rect Window::GetRawClientRect(bool relative, bool scrolled) const // Excludes scroll bars and menu
	{
		Rect rect = GetRect(relative, true);

		uint8_t titleBarHeight = (m_flags & WIN_BORDERLESS) ? 0 : (m_buttonSize + 2);

		if (relative)
		{
			rect.x = 0;
			rect.y = 0;
		}
		else
		{
			rect.x += m_borderWidth;
			rect.y += m_borderWidth + titleBarHeight;
		}

		rect.w -= (2 * m_borderWidth);
		rect.h -= (2 * m_borderWidth) + titleBarHeight;

		if (scrolled)
		{
			rect.x -= m_scrollPos.x;
			rect.y -= m_scrollPos.y;
		}

		return rect;
	}

	Rect Window::GetClientRect(bool relative, bool scrolled) const
	{
		uint8_t titleBarHeight = (m_flags & WIN_BORDERLESS) ? 0 : (m_buttonSize + 2);

		Rect rect = GetRect(relative, true);
		if (relative)
		{
			rect.x = 0;
			rect.y = 0;
		}
		else
		{
			rect.x += m_borderWidth;
			rect.y += m_borderWidth + titleBarHeight;
		}

		rect.w -= (2 * m_borderWidth);
		rect.h -= (2 * m_borderWidth) + titleBarHeight;

		if (m_menu)
		{
			int height = m_menu->GetHeight(rect.w);
			rect.y += relative ? 0 : height;
			rect.h -= height;
		}

		if (m_toolbar)
		{
			int height = m_toolbar->GetHeight(rect.w);
			rect.y += relative ? 0 : height;
			rect.h -= height;
		}

		ScrollStateRef scroll = m_scrollBars->GetScrollState();
		auto size = m_scrollBars->GetSize();

		rect.w -= (scroll->showV ? size : 0);
		rect.h -= (scroll->showH ? size : 0);

		if (scrolled)
		{
			rect.x -= m_scrollPos.x;
			rect.y -= m_scrollPos.y;
		}
		
		return rect;
	}

	Rect Window::GetTitleBarRect(Rect rect) const
	{
		int closeButtonW = (m_flags & WindowFlags::WIN_CLOSE) ? m_buttonSize + 2 : 0;
		int minMaxButtonW = (m_flags & WindowFlags::WIN_MINMAX) ? (m_buttonSize + 2) * 2 : 0;

		rect.x += m_borderWidth;
		rect.y += m_borderWidth;
		rect.w -= (2 * m_borderWidth) + minMaxButtonW + closeButtonW;
		rect.h = m_buttonSize + 2;
		return rect;
	}

	Rect Window::GetCloseButtonRect(Rect rect) const
	{
		rect.x += rect.w - (m_borderWidth + (m_buttonSize + 2));
		rect.y += m_borderWidth;
		rect.w = m_buttonSize + 2;
		rect.h = m_buttonSize + 2;

		return rect;
	}

	Rect Window::GetMinimizeButtonRect(Rect rect) const
	{
		int closeOffset = (m_flags & WIN_CLOSE) ? (m_buttonSize + 2): 0;
		rect.x += rect.w - (m_borderWidth + closeOffset + (m_buttonSize + 2) * 2);
		rect.y += m_borderWidth;

		rect.w = m_buttonSize + 2;
		rect.h = m_buttonSize + 2;

		return rect;
	}

	Rect Window::GetMaximizeButtonRect(Rect rect) const
	{
		int closeOffset = (m_flags & WIN_CLOSE) ? (m_buttonSize + 2) : 0;
		rect.x += rect.w - (m_borderWidth + closeOffset + (m_buttonSize + 2));
		rect.y += m_borderWidth;

		rect.w = m_buttonSize + 2;
		rect.h = m_buttonSize + 2;

		return rect;
	}

	void Window::DrawTitleBar(Rect rect, bool active)
	{	
		auto titleBar = GetTitleBarRect(rect);
		if (active)
		{
			SetDrawColor(m_activeTitleBarColor);
			SDL_RenderFillRect(m_renderer, &titleBar);
		}
		else
		{
			SetDrawColor(Color::C_LIGHT_GREY);
			SDL_RenderFillRect(m_renderer, &titleBar);
			Draw3dFrame(&titleBar, true);
		}
	}

	void Window::DrawTitle(Rect rect, bool active)
	{
		TexturePtr title;
		if (active && m_activeTitle)
		{
			title = m_activeTitle;
		}
		else if (!active && m_inactiveTitle)
		{
			title = m_inactiveTitle;
		}

		if (title)
		{
			Rect target = GetTitleBarRect(rect);

			target.x += m_buttonSize / 4;
			target.y += (target.h - m_titleStrRect.h) / 2;
			target.y -= 2; // Fudge factor, fonts seem rendered lower in SDL2_ttf-2.0.18
			target.w = std::min(m_titleStrRect.w, target.w - (m_buttonSize / 2));
			target.h = std::min(m_titleStrRect.h, target.h);

			Rect source = { 0, 0, target.w, target.h };
			SDL_RenderCopy(m_renderer, title.get(), &source, &target);
		}
	}

	HitResult Window::HitTest(const PointRef pt)
	{
		if (!(m_showState & WST_VISIBLE))
		{
			return HitZone::HIT_NOTHING;
		}

		Rect wndRect = GetRect(false);
		Rect intersect = wndRect;
		if (m_parent != nullptr)
		{
			intersect = wndRect.IntersectRect(&GetClipRect(GetParentWnd()));
		}

		if (!intersect.PointInRect(pt))
		{
			return HitZone::HIT_NOTHING;
		}

		if (GetTitleBarRect(wndRect).PointInRect(pt))
		{
			return HitResult(HitZone::HIT_TITLEBAR, this);
		}
		else if (GetClientRect(false, false).PointInRect(pt))
		{
			for (auto & child : m_controls)
			{
				HitResult childHit = child.second->HitTest(pt);
				if (childHit)
				{
					return childHit;
				}
			}
			return HitResult(HitZone::HIT_CLIENT, this);
		}

		// Title bar buttons
		if (GetCloseButtonRect(wndRect).PointInRect(pt))
		{
			return HitResult(HitZone::HIT_CLOSEBUTTON, this);
		}
		if (GetMinimizeButtonRect(wndRect).PointInRect(pt))
		{
			return HitResult(HitZone::HIT_MINBUTTON, this);
		}
		if (GetMaximizeButtonRect(wndRect).PointInRect(pt))
		{
			return HitResult(HitZone::HIT_MAXBUTTON, this);
		}

		if (m_menu)
		{
			HitResult menuHit = m_menu->HitTest(pt);
			if (menuHit)
			{
				return menuHit;
			}
		}

		if (m_toolbar)
		{
			HitResult toolbarHit = m_toolbar->HitTest(pt);
			if (toolbarHit)
			{
				return toolbarHit;
			}
		}

		HitResult scrollHit = m_scrollBars->HitTest(pt);
		if (scrollHit)
		{
			return scrollHit;
		}

		// Resize handles
		if (m_showState & (WST_MAXIMIZED | WST_MINIMIZED) ||
			!(m_flags & WindowFlags::WIN_CANRESIZE))
		{
			return HitResult(HitZone::HIT_NOTHING, this);
		}

		bool left = pt->x < wndRect.x + 2*m_borderWidth;
		bool top = pt->y < wndRect.y + 2*m_borderWidth;
		bool right = pt->x > wndRect.x + wndRect.w - 2*m_borderWidth;
		bool bottom = pt->y > wndRect.y + wndRect.h - 2*m_borderWidth;

		if (top)
		{
			if (left) return HitResult(HitZone::HIT_CORNER_TOPLEFT, this);
			else if (right) return HitResult(HitZone::HIT_CORNER_TOPRIGHT, this);
			else return HitResult(HitZone::HIT_BORDER_TOP, this);
		}
		else if (bottom)
		{
			if (left) return HitResult(HitZone::HIT_CORNER_BOTTOMLEFT, this);
			else if (right) return HitResult(HitZone::HIT_CORNER_BOTTOMRIGHT, this);
			else return HitResult(HitZone::HIT_BORDER_BOTTOM, this);
		}
		else if (left)
		{
			return HitResult(HitZone::HIT_BORDER_LEFT, this);
		}
		else if (right) 
		{
			return HitResult(HitZone::HIT_BORDER_RIGHT, this);
		}

		return HIT_NOTHING;
	}

	Rect Window::GetClipRect(WindowRef win)
	{
		Rect rect = win->GetClientRect(false, false);

		while (win)
		{
			rect = rect.IntersectRect(&win->GetClientRect(false, false));
			win = win->GetParentWnd();
		}

		return rect;
	}

	void Window::Draw()
	{
		if (!(m_showState & WST_VISIBLE))
			return;

		ClipRect clip(m_renderer, m_parent ? &GetClipRect(GetParentWnd()) : nullptr, m_parent);
		if (clip || m_parent == nullptr)
		{
			Rect rect = GetRect(false);

			bool active = (WINMGR().GetActive() == this || (m_flags & WIN_ACTIVE));

			if (m_flags & WIN_BORDERLESS)
			{
				SetBorderWidth(0);
			}
			else
			{
				if (m_flags & WIN_DIALOG)
				{
					DrawFilledRect(&rect, Color::C_LIGHT_GREY);

					Draw3dFrame(&rect, true, Color::C_DARK_GREY);
					Draw3dFrame(&rect.Deflate(1), true, Color::C_MED_GREY);
				}
				else
				{
					DrawReliefBox(&rect, Color::C_LIGHT_GREY, false);
				}

				DrawTitleBar(rect, active);
				DrawTitle(rect, active);

				if (m_flags & WindowFlags::WIN_CLOSE)
				{
					DrawCloseButton(rect, Color::C_LIGHT_GREY);
				}
				if (m_flags & WindowFlags::WIN_MINMAX)
				{
					DrawMinMaxButtons(rect, Color::C_LIGHT_GREY);
				}
			}

			if (!(m_showState & WST_MINIMIZED))
			{
				Rect clientRect = GetRawClientRect(false, false);
				if (m_menu)
				{
					// Active menu needs to be drawn on top of everything, let Window Manager handle it
					if (!active)
					{
						DrawMenu();
					}
					int menuHeight = m_menu->GetHeight(clientRect.w);
					clientRect.y += menuHeight;
					clientRect.h -= menuHeight;
				}

				if (m_toolbar)
				{
					DrawToolbar();

					int toolbarHeight = m_toolbar->GetHeight(clientRect.w);
					clientRect.y += toolbarHeight;
					clientRect.h -= toolbarHeight;
				}

				if (!m_backgroundColor.IsTransparent() && !(m_flags & WIN_DIALOG))
				{
					DrawFilledRect(&clientRect, m_backgroundColor);
				}
				m_scrollBars->Draw(&clientRect);

				DrawGrid();

				DrawControls();
			}
		}
	}
	
	void Window::DrawGrid()
	{
		int gridSize = m_grid.GetSize();
		if (m_grid.IsVisible() && gridSize > 1)
		{
			SetDrawColor(Color::C_BLACK);
			Rect clientRect = GetClientRect(false, true);
			for (int x = clientRect.x; x < clientRect.x + clientRect.w; x += gridSize)
			{
				for (int y = clientRect.y; y < clientRect.y + clientRect.h; y += gridSize)
				{
					SDL_RenderDrawPoint(m_renderer, x, y);
				}
			}
		}
	}


	void Window::DrawMenu()
	{
		if (m_menu)
		{
			Rect clientRect = GetRawClientRect(false, false);
			m_menu->Draw(&clientRect);
		}
	}

	void Window::DrawToolbar()
	{
		Rect clientRect = GetRawClientRect(false, false);
		if (m_menu)
		{
			int menuHeight = m_menu->GetHeight(clientRect.w);
			clientRect.y += menuHeight;
			clientRect.h -= menuHeight;
		}

		m_toolbar->Draw(&clientRect);
	}

	void Window::DrawControls()
	{
		ClipRect clip(m_renderer, &GetClientRect(false, false));
		if (clip)
		{
			for (auto & ctrl : m_controls)
			{
				ctrl.second->Draw();
			}
		}
	}

	void Window::RenderTitle()
	{
		FontRef titleFont = RES().FindFont("coreUI.defaultBold");
		if (titleFont == nullptr)
		{
			std::cerr << "win.title font not found, using default" << std::endl;
			titleFont = m_font;
		}
		if (titleFont == nullptr)
		{
			throw std::invalid_argument("no font");
		}

		SDL_Surface* active = TTF_RenderText_Blended(titleFont, m_text.c_str(), CoreUI::Color::C_WHITE);
		m_activeTitle = SurfaceToTexture(active);

		SDL_Surface* inactive = TTF_RenderText_Blended(titleFont, m_text.c_str(), CoreUI::Color::C_DARK_GREY);
		m_inactiveTitle = SurfaceToTexture(inactive);

		SDL_QueryTexture(m_activeTitle.get(), NULL, NULL, &m_titleStrRect.w, &m_titleStrRect.h);
		m_titleStrRect.x = 0;
		m_titleStrRect.y = 0;
	}

	void Window::ToggleButtonState(HitZone button, bool pushed)
	{
		if (pushed)
		{
			m_pushedState = HitZone(m_pushedState | button);
		}
		else
		{
			m_pushedState = HitZone(m_pushedState & ~button);
		}
	}

	void Window::ButtonPushed(HitZone button)
	{
		const ScrollStateRef scroll = m_scrollBars->GetScrollState();

		//TODO: Improve this
		int hScroll = scroll->hMax / 10;
		int vScroll = scroll->vMax / 10;

		switch (button)
		{
		case HIT_MAXBUTTON: Maximize(); break;
		case HIT_MINBUTTON: Minimize(); break;
		case HIT_CLOSEBUTTON: Close();
		case HIT_HSCROLL_LEFT: m_scrollBars->ScrollRel(&Point({ -hScroll, 0 })); break;
		case HIT_HSCROLL_RIGHT: m_scrollBars->ScrollRel(&Point({ hScroll, 0 })); break;
		case HIT_VSCROLL_UP: m_scrollBars->ScrollRel(&Point({ 0, -vScroll })); break;
		case HIT_VSCROLL_DOWN: m_scrollBars->ScrollRel(&Point({ 0, vScroll })); break;
		default: break;
		}
	}

	bool Window::MovePos(PointRef pos)
	{
		if ((m_flags & WindowFlags::WIN_CANMOVE) &&
			!(m_showState & WST_MAXIMIZED))
		{
			return Widget::MovePos(pos);
		}

		return false;
	}

	bool Window::MoveRel(PointRef rel)
	{
		bool clip = false;
		if ((m_flags & WindowFlags::WIN_CANMOVE) &&
			!(m_showState & WST_MAXIMIZED))
		{
			return Widget::MoveRel(rel);
		}		

		return false;
	}

	bool Window::ResizeRel(PointRef rel)
	{
		if (m_flags & WindowFlags::WIN_CANRESIZE &&
			!(m_showState & WST_MAXIMIZED))
		{
			return Widget::ResizeRel(rel);
		}

		return false;
	}

	bool Window::Resize(PointRef size)
	{
		bool clip = false;
		if (m_flags & WindowFlags::WIN_CANRESIZE &&
			!(m_showState & WST_MAXIMIZED))
		{
			return Widget::Resize(size);
		}

		return false;
	}

	bool Window::MoveRect(RectRef rect)
	{
		if (m_flags & WindowFlags::WIN_CANRESIZE &&
			!(m_showState & WST_MAXIMIZED))
		{
			return Widget::MoveRect(rect);
		}

		return false;
	}

	void Window::Minimize()
	{
		if (!(m_flags & WindowFlags::WIN_MINMAX))
		{
			return;
		}

		if (m_showState & WST_MINIMIZED)
		{
			Restore();
			GetParentWnd()->SetMinimizedChild(this, false);
		}
		else
		{
			m_showState = WindowState(m_showState | WST_MINIMIZED);
			m_showState = WindowState(m_showState & ~WST_MAXIMIZED);

			GetParentWnd()->SetMinimizedChild(this, true);
		}
	}

	void Window::Maximize()
	{
		if (!(m_flags & WindowFlags::WIN_MINMAX))
		{
			return;
		}

		if (m_showState & WST_MAXIMIZED)
		{
			Restore();
		}
		else
		{
			m_scrollBars->ScrollTo(&Point({ 0,0 }));
			m_showState = WindowState(m_showState | WST_MAXIMIZED);
			m_showState = WindowState(m_showState & ~WST_MINIMIZED);
		}
	}

	void Window::Restore()
	{
		if (!(m_flags & WindowFlags::WIN_MINMAX))
		{
			return;
		}

		m_showState = WindowState(m_showState & ~(WST_MAXIMIZED | WST_MINIMIZED));
	}

	int Window::GetMinimizedChildIndex(WindowRef child) const
	{
		auto find = std::find_if(m_minimizedChildren.begin(), m_minimizedChildren.end(), [child](WindowRef min) { return min == child; });
		if (find != m_minimizedChildren.end())
		{
			return (int)(find - m_minimizedChildren.begin());
		}

		return -1;
	}

	void Window::Close()
	{
		if (!(m_flags & WindowFlags::WIN_CLOSE))
		{
			return;
		}

		PostEvent(EVENT_WINDOW_CLOSE);
	}

	void Window::SetMinimizedChild(WindowRef child, bool add)
	{
		if (add)
		{
			if (GetMinimizedChildIndex(child) != -1)
				return;

			auto firstHole = std::find_if(m_minimizedChildren.begin(), m_minimizedChildren.end(), [](WindowRef min) { return min == nullptr; });
			if (firstHole == m_minimizedChildren.end())
			{
				m_minimizedChildren.push_back(child);
			}
			else
			{
				*firstHole = child;
			}
		}
		else
		{
			auto find = std::find_if(m_minimizedChildren.begin(), m_minimizedChildren.end(), [child](WindowRef min) { return min == child; });
			if (find != m_minimizedChildren.end())
			{
				*find = nullptr;
			}
		}
	}

	bool Window::HandleEvent(SDL_Event * e)
	{
		Point pt(e->button.x, e->button.y);
		HitResult hit = HitTest(&pt);

		if (e->type == SDL_MOUSEBUTTONDOWN)
		{
			if (e->button.button == SDL_BUTTON_LEFT)
			{
				WINMGR().SetActive(this);
				bool capture = false;
				switch (HitZone(hit))
				{
				case HIT_CLOSEBUTTON:
				case HIT_MINBUTTON:
				case HIT_MAXBUTTON:
					capture = true;
					ToggleButtonState(hit, true);
					break;

				case HIT_TITLEBAR:
					if (e->button.clicks == 2)
					{
						Maximize();
					}
					else
					{
						capture = true;
					}
					break;
				default: break;
				}

				if (hit.zone & (HIT_BORDER_ANY | HIT_CORNER_ANY))
				{
					capture = true;
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
				ToggleButtonState(capture.Target, false);
				switch (HitZone(hit))
				{
				case HIT_CLOSEBUTTON:
				case HIT_MAXBUTTON:
				case HIT_MINBUTTON:					
					if (hit == capture.Target)
					{
						ButtonPushed(capture.Target);
					}
					break;
				default: break;
				}
				WINMGR().ReleaseCapture();
				return true;
			}
		}
		else if (e->type == SDL_MOUSEWHEEL)
		{
			int y = e->wheel.y;
			if (SDL_MOUSEWHEEL_FLIPPED)
			{
				y = -y;
			}

			if (GetScrollBars())
			{
				GetScrollBars()->ScrollRel(&Point(0, y * 10));
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
				case HIT_TITLEBAR:
					MovePos(&newPos);
					break;
				case HIT_BORDER_LEFT:
					MoveRect(&Rect(newPos.x, capture.Origin.y, capture.Origin.w + delta.x, capture.Origin.h));
					break;
				case HIT_BORDER_RIGHT:
					Resize(&Point(capture.Origin.w - delta.x, capture.Origin.h));
					break;
				case HIT_BORDER_TOP:
					MoveRect(&Rect(capture.Origin.x, newPos.y, capture.Origin.w, capture.Origin.h + delta.y));
					break;
				case HIT_BORDER_BOTTOM:
					Resize(&Point(capture.Origin.w, capture.Origin.h - delta.y));
					break;
				case HIT_CORNER_TOPLEFT:
					MoveRect(&Rect(newPos.x, newPos.y, capture.Origin.w + delta.x, capture.Origin.h + delta.y));
					break;
				case HIT_CORNER_TOPRIGHT:
					MoveRect(&Rect(capture.Origin.x, newPos.y, capture.Origin.w - delta.x, capture.Origin.h + delta.y));
					break;
				case HIT_CORNER_BOTTOMLEFT:
					MoveRect(&Rect(newPos.x, capture.Origin.y, capture.Origin.w + delta.x, capture.Origin.h - delta.y));
					break;
				case HIT_CORNER_BOTTOMRIGHT:
					Resize(&Point(capture.Origin.w - delta.x, capture.Origin.h - delta.y));
					break;
				case HIT_CLOSEBUTTON:
				case HIT_MAXBUTTON:
				case HIT_MINBUTTON:
					ToggleButtonState(capture.Target, capture.Target.target->HitTest(&pt) == capture.Target);
					break;
				default:
					handled = false;
				}
				if (handled)
					return handled;
			}
			else
			{
				bool handled = true;
				switch ((HitZone)hit)
				{
				case HIT_BORDER_TOP:
				case HIT_BORDER_BOTTOM:
					SDL_SetCursor(RES().FindCursor("size.NS"));
					break;
				case HIT_BORDER_LEFT:
				case HIT_BORDER_RIGHT:
					SDL_SetCursor(RES().FindCursor("size.WE"));
					break;
				case HIT_CORNER_TOPLEFT:
				case HIT_CORNER_BOTTOMRIGHT:
					SDL_SetCursor(RES().FindCursor("size.NWSE"));
					break;
				case HIT_CORNER_TOPRIGHT:
				case HIT_CORNER_BOTTOMLEFT:
					SDL_SetCursor(RES().FindCursor("size.NESW"));
					break;
				case HIT_TITLEBAR:
				case HIT_CLOSEBUTTON:
				case HIT_MAXBUTTON:
				case HIT_MINBUTTON:
					SDL_SetCursor(RES().FindCursor("default"));
					break;
				default:
					SDL_SetCursor(RES().FindCursor("default"));
					handled = false;
				}

				if (handled)
				{
					return handled;
				}
			}
		}

		// Pass to menu
		if (m_menu && m_menu->HandleEvent(e))
		{
			return true;
		}

		if (m_toolbar && m_toolbar->HandleEvent(e))
		{
			return true;
		}

		// Pass to controls
		{
			for (auto & child : m_controls)
			{
				if (child.second->HandleEvent(e))
				{
					return true;
				}
			}
		}

		if (e->type == SDL_KEYDOWN && GetScrollBars())
		{
			switch (e->key.keysym.sym)
			{
			case SDLK_LEFT:
				GetScrollBars()->ScrollRel(&Point(-2, 0)); return true;
			case SDLK_RIGHT:
				GetScrollBars()->ScrollRel(&Point(2, 0)); return true;
			case SDLK_UP:
				GetScrollBars()->ScrollRel(&Point(0, -2)); return true;
			case SDLK_DOWN:
				GetScrollBars()->ScrollRel(&Point(0, 2)); return true;
			}
		}

		return false;
	}

	void Window::SetMenu(MenuPtr menu)
	{	
		m_menu = menu; 
		m_menu->SetParent(this); 
		m_menu->Init();
	}

	void Window::SetToolbar(ToolbarPtr toolbar)
	{
		m_toolbar = toolbar;
		m_toolbar->SetParent(this);
		m_toolbar->Init();
	}

	struct Window::shared_enabler : public Window
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: Window(std::forward<Args>(args)...)
		{
		}
	};
}