#pragma once
#include "Common.h"
#include "Core/Rect.h"
#include "Core/Point.h"
#include "Core/Widget.h"
#include <string>

namespace ResourceMap
{
	struct ResourceInfo;
}

namespace CoreUI
{
	class DllExport Image : public Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Image)

		enum ImageAlign : uint8_t {
			IMG_H_LEFT = 1,
			IMG_H_RIGHT = 2,
			IMG_H_CENTER = IMG_H_LEFT | IMG_H_RIGHT,

			IMG_V_TOP = 8,
			IMG_V_BOTTOM = 16,
			IMG_V_CENTER = IMG_V_TOP | IMG_V_BOTTOM,

			IMG_CENTER = IMG_H_CENTER | IMG_V_CENTER,

			IMG_DEFAULT = IMG_H_LEFT | IMG_V_TOP,
		};

		virtual ~Image() = default;
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

		static ImagePtr FromFile(RendererRef renderer, const char* fileName);
		static ImagePtr FromTexture(RendererRef renderer, TexturePtr texture);
		static ImagePtr FromMap(RendererRef renderer, TexturePtr texture, RectRef subset);
		static ImagePtr FromResource(RendererRef renderer, ResourceMap::ResourceInfo & res);

		explicit operator bool() const { return IsSet(); }

		void Draw() override {}
		void Draw(const PointRef pos);
		void Draw(const RectRef rect, uint8_t align = IMG_DEFAULT);
		bool IsSet() const { return m_texture != nullptr; }

	protected:
		Image(RendererRef renderer);

		virtual bool LoadFromFile(const char* fileName);
		virtual bool LoadFromTexture(TexturePtr);
		virtual bool LoadFromMap(TexturePtr, RectRef);
		virtual bool LoadFromResource(ResourceMap::ResourceInfo & res);

		TexturePtr m_texture;

		struct shared_enabler;
	};
}