#pragma once
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <algorithm>
#include <memory>
#include <iostream>

#ifdef COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#elif defined _WINDOWS
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#else 
#define DllExport
#endif

namespace CoreUI
{
	using CreationFlags = uint32_t;
	using EventCode = Sint32;

	enum HitZone : uint32_t {
		HIT_NOTHING				= 0x0,
		HIT_TITLEBAR			= 0x1,
		HIT_CLIENT				= 0x2,

		HIT_BORDER_TOP			= 0x4,
		HIT_BORDER_BOTTOM		= 0x8,
		HIT_BORDER_LEFT			= 0x10,
		HIT_BORDER_RIGHT		= 0x20,

		HIT_CORNER_TOPLEFT		= 0x40,
		HIT_CORNER_TOPRIGHT		= 0x80,
		HIT_CORNER_BOTTOMLEFT	= 0x100,
		HIT_CORNER_BOTTOMRIGHT	= 0x200,

		HIT_BORDER_ANY = HIT_BORDER_LEFT | HIT_BORDER_RIGHT | HIT_BORDER_TOP | HIT_BORDER_BOTTOM,
		HIT_CORNER_ANY = HIT_CORNER_TOPLEFT | HIT_CORNER_TOPRIGHT | HIT_CORNER_BOTTOMLEFT | HIT_CORNER_BOTTOMRIGHT,

		HIT_SYSMENU				= 0x400,
		HIT_MINBUTTON			= 0x800,
		HIT_MAXBUTTON			= 0x1000,

		HIT_BUTTON_ANY = HIT_SYSMENU | HIT_MINBUTTON | HIT_MAXBUTTON,

		HIT_HSCROLL_LEFT		= 0x2000,
		HIT_HSCROLL_RIGHT		= 0x4000,
		HIT_HSCROLL_SLIDER		= 0x8000,
		HIT_HSCROLL_AREA		= 0x10000,

		HIT_HSCROLL_ANY = HIT_HSCROLL_LEFT | HIT_HSCROLL_RIGHT | HIT_HSCROLL_SLIDER | HIT_HSCROLL_AREA,

		HIT_VSCROLL_UP			= 0x20000,
		HIT_VSCROLL_DOWN		= 0x40000,
		HIT_VSCROLL_SLIDER		= 0x80000,
		HIT_VSCROLL_AREA		= 0x100000,

		HIT_VSCROLL_ANY = HIT_VSCROLL_UP | HIT_VSCROLL_DOWN | HIT_VSCROLL_SLIDER | HIT_VSCROLL_AREA,

		HIT_MENU				= 0x200000,
		HIT_MENU_ITEM			= 0x400000,

		HIT_TOOLBAR				= 0x800000,

		HIT_CONTROL				= 0x2000000,
	};

	// Creation flags.  These attribute don't change during the lifetime of the window
	enum WindowFlags : CreationFlags {
		WIN_SYSMENU = 1, // Top left button in titlebar (left side)
		WIN_MINMAX = 2, // Minimize / Maximize buttons in titlebar (right side)
		WIN_CANMOVE = 4, // Window can be moved
		WIN_CANRESIZE = 8, // Window can be resized
		WIN_ACTIVE = 32, // Always look active, usually for main window

		WIN_NOSCROLL = 64, // No scroll bars
		WIN_FILL = 128, // Fill parent
		WIN_AUTOSIZE = 256, // Window grows & shrinks to accomodate contents

		WIN_NOFOCUS = 512, // Control doesn't get focus

		WIN_DIALOG = 1024, // Dialog look

		WIN_BORDERLESS = 2048, // No border or title bar

		WIN_NOACTIVE = 4096, // Window is not activated on click

		WIN_DEFAULT = WIN_SYSMENU | WIN_MINMAX | WIN_CANMOVE | WIN_CANRESIZE,
		WIN_DEFAULTDLG = WIN_SYSMENU | WIN_MINMAX | WIN_CANMOVE | WIN_NOSCROLL | WIN_DIALOG,

		WIN_CUSTOMBASE = 65536
	};

	enum WindowState : uint16_t {
		WST_VISIBLE = 1,
		WST_MAXIMIZED = 2,
		WST_MINIMIZED = 4,
	};

	class Widget;
	using WidgetPtr = std::shared_ptr<Widget>;
	using WidgetRef = Widget * ;

	class TextBox;
	using TextBoxPtr = std::shared_ptr<TextBox>;
	using TextBoxRef = TextBox * ;

	class TreeNode;
	using TreeNodeRef = TreeNode * ;

	class Toolbar;
	using ToolbarPtr = std::shared_ptr<Toolbar>;
	using ToolbarRef = Toolbar * ;

	class ToolbarItem;
	using ToolbarItemPtr = std::shared_ptr<ToolbarItem>;
	using ToolbarItemRef = ToolbarItem * ;

	class Menu;
	using MenuPtr = std::shared_ptr<Menu>;
	using MenuRef = Menu * ;

	class MenuItem;
	using MenuItemPtr = std::shared_ptr<MenuItem>;
	using MenuItemRef = MenuItem * ;

	class Tree;
	using TreePtr = std::shared_ptr<Tree>;
	using TreeRef = Tree * ;

	class Label;
	using LabelPtr = std::shared_ptr<Label>;
	using LabelRef = Label * ;

	class Button;
	using ButtonPtr = std::shared_ptr<Button>;
	using ButtonRef = Button * ;

	class ScrollBars;
	using ScrollBarsPtr = std::unique_ptr<ScrollBars>;
	using ScrollBarsRef = ScrollBars * ;

	struct ScrollState;
	using ScrollStateRef = ScrollState *;

	class Rect;
	using RectRef = Rect * ;

	class Point;
	using PointRef = Point * ;

	class ImageMap;
	using ImageMapPtr = std::shared_ptr<ImageMap>;
	using ImageMapRef = ImageMap * ;

	class Image;
	using ImagePtr = std::shared_ptr<Image>;
	using ImageRef = Image * ;

	class Window;
	using WindowPtr = std::shared_ptr<Window>;
	using WindowRef = Window * ;

	// SDL Wrappers
	using MainWindowRef = SDL_Window * ;
	using RendererRef = SDL_Renderer * ;
	using TextureRef = SDL_Texture * ;
	using FontRef = TTF_Font *;
	using CursorRef = SDL_Cursor * ;

	struct sdl_deleter
	{
		void operator()(MainWindowRef p) const { SDL_DestroyWindow(p); /*std::cout << "DestroyWindow" << std::endl;*/ }
		void operator()(RendererRef p) const { SDL_DestroyRenderer(p); /*std::cout << "DestroyRenderer" << std::endl;*/ }
		void operator()(TextureRef p) const { SDL_DestroyTexture(p); /*std::cout << "DestroyTexture " << std::endl;*/ }
		void operator()(FontRef p) const { TTF_CloseFont(p); /*std::cout << "CloseFont" << std::endl;*/ }
		void operator()(CursorRef p) const { SDL_FreeCursor(p); /*std::cout << "FreeCursor" << std::endl;*/ }
	};

	using MainWindowPtr = std::unique_ptr<SDL_Window, sdl_deleter>;
	using RendererPtr = std::unique_ptr<SDL_Renderer, sdl_deleter>;
	using FontPtr = std::unique_ptr<TTF_Font, sdl_deleter>;
	using CursorPtr = std::unique_ptr<SDL_Cursor, sdl_deleter>;
	using TexturePtr = std::shared_ptr<SDL_Texture>;

	template <typename T>
	T clip(const T& n, const T& lower, const T& upper) {
		return (std::max)(lower, (std::min)(n, upper));
	}
}

