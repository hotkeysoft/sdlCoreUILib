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


#ifdef  COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#else
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#endif

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

	class DllExport WindowManager
	{
	public:
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

		void Init(RendererPtr & renderer);

		void Draw();
		WindowPtr AddWindow(const char* id, Rect pos, CreationFlags flags = WindowFlags::WIN_DEFAULT);
		WindowPtr AddWindow(const char* id, WindowPtr parent, Rect pos, CreationFlags flags = WindowFlags::WIN_DEFAULT);
		WindowPtr FindWindow(const char* id);
		WindowList GetWindowList(WindowRef parent);

		Uint32 GetEventType(const char * type);
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

		TexturePtr SurfaceToTexture(SDL_Surface* surf);

	protected:
		Uint32 FindEventType(const char * type) const;

		void RaiseSingleWindow(WindowRef);
		void RaiseChildren(WindowRef);

		WindowManager() : m_renderer(nullptr) {}
		RendererRef m_renderer;

		WindowList m_windows;
		WindowRef m_activeWindow;

		EventMap m_registeredEvents;
		ReverseEventMap m_registeredEventsReverse;
		TimerList m_timers;

		CaptureInfo m_capture;
	};

	constexpr auto WINMGR = &WindowManager::Get;

}