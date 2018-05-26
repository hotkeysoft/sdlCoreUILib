#pragma once
#include "Common.h"
#include <string>

#ifdef  COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#else
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#endif

namespace CoreUI
{
	class DllExport Color : public SDL_Color
	{
	public:
		Color() : SDL_Color({ 0, 0, 0, 255 }) {}
		Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) : SDL_Color({ red, green, blue, alpha }) {};

		bool operator==(const Color & rhs) { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; }
		bool operator!=(const Color & rhs) { return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a; }

		bool IsTransparent() { return a == 0; }

		Color Darken() const { return Color(r / 2, g / 2, b / 2, a); }

		static const Color C_TRANSPARENT;

		static const Color C_WHITE;
		static const Color C_BLACK;
		static const Color C_VLIGHT_GREY;
		static const Color C_LIGHT_GREY; 
		static const Color C_MED_GREY;
		static const Color C_DARK_GREY;
	};
}