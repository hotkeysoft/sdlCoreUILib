#pragma once
#include "Common.h"
#include "Widget.h"
#include "Rect.h"
#include "Color.h"
#include "Grid.h"
#include "WindowManager.h"
#include "Widgets/ScrollBars.h"
#include <string>
#include <vector>

namespace CoreUI
{
	class DllExport Window : public Widget
	{
	public:
		enum WindowEvents : EventCode
		{
			EVENT_WINDOW_ACTIVATED, // Window activated
			EVENT_WINDOW_DEACTIVATED, // Window activated
		};

		DECLARE_EVENT_CLASS_NAME(Window)

		using MinWindowList = std::vector<WindowRef>;
		using ControlList = std::map<std::string, WidgetPtr>;

		virtual ~Window() = default;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		static WindowRef GetNullWnd() { return &m_nullWnd; }

		static WindowPtr Create(const char* id, RendererRef renderer, WindowRef parent, FontRef font, Rect rect, CreationFlags flags);
		void SetText(const char * text) override;

		void SetActive() override;
		void SetFocus(WidgetRef focus, WidgetRef parent = nullptr) override;

		WindowManager::WindowList GetChildWindows();

		void AddControl(WidgetPtr);
		WidgetPtr FindControl(const char* id) const;
		const ControlList &GetControls() const { return m_controls; }

		WindowRef GetParentWnd() const { return static_cast<WindowRef>(m_parent); }

		Rect GetRawClientRect(bool relative = true, bool scrolled = true) const; // Excludes scroll bars and menu
		Rect GetClientRect(bool relative = true, bool scrolled = true) const override;
		Rect GetRect(bool relative = true, bool scrolled = true) const override;

		HitResult HitTest(const PointRef) override;
		void Draw() override;
		
		WindowState GetShowState() const { return m_showState; }

		void ToggleButtonState(HitZone button, bool pushed);
		void ButtonPushed(HitZone button);

		// Size and Position
		bool MoveRel(PointRef rel) override;
		bool MovePos(PointRef pos) override;
		bool MoveRect(RectRef rect) override;
		bool ResizeRel(PointRef rel) override;
		bool Resize(PointRef size) override;

		void Maximize();
		void Minimize();
		void Restore();

		// Grid
		Grid& GetGrid() { return m_grid; }

		bool HandleEvent(SDL_Event *) override;

		ScrollBarsRef GetScrollBars() { return m_scrollBars.get(); }

		bool GetPushedState(HitZone id) { return m_pushedState & id; }

		void SetMenu(MenuPtr menu);
		MenuPtr GetMenu() { return m_menu; }

		void SetToolbar(ToolbarPtr toolbar);
		ToolbarPtr GetToolbar() { return m_toolbar; }

	protected:
		Window(const char* id, RendererRef renderer, WindowRef parent, FontRef font, Rect rect, CreationFlags flags);
		Window();

		Rect GetTitleBarRect(Rect base) const;
		Rect GetSystemMenuButtonRect(Rect base) const;
		Rect GetMinimizeButtonRect(Rect base) const;
		Rect GetMaximizeButtonRect(Rect base) const;

		void SetMinimizedChild(WindowRef child, bool add);
		int GetMinimizedChildIndex(WindowRef child) const;

		void DrawMinMaxButtons(Rect pos, const CoreUI::Color & col);
		void DrawSystemMenuButton(Rect pos, const CoreUI::Color & col);
		void DrawTitleBar(Rect rect, bool active);
		void DrawTitle(Rect rect, bool active);
		void DrawControls();
		void DrawMenu();
		void DrawToolbar();
		void DrawGrid();
		void RenderTitle();

		Rect GetClipRect(WindowRef win);

		WindowState m_showState;
		HitZone m_pushedState;

		static const Color m_activeTitleBarColor;

		TexturePtr m_activeTitle;
		TexturePtr m_inactiveTitle;
		Rect m_titleStrRect;

		MinWindowList m_minimizedChildren;
		ScrollBarsPtr m_scrollBars;

		static Window m_nullWnd;			

		ControlList m_controls;

		MenuPtr m_menu;
		ToolbarPtr m_toolbar;

		Grid m_grid;

		struct shared_enabler;

		friend class ScrollBars;
		friend class WindowManager;
	};
}