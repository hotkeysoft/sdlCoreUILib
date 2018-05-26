#include "stdafx.h"
#include <SDL.h>
#include <SDL_image.h>
#include "Image.h"

namespace CoreUI
{
	Image::Image(RendererRef renderer) : Widget("image", renderer, nullptr, Rect(), nullptr)
	{
		if (m_renderer == nullptr)
		{
			throw std::invalid_argument("no renderer");
		}
	}

	ImagePtr Image::FromFile(RendererRef renderer, const char* fileName)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer);
		ImagePtr image = std::static_pointer_cast<Image>(ptr);
		return (image->LoadFromFile(fileName)) ? image : nullptr;
	}

	ImagePtr Image::FromTexture(RendererRef renderer, TexturePtr texture)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer);
		ImagePtr image = std::static_pointer_cast<Image>(ptr);
		return (image->LoadFromTexture(texture)) ? image : nullptr;
	}

	ImagePtr Image::FromMap(RendererRef renderer, TexturePtr texture, RectRef subset)
	{
		auto ptr = std::make_shared<shared_enabler>(renderer);
		ImagePtr image = std::static_pointer_cast<Image>(ptr);
		return (image->LoadFromMap(texture, subset)) ? image : nullptr;
	}

	bool Image::LoadFromFile(const char* fileName)
	{
		m_texture = TexturePtr(IMG_LoadTexture(m_renderer, fileName), sdl_deleter());
		if (m_texture)
		{
			SDL_QueryTexture(m_texture.get(), NULL, NULL, &m_rect.w, &m_rect.h);
			m_rect.x = 0;
			m_rect.y = 0;
			return true;
		}
		m_rect = Rect();
		return false;
	}

	bool Image::LoadFromTexture(TexturePtr texture)
	{
		m_texture = texture;
		if (m_texture == nullptr)
		{
			m_rect = Rect();
			return false;
		}

		SDL_QueryTexture(m_texture.get(), NULL, NULL, &m_rect.w, &m_rect.h);
		m_rect.x = 0;
		m_rect.y = 0;
		return true;
	}

	bool Image::LoadFromMap(TexturePtr texture, RectRef subset)
	{
		m_texture = texture;
		if (m_texture == nullptr)
		{
			m_rect = Rect();
			return false;
		}

		m_rect = *subset;
		return true;
	}

	void Image::Draw(const PointRef pos)
	{
		Rect rect = m_rect.Offset(pos);
		if (m_texture && m_renderer)
		{
			SDL_RenderCopy(m_renderer, m_texture.get(), &m_rect, &rect);
		}
	}
	
	void Image::Draw(const RectRef rect, uint8_t align)
	{
		if (m_texture && m_renderer)
		{
			bool hCenter = (align & IMG_H_CENTER) == IMG_H_CENTER;
			bool vCenter = (align & IMG_V_CENTER) == IMG_V_CENTER;

			Rect target = m_rect.CenterInTarget(rect, hCenter, vCenter);
			Rect source = { m_rect.x, m_rect.y, target.w, target.h };

			int deltaX = target.x - rect->x;
			if (deltaX < 0) // Clip x
			{
				target.x = rect->x;
				target.w = rect->w;

				source.x -= deltaX;
				source.w = rect->w;
			}

			int deltaY = target.y - rect->y;
			if (deltaY < 0) // Clip y
			{
				target.y = rect->y;
				target.h = rect->h;

				source.y -= deltaY;
				source.h = rect->h;
			}

			if ((align & IMG_H_CENTER) == IMG_H_RIGHT)
			{
				target.x += rect->w - source.w;
			}

			if ((align & IMG_V_CENTER) == IMG_V_BOTTOM)
			{
				target.y += rect->h - source.h;
			}

			SDL_RenderCopy(m_renderer, m_texture.get(), &source, &target);
		}
	}

	struct Image::shared_enabler : public Image
	{
		template <typename... Args>
		shared_enabler(Args &&... args)
			: Image(std::forward<Args>(args)...)
		{
		}
	};
}