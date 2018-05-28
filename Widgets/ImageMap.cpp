#include "stdafx.h"
#include <SDL.h>
#include <SDL_image.h>
#include "ImageMap.h"
#include "ResourceMap.h"

namespace CoreUI
{
	ImageMap::ImageMap(RendererRef renderer, int tileWidth, int tileHeight) : Image(renderer), m_tileWidth(tileWidth), m_tileHeight(tileHeight)
	{
		if (tileWidth < 8 || tileWidth > 128 || tileHeight < 8 || tileHeight > 128)
		{
			throw std::out_of_range("width and height must be in range [8-128]");
		}
	}

	ImageMapPtr ImageMap::FromFile(RendererRef renderer, const char* fileName, int tileWidth, int tileHeight)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer, tileWidth, tileHeight);
		ImageMapPtr imageMap = std::static_pointer_cast<ImageMap>(ptr);
		return (imageMap->LoadFromFile(fileName)) ? imageMap : nullptr;
	}

	ImageMapPtr ImageMap::FromResource(RendererRef renderer, ResourceMap::ResourceInfo & res)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer, res.i1, res.i2);
		ImageMapPtr imageMap = std::static_pointer_cast<ImageMap>(ptr);
		return (imageMap->LoadFromResource(res)) ? imageMap : nullptr;
	}

	bool ImageMap::LoadFromFile(const char* fileName)
	{
		if (!Image::LoadFromFile(fileName))
		{
			m_texture = nullptr;
			return false;
		}
		
		return PostLoad();
	}

	bool ImageMap::LoadFromResource(ResourceMap::ResourceInfo & res)
	{
		if (!Image::LoadFromResource(res))
		{
			m_texture = nullptr;
			return false;
		}

		return PostLoad();
	}

	bool ImageMap::PostLoad()
	{
		if (m_rect.w % m_tileWidth != 0)
		{
			std::cerr << "imagemap width is not a multiple of tileWidth" << std::endl;
			m_texture = nullptr;
			return false;
		}
		if (m_rect.h % m_tileHeight != 0)
		{
			std::cerr << "imagemap height is not a multiple of tileHeight" << std::endl;
			m_texture = nullptr;
			return false;
		}

		m_cols = m_rect.w / m_tileWidth;
		m_rows = m_rect.h / m_tileHeight;

		m_tiles.resize(m_cols * m_rows);

		return true;
	}

	ImageRef ImageMap::GetTile(int index)
	{
		if (index < 0 || index > m_tiles.size() - 1)
		{
			throw std::out_of_range("invalid tile index");
		}

		if (m_tiles[index] == nullptr)
		{
			LoadTile(index);
		}
		return m_tiles[index].get();
	}

	void ImageMap::LoadTile(int index)
	{
		int row = index / m_cols;
		int col = index % m_cols;

		ImagePtr image = Image::FromMap(m_renderer, m_texture, &Rect(col*m_tileWidth, row*m_tileHeight, m_tileWidth, m_tileHeight));
		m_tiles[index] = image;
	}

	struct ImageMap::shared_enabler : public ImageMap
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: ImageMap(std::forward<Args>(args)...)
		{
		}
	};
}