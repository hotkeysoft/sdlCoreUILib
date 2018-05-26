#include "stdafx.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "ResourceManager.h"
#include "Widgets\Image.h"
#include "Widgets\ImageMap.h"

namespace CoreUI
{
	ResourceManager & ResourceManager::Get()
	{
		static ResourceManager manager;
		return manager;
	}

	void ResourceManager::Init(RendererPtr & renderer)
	{
		m_renderer = renderer.get();
	}

	FontRef ResourceManager::LoadFont(const char * id, const char * fileName, size_t size)
	{		
		if (id == nullptr || fileName == nullptr)
		{
			throw std::invalid_argument("id or filename is null");
		}
		if (m_fonts.find(id) != m_fonts.end())
		{
			throw std::invalid_argument("font id already loaded: " + std::string(id));
		}

		FontPtr font = FontPtr(TTF_OpenFont(fileName, (int)size), sdl_deleter());
		FontRef ref = font.get();
		if (font != nullptr)
		{
			m_fonts[id] = std::move(font);
		}

		return ref;
	}

	FontRef ResourceManager::FindFont(const char * id)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		auto it = m_fonts.find(id);
		if (it == m_fonts.end())
		{
			return nullptr;
		}

		return it->second.get();
	}

	ImageRef ResourceManager::LoadImage(const char * id, const char * fileName)
	{
		if (id == nullptr || fileName == nullptr)
		{
			throw std::invalid_argument("id or filename is null");
		}
		if (m_images.find(id) != m_images.end())
		{
			throw std::invalid_argument("image id already loaded: " + std::string(id));
		}

		ImagePtr image = Image::FromFile(m_renderer, fileName);
		if (image != nullptr)
		{			
			m_images[id] = image;
		}
		else
		{
			std::cerr << "Image not loaded" << fileName << std::endl;
		}
		
		return image.get();
	}

	ImageMapRef ResourceManager::LoadImageMap(const char * id, const char * fileName, int tileWidth, int tileHeight)
	{
		if (id == nullptr || fileName == nullptr)
		{
			throw std::invalid_argument("id or filename is null");
		}
		if (m_images.find(id) != m_images.end())
		{
			throw std::invalid_argument("image id already loaded: " + std::string(id));
		}

		ImageMapPtr image = ImageMap::FromFile(m_renderer, fileName, tileWidth, tileHeight);
		if (image == nullptr)
		{
			std::cerr << "Image not loaded" << fileName << std::endl;
			return nullptr;
		}

		m_images[id] = image;
		return image.get();
	}

	ImageRef ResourceManager::FindImage(const char * id, int index)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		auto it = m_images.find(id);
		if (it == m_images.end())
		{
			return nullptr;
		}

		if (index == -1)
		{
			return it->second.get();
		}

		ImageMapPtr imageMap = std::dynamic_pointer_cast<ImageMap>(it->second);
		if (imageMap == nullptr)
		{
			throw std::invalid_argument("not an image map");
		}

		return imageMap->GetTile(index);
	}

	CursorRef ResourceManager::LoadCursor(const char * id, SDL_SystemCursor cursorType)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id");
		}
		if (m_images.find(id) != m_images.end())
		{
			throw std::invalid_argument("cursor id: " + std::string(id));
		}

		CursorPtr cursor = CursorPtr(SDL_CreateSystemCursor(cursorType), sdl_deleter());
		CursorRef ref = cursor.get();
		if (cursor != nullptr)
		{
			m_cursors[id] = std::move(cursor);
		}

		return ref;

	}

	CursorRef ResourceManager::FindCursor(const char * id)
	{
		if (id == nullptr)
		{
			throw std::invalid_argument("id is null");
		}

		auto it = m_cursors.find(id);
		if (it == m_cursors.end())
		{
			return nullptr;
		}

		return it->second.get();
	}
}