#pragma once
#include "Common.h"
#include "Core/Widget.h"
#include "Core/Rect.h"
#include "Core/Color.h"
#include "Core/WindowManager.h"
#include "Button.h"
#include <string>
#include <vector>

namespace CoreUI
{
	class DllExport ToolbarItem : public Button
	{
	public:
		DECLARE_EVENT_CLASS_NAME(ToolbarItem)

		virtual ~ToolbarItem() = default;
		ToolbarItem(const ToolbarItem&) = delete;
		ToolbarItem& operator=(const ToolbarItem&) = delete;
		ToolbarItem(ToolbarItem&&) = delete;
		ToolbarItem& operator=(ToolbarItem&&) = delete;

		void Init() override;

		static ToolbarItemPtr Create(const char* id, RendererRef renderer, const char* label, ImageRef image = nullptr, FontRef font = nullptr);

		HitResult HitTest(const PointRef pt) override;

		void Draw(const RectRef);

	protected:
		ToolbarItem(const char* id, RendererRef renderer, const char* label, ImageRef image, FontRef font);

		Rect m_drawLocation;

		struct shared_enabler;
	};
}