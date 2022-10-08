#include "stdafx.h"
#include "SDL.h"
#include "ResourceManager.h"
#include "WindowManager.h"
#include "Window.h"
#include "Tooltip.h"
#include <algorithm>
#include <iostream>

#ifdef _WINDOWS
#include "basetsd.h"
#endif

namespace CoreUI
{
#ifndef _WINDOWS
	__inline void * LongToPtr(const long l)
	{
		return (void*)l;
	}

	__inline unsigned long PtrToUlong(const void *p)
	{
		return((unsigned long)p);
	}
#endif

	WindowManager & WindowManager::Get()
	{
		static WindowManager manager;
		return manager;
	}

	void WindowManager::Init(SDL_Window * window, RendererRef renderer)
	{
		m_window = window;
		m_renderer = renderer;
		GetEventType("timer");

		LoadScreenResolutions();
	}

	void WindowManager::Dispose()
	{
		m_activeWindow = nullptr;
		m_capture = CaptureInfo();
		m_windowSize = Rect();
		m_screenResolutions.clear();

		m_registeredEvents.clear();
		m_registeredEventsReverse.clear();
		m_timers.clear();

		m_windows.clear();
	}

	void WindowManager::Draw()
	{
		for (auto & window : m_windows)
		{
			window->Draw();
		}

		// Active menu needs to be drawn on top of everything
		if (m_activeWindow && !(m_activeWindow->GetShowState() & WST_MINIMIZED))
		{
			m_activeWindow->DrawMenu();
		}

		if (m_tooltipWindow)
		{
			m_tooltipWindow->Draw();
		}
	}

	WindowPtr WindowManager::AddWindowFill(const char * id, CreationFlags flags)
	{

		return AddWindow(id, nullptr, Rect(), flags | WIN_FILL);
	}

	WindowPtr WindowManager::AddWindow(const char * id, Rect pos, CreationFlags flags)
	{
		return AddWindow(id, nullptr, pos, flags);
	}

	WindowPtr WindowManager::AddWindow(const char* id, WindowPtr parent, Rect pos, CreationFlags flags)
	{
		if (FindWindow(id) != nullptr)
		{
			throw std::invalid_argument("windows already exists:" + (std::string)id);
		}

		WindowPtr newWindow = Window::Create(id, m_renderer, parent.get(), RES().FindFont("default"), pos, flags);

		if (id == Tooltip::GetId())
		{
			m_tooltipWindow = newWindow;
		}
		else
		{
			m_windows.push_back(newWindow);
		}

		return newWindow;
	}

	WindowPtr WindowManager::FindWindow(const char* id)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id can't be null");
		}
		else if (id == Tooltip::GetId())
		{
			return m_tooltipWindow;
		}

		for (auto & it : m_windows)
		{
			if (id == it->GetId())
			{
				return it;
			}
		}

		return nullptr;
	}

	WindowRef WindowManager::FindParentWindow(WidgetRef w)
	{
		WindowRef wnd = nullptr;
		while (w)
		{
			wnd = dynamic_cast<WindowRef>(w);
			if (wnd)
			{
				break;
			}
			w = w->GetParent();
		}
		return wnd;
	}

	bool WindowManager::RemoveWindow(const char* id)
	{
		if (id == Tooltip::GetId())
		{
			m_tooltipWindow = nullptr;
			return true;
		}

		WindowPtr wnd = FindWindow(id);
		if (wnd == nullptr)
		{
			return false;
		}
		m_windows.remove(wnd);
		if (m_activeWindow == wnd.get())
		{
			m_activeWindow = nullptr;
		}
		return true;
	}

	WindowManager::WindowList WindowManager::GetWindowList(WindowRef parent)
	{
		WindowList childWindows;
		std::copy_if(m_windows.begin(), m_windows.end(), std::back_inserter(childWindows),
			[parent](WindowPtr window) { return window->GetParent() == parent; });
		return childWindows;
	}

	HitResult WindowManager::HitTest(PointRef pt)
	{
		for (auto it = m_windows.rbegin(); it != m_windows.rend(); ++it)
		{
			auto & window = *it;
			auto hitResult = window->HitTest(pt);

			if (hitResult)
			{
				return hitResult;
			}
		}

		return HIT_NOTHING;
	}

	CoreUI::WindowRef WindowManager::GetActive()
	{
		return m_activeWindow ? m_activeWindow : Window::GetNullWnd();
	}

	void WindowManager::SetActive(WindowRef win)
	{
		if ((win == m_activeWindow) || (win->GetFlags() & WIN_NOACTIVE))
		{
			return;
		}

		if (m_activeWindow)
		{
			m_activeWindow->PostEvent(Window::EVENT_WINDOW_DEACTIVATED);
		}

		m_activeWindow = win;

		if (m_activeWindow)
		{
			m_activeWindow->PostEvent(Window::EVENT_WINDOW_ACTIVATED);
		}

		MoveToFront(win);
	}
	const CaptureInfo & WindowManager::StartCapture(HitResult hit, PointRef pt)
	{
		if (hit && hit.target)
		{
			m_capture.Captured = true;
			m_capture.Target = hit;
			m_capture.Origin = hit.target->GetRect(true, false);
			m_capture.Delta = m_capture.Origin.Origin();
			m_capture.Delta.x -= pt->x;
			m_capture.Delta.y -= pt->y;
		}
		return m_capture;
	}

	Uint32 timerCallbackFunc(Uint32 interval, void *param)
	{
		static Uint32 timerEventID = WINMGR().GetEventType(Timer::EventClassName());
		if (timerEventID == (Uint32)-1)
			return -1;

		SDL_Event timerEvent;
		SDL_zero(timerEvent);

		Timer* timer = (Timer*)param;

		timerEvent.type = timerEventID;
		timerEvent.user.code = timer->GetID();
		timerEvent.user.data1 = timer;

		SDL_PushEvent(&timerEvent);
		return(timer->IsOneShot() ? 0 : interval);
	}

	Uint32 WindowManager::AddTimer(Uint32 interval, bool oneShot, Widget* owner)
	{
		std::unique_ptr<Timer> timer = std::make_unique<Timer>(oneShot, owner);

		Uint32 timerID = SDL_AddTimer(interval, timerCallbackFunc, timer.get());
		timer->SetID(timerID);

		m_timers.emplace(timerID, std::move(timer));
		return timerID;
	}

	void WindowManager::DeleteTimer(Uint32 timerID)
	{
		if (!SDL_RemoveTimer(timerID))
		{
			throw std::invalid_argument("invalid timer id");
		}
		m_timers.erase(timerID);
	}

	void WindowManager::DeleteAllTimers()
	{
		for (auto& timer : m_timers)
		{
			SDL_RemoveTimer(timer.second->GetID());
		}
		m_timers.clear();
	}

	void WindowManager::RaiseSingleWindow(WindowRef win)
	{
		const std::string & id = win->GetId();

		auto found = std::find_if(m_windows.begin(), m_windows.end(), [id](WindowPtr window) { return window->GetId() == id; });
		if (found != m_windows.end())
		{
			m_windows.splice(m_windows.end(), m_windows, found);
		}
	}

	void WindowManager::RaiseChildren(WindowRef win)
	{
		RaiseSingleWindow(win);
		for (auto & child : win->GetChildWindows())
		{
			RaiseChildren(child.get());
		}
	}

	void WindowManager::MoveToFront(WindowRef win)
	{
		if (win == nullptr)
			return;

		if (win->HasParent())
		{
			RaiseChildren(win->GetParentWnd());
		}
		RaiseChildren(win);
	}

	TexturePtr WindowManager::SurfaceToTexture(SDL_Surface* surf)
	{
		TexturePtr texture = TexturePtr(SDL_CreateTextureFromSurface(m_renderer, surf), sdl_deleter());

		SDL_FreeSurface(surf);

		return std::move(texture);
	}

	std::string WindowManager::GetEventName(Uint32 eventId) const
	{
		auto found = m_registeredEventsReverse.find(eventId);
		if (found == m_registeredEventsReverse.end())
		{
			return "<null>";
		}
		return found->second;
	}

	Uint32 WindowManager::GetEventType(const char * type)
	{
		if (type == nullptr)
		{
			throw std::invalid_argument("event type is null");
		}

		Uint32 eventId = FindEventType(type);
		if (eventId == (Uint32)-1)
		{
			eventId = SDL_RegisterEvents(1);
			if (eventId == (Uint32)-1)
			{
				throw std::out_of_range(" too many registered events");
			}
			m_registeredEvents[type] = eventId;
			m_registeredEventsReverse[eventId] = type;

			std::cout << "Register: " << type << ", eventID = " << eventId << std::endl;
		}

		return eventId;
	}

	Uint32 WindowManager::FindEventType(const char * type) const
	{
		auto it = m_registeredEvents.find(type);
		if (it != m_registeredEvents.end())
		{
			return it->second;
		}
		return (Uint32)-1;
	}

	bool WindowManager::IsFullscreen() const
	{
		return SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN;
	}

	void WindowManager::ToggleFullscreen()
	{
		SDL_SetWindowFullscreen(m_window, IsFullscreen() ? 0 : SDL_WINDOW_FULLSCREEN);
		SDL_ShowCursor(1);
		PostEvent(EVENT_WINDOWMANAGER_DISPLAYCHANGED);
	}

	ScreenResolution WindowManager::GetScreenResolution() const
	{
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(0, &mode);
		ScreenResolution res = { -1, mode.w, mode.h };
		return res;
	}

	void WindowManager::SetScreenResolution(int modeId)
	{
		SDL_DisplayMode mode;
		if (SDL_GetDisplayMode(0, modeId, &mode) == -1)
		{
			std::cout << "Cannot find mode " << modeId << std::endl;
			return;
		}

		if (IsFullscreen())
		{
			SDL_SetWindowFullscreen(m_window, 0);
			SDL_SetWindowSize(m_window, mode.w, mode.h);
			SDL_SetWindowDisplayMode(m_window, &mode);
			SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
		}
		else
		{
			SDL_SetWindowSize(m_window, mode.w, mode.h);
		}

		PostEvent(EVENT_WINDOWMANAGER_DISPLAYCHANGED);

		m_windowSize.Clear();
	}

	Rect WindowManager::GetWindowSize(bool force) const
	{
		if (force || m_windowSize.IsEmpty())
		{
			SDL_GetWindowSize(m_window, &m_windowSize.w, &m_windowSize.h);
		}

		return m_windowSize;
	}

	int WindowManager::LoadScreenResolutions()
	{
		m_screenResolutions.clear();
		static int display_in_use = 0; /* Only using first display */

		int i, display_mode_count;
		SDL_DisplayMode mode;

		display_mode_count = SDL_GetNumDisplayModes(display_in_use);
		if (display_mode_count < 1) {
			SDL_Log("SDL_GetNumDisplayModes failed: %s", SDL_GetError());
			return 1;
		}

		for (i = 0; i < display_mode_count; ++i) {
			if (SDL_GetDisplayMode(display_in_use, i, &mode) != 0) {
				SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
				return 1;
			}

//			if (mode.h < 720)
//				continue;
			m_screenResolutions.insert({ i, mode.w, mode.h });
		}

		return 0;
	}

	void WindowManager::PostEvent(EventCode code, void * data1, void * data2)
	{
		static Uint32 type = GetEventType("winmgr");

		SDL_Event toPost;
		SDL_zero(toPost);

		toPost.type = type;
		toPost.user.code = code;
		toPost.user.data1 = data1;
		toPost.user.data2 = data2;

		SDL_PushEvent(&toPost);
	}

}