#pragma once
#include "Common.h"
#include "Image.h"
#include <vector>
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
	class DllExport ImageMap : public Image
	{
	public:
		DECLARE_EVENT_CLASS_NAME(ImageMap)

		using ImageTiles = std::vector<ImagePtr>;

		virtual ~ImageMap() = default;
		ImageMap(const ImageMap&) = delete;
		ImageMap& operator=(const ImageMap&) = delete;
		ImageMap(ImageMap&&) = delete;
		ImageMap& operator=(ImageMap&&) = delete;

		static ImageMapPtr FromFile(RendererRef renderer, const char* fileName, int tileWidth, int tileHeight);
		ImageRef GetTile(int index);

	protected:
		ImageMap(RendererRef renderer, int tileWidth, int tileHeight);

		bool LoadFromFile(const char* fileName);
		void LoadTile(int index);

		int m_tileWidth;
		int m_tileHeight;

		int m_cols;
		int m_rows;

		ImageTiles m_tiles;

		struct shared_enabler;
	};
}