#pragma once
#include "CoreUI.h"
#include "Image.h"
#include <vector>
#include <string>

namespace ResourceMap
{
	struct ResourceInfo;
}

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
		static ImageMapPtr FromResource(RendererRef renderer, ResourceMap::ResourceInfo & res);
		ImageRef GetTile(int index);

	protected:
		ImageMap(RendererRef renderer, int tileWidth, int tileHeight);

		bool LoadFromFile(const char* fileName) override;
		bool LoadFromResource(ResourceMap::ResourceInfo & res) override;
		bool PostLoad();
		void LoadTile(int index);	

		int m_tileWidth;
		int m_tileHeight;

		int m_cols;
		int m_rows;

		ImageTiles m_tiles;

		struct shared_enabler;
	};
}