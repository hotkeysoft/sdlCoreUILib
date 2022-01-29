#pragma once
#include "Common.h"
#include "Color.h"
#include "Rect.h"
#include "Point.h"
#include "Widget.h"
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <set>
#include <sstream>

namespace CoreUI
{
	struct DllExport CaptureInfo
	{
		CaptureInfo() : Captured(false) {}
		void Reset() { Captured = false; Target = HIT_NOTHING; Delta = Point(); Origin = Rect(); }
		explicit operator bool() const { return Captured; }
		bool Captured;
		HitResult Target;
		Point Delta;
		Rect Origin;
	};

	struct DllExport ScreenResolution
	{
		int id;
		int w;
		int h;

		bool operator<(const ScreenResolution & rhs) const { return (w == rhs.w) ? (h > rhs.h) : (w > rhs.w); }
		std::string ToString() const
		{
			std::ostringstream os;
			os << w << " x " << h;
			return os.str();
		}
	};

	enum WindowManagerEvents : EventCode
	{
		EVENT_WINDOWMANAGER_DISPLAYCHANGED, 
	};

	class DllExport WindowManager
	{
	public:
		using ResolutionList = std::set<ScreenResolution>;
		using EventMap = std::map<std::string, Uint32>;
		using ReverseEventMap = std::map<Uint32, std::string>;
		using TimerList = std::vector<Uint32>;
		using WindowList = std::list<WindowPtr>;

		virtual ~WindowManager() = default;
		WindowManager(const WindowManager&) = delete;
		WindowManager& operator=(const WindowManager&) = delete;
		WindowManager(WindowManager&&) = delete;
		WindowManager& operator=(WindowManager&&) = delete;

		static WindowManager & Get();

		void Init(SDL_Window * window, RendererRef renderer);
		void Dispose();

		void Draw();

		WindowPtr AddWindow(const char* id, Rect pos, CreationFlags flags = WindowFlags::WIN_DEFAULT);
		WindowPtr AddWindow(const char* id, WindowPtr parent, Rect pos, CreationFlags flags = WindowFlags::WIN_DEFAULT);
		WindowPtr AddWindowFill(const char* id, CreationFlags flags = WindowFlags::WIN_DEFAULT);
		WindowPtr FindWindow(const char* id);
		WindowList GetWindowList(WindowRef parent);
		bool RemoveWindow(const char* id);

		Uint32 GetEventType(const char * type = "winmgr");
		std::string GetEventName(Uint32 eventType) const;

		void MoveToFront(WindowRef);

		HitResult HitTest(PointRef);
		WindowRef GetActive();
		void SetActive(WindowRef);

		const CaptureInfo & StartCapture(HitResult, PointRef);
		const CaptureInfo & GetCapture() const { return m_capture; }
		void ReleaseCapture() { m_capture.Reset(); }

		Uint32 AddTimer(Uint32 interval);
		void DeleteTimer(Uint32 timerID);

		TexturePtr SurfaceToTexture(SDL_Surface * surf);

		bool IsFullscreen() const;
		void ToggleFullscreen();
		ScreenResolution GetScreenResolution() const;
		void SetScreenResolution(int modeId);
		ResolutionList GetScreenResolutions() const { return m_screenResolutions; }
		Rect GetWindowSize() const;

	protected:
		void PostEvent(EventCode eventCode, void * data1 = nullptr, void * data2 = nullptr);

		Uint32 FindEventType(const char * type) const;

		void RaiseSingleWindow(WindowRef);
		void RaiseChildren(WindowRef);

		int LoadScreenResolutions();

		WindowManager() : m_renderer(nullptr) {}
		RendererRef m_renderer;
		SDL_Window * m_window;

		WindowList m_windows;
		WindowRef m_activeWindow;

		EventMap m_registeredEvents;
		ReverseEventMap m_registeredEventsReverse;
		TimerList m_timers;

		CaptureInfo m_capture;

		mutable Rect m_windowSize;
		ResolutionList m_screenResolutions;
	};

	constexpr auto WINMGR = &WindowManager::Get;

}