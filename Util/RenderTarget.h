#pragma once
#include "Common.h"

namespace CoreUI
{
	class RenderTarget
	{
	public:
		RenderTarget(RendererRef ren, TextureRef texture) : m_renderer(ren)
		{
			if (texture == nullptr)
			{
				throw std::invalid_argument("target is null");
			}

			m_oldTarget = SDL_GetRenderTarget(m_renderer);
			if (SDL_SetRenderTarget(m_renderer, texture) == 0)
			{
				m_target = texture;
			}
			else
			{
				m_target = nullptr;
			}
		}

		explicit operator bool() const { return m_target != nullptr; }
		TextureRef GetRenderTarget() { return m_target; }

		~RenderTarget()
		{
			if (m_target == nullptr)
				return;

			SDL_SetRenderTarget(m_renderer, m_oldTarget);
		}
	private:
		RendererRef m_renderer;
		TextureRef m_oldTarget;
		TextureRef m_target;
	};
}
