#pragma once
#include "Common.h"
#include "Core\Widget.h"
#include "Core\Rect.h"
#include "Core\Point.h"
#include <vector>

#ifdef  COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#else
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#endif

namespace CoreUI
{
	class DllExport MenuItem : public Widget, public std::enable_shared_from_this<MenuItem>
	{
	public:
		using MenuItems = std::vector<MenuItemPtr>;
		using HotkeyMap = std::map<SDL_Keycode, MenuItemPtr>;

		virtual ~MenuItem() = default;
		MenuItem(const MenuItem&) = delete;
		MenuItem& operator=(const MenuItem&) = delete;
		MenuItem(MenuItem&&) = delete;
		MenuItem& operator=(MenuItem&&) = delete;

		void Init() override;

		static MenuItemPtr Create(RendererRef renderer, const char * id, const char * name, MenuItemRef parent);

		MenuItemPtr AddMenuItem(const char * id, const char * name, SDL_Keycode hotkey = SDLK_UNKNOWN);
		void AddSeparator();
		MenuItemRef GetParentMenuItem() { return dynamic_cast<MenuItemRef>(m_parent); }

		bool Hit(PointRef pt);

		void Draw() override {}
		void Draw(const PointRef pt);

		bool IsOpened() { return m_opened; }
		bool HasSubMenu() { return !m_items.empty(); }
		LabelRef GetLabel() { return m_label.get(); }

	protected:
		MenuItem(RendererRef renderer, const char * id, const char * name, MenuItemRef parent);

		MenuItemPtr ItemAt(PointRef pt);

		void Render(bool active);

		bool m_opened;
		LabelPtr m_label;
		LabelPtr m_activeLabel;
		Rect m_labelRect;
		MenuItems m_items;
		TexturePtr m_renderedMenu;
		TexturePtr m_renderedActiveMenu;
		Rect m_renderedMenuRect;

		struct shared_enabler;

		HotkeyMap m_hotkeys;

		friend class Menu;
	};
}