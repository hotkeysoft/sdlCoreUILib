#pragma once
#include "Common.h"
#include "Core\Rect.h"

namespace CoreUI
{
	class ClipRect
	{
	public:
		ClipRect(RendererRef ren, RectRef rect, bool mergeCurrent = true) : m_renderer(ren)
		{
			if (rect == nullptr)
			{
				m_clipRect = Rect();
				SDL_RenderSetClipRect(m_renderer, nullptr);
				return;
			}

			SDL_RenderGetClipRect(m_renderer, &m_lastClipRect);
			if (mergeCurrent && !m_lastClipRect.IsEmpty())
			{
				m_clipRect = rect->IntersectRect(&m_lastClipRect);		
			}
			else
			{
				m_clipRect = *rect;
			}

#ifdef DEBUG_CLIP
			SDL_SetRenderDrawColor(m_renderer, 255, m_clipRect.IsEmpty() ? 255 : 0, 0, 255);
			SDL_RenderDrawRect(m_renderer, &m_clipRect);
#endif
			if (m_clipRect.IsEmpty())
			{
				SDL_RenderSetClipRect(m_renderer, &Rect(0,0,1,1));
				
			}
			else
			{
				SDL_RenderSetClipRect(m_renderer, &m_clipRect);
			}
		}

		explicit operator bool() const { return !m_clipRect.IsEmpty(); }
		RectRef GetClipRegion() { return &m_clipRect; }
		bool IsClipRegionEmpty() const { return m_clipRect.IsEmpty(); }

		~ClipRect()
		{
			if (m_lastClipRect.IsEmpty())
				return;
			SDL_RenderSetClipRect(m_renderer, &m_lastClipRect);
		}
	private:
		RendererRef m_renderer;
		Rect m_clipRect;
		Rect m_lastClipRect;
	};
}
