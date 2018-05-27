#pragma once
#include "Common.h"
#include "Core\Widget.h"
#include "Core\Rect.h"
#include "Core\Point.h"
#include <string>
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
	class DllExport Menu : public Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Menu)

		enum TextBoxEvents : EventCode
		{
			EVENT_MENU_SELECTED // MenuItemRef in data2
		};

		using MenuItems = std::vector<MenuItemPtr>;
		using HotkeyMap = std::map<SDL_Keycode, MenuItemPtr>;

		virtual ~Menu() = default;
		Menu(const Menu&) = delete;
		Menu& operator=(const Menu&) = delete;
		Menu(Menu&&) = delete;
		Menu& operator=(Menu&&) = delete;

		void Init() override;

		static MenuPtr Create(RendererRef renderer, const char * id);

		MenuItemPtr AddMenuItem(const char * id, const char * name, SDL_Keycode hotkey = SDLK_UNKNOWN);		

		bool HandleEvent(SDL_Event *) override;
		HitResult HitTest(const PointRef) override;

		void Draw() override {};
		void Draw(const RectRef);

		void OpenMenu(MenuItemRef item);
		void CloseMenu();

		int GetHeight(int clientWidth) const;

		void MoveLeft();
		void MoveRight();
		void MoveUp();
		void MoveDown();

	protected:
		Menu(RendererRef renderer, const char * id);

		MenuItemPtr ItemAt(PointRef pt);

		void DrawActiveFrame(MenuItemRef parent);
		void DrawOpenedMenu(MenuItemPtr & item);
		void CloseMenuItem(MenuItemRef item);

		MenuItems::const_iterator FindMenuItem(MenuItemRef item, MenuItemRef parent = nullptr) const;

		MenuItemRef m_active;
		int m_lineHeight;
		MenuItems m_items;
		HotkeyMap m_hotkeys;

		struct shared_enabler;
	};
}