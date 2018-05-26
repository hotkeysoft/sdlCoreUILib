#pragma once
#include "Common.h"
#include "Color.h"
#include <string>
#include <map>

#ifdef  COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#else
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#endif

namespace CoreUI
{
	class DllExport ResourceManager
	{
	public:
		using FontList = std::map<std::string, FontPtr>;
		using ImageList = std::map<std::string, ImagePtr>;
		using CursorList = std::map<std::string, CursorPtr>;

		virtual ~ResourceManager() = default;
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		static ResourceManager & Get();

		void Init(RendererPtr & renderer);

		// Fonts
		FontRef LoadFont(const char * id, const char * fileName, size_t size);
		FontRef FindFont(const char * id);

		// Images
		ImageRef LoadImage(const char * id, const char * fileName);
		ImageMapRef LoadImageMap(const char * id, const char * fileName, int tileWidth, int tileHeight);
		ImageRef FindImage(const char * id, int index = -1);

		// Cursors
		CursorRef LoadCursor(const char * id, SDL_SystemCursor);
		CursorRef FindCursor(const char * id);

	protected:
		ResourceManager() : m_renderer(nullptr) {}
		RendererRef m_renderer;
		FontList m_fonts;
		ImageList m_images;
		CursorList m_cursors;
	};

	constexpr auto RES = &ResourceManager::Get;
}