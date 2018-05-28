#include "stdafx.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "ResourceManager.h"
#include "Widgets\Image.h"
#include "Widgets\ImageMap.h"
#include "Util\WinResource.h"
#include "ResourceMap.h"

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

		LoadInternalResources();
	}

	void ResourceManager::Dispose()
	{
		m_fonts.clear();
		m_images.clear();
		m_cursors.clear();
		m_renderer = nullptr;
	}

	void ResourceManager::LoadInternalResources()
	{
		for (auto & res : ResourceMap::g_ResourceMap)
		{
			LoadInternalResource(res);
		}
	}

	void ResourceManager::LoadInternalResource(ResourceMap::ResourceInfo & res)
	{
		std::cout << "Loading internal resource: " << res.id << std::endl;
		switch (res.type)
		{
		case ResourceMap::RES_IMAGEMAP:
			LoadImageMap(res);
			break;
		case ResourceMap::RES_IMAGE:
			LoadImage(res);
			break;
		case ResourceMap::RES_FONT:
			LoadFont(res);
			break;
		default:
			std::cerr << "Resource type not supported" << res.type << std::endl;
		}
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

	FontRef ResourceManager::LoadFont(ResourceMap::ResourceInfo & res)
	{
		if (res.id == nullptr || res.winResId == 0 || res.winResType == nullptr || res.i1 < 1)
		{
			throw std::invalid_argument("id or resourceID or size is null");
		}
		if (m_fonts.find(res.id) != m_fonts.end())
		{
			throw std::invalid_argument("font id already loaded: " + std::string(res.id));
		}

		WinUtil::WinResource::DllResource resource = WinUtil::WinResource::LoadResource(res.winResId, res.winResType);
		if (!resource.IsLoaded())
		{
			std::cerr << "Window resource not found: " << res.winResId << " while loading font " << res.id << std::endl;
			return nullptr;
		}
		
		FontPtr font = FontPtr(TTF_OpenFontRW(SDL_RWFromConstMem(resource.data, resource.size), 0, res.i1), sdl_deleter());
		FontRef ref = font.get();
		if (font != nullptr)
		{
			m_fonts[res.id] = std::move(font);
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
			std::cerr << "Image not loaded " << fileName << std::endl;
		}
		
		return image.get();
	}

	ImageRef ResourceManager::LoadImage(ResourceMap::ResourceInfo & res)
	{
		if (res.id == nullptr || res.winResId == 0 || res.winResType == nullptr)
		{
			throw std::invalid_argument("id or resourceID is null");
		}
		if (m_images.find(res.id) != m_images.end())
		{
			throw std::invalid_argument("image id already loaded: " + std::string(res.id));
		}

		ImagePtr image = Image::FromResource(m_renderer, res);
		if (image != nullptr)
		{
			m_images[res.id] = image;
		}
		else
		{
			std::cerr << "Image not loaded " << res.id << ", internal resource id: " << res.winResId << std::endl;
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
			std::cerr << "Image map not loaded " << fileName << std::endl;
			return nullptr;
		}

		m_images[id] = image;
		return image.get();
	}

	ImageMapRef ResourceManager::LoadImageMap(ResourceMap::ResourceInfo & res)
	{
		if (res.id == nullptr || res.winResId == 0 || res.winResType == nullptr)
		{
			throw std::invalid_argument("id or winResId is null");
		}
		if (m_images.find(res.id) != m_images.end())
		{
			throw std::invalid_argument("image id already loaded: " + std::string(res.id));
		}

		ImageMapPtr image = ImageMap::FromResource(m_renderer, res);
		if (image == nullptr)
		{
			std::cerr << "Image map not loaded " << res.id << ", internal resource id: " << res.winResId << std::endl;
			return nullptr;
		}

		m_images[res.id] = image;
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