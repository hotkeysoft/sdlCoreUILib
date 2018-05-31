#pragma once
#include "Common.h"
#include "Core/Widget.h"
#include "Core/Rect.h"
#include "Core/Point.h"
#include <string>
#include <vector>

namespace CoreUI
{
	class DllExport Toolbar : public Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Toolbar)

		enum ToolbarEvents : EventCode
		{
			EVENT_TOOLBAR_CLICKED // ToolbarItemRef in data2
		};

		using ToolbarItems = std::vector<ToolbarItemPtr>;

		virtual ~Toolbar() = default;
		Toolbar(const Toolbar&) = delete;
		Toolbar& operator=(const Toolbar&) = delete;
		Toolbar(Toolbar&&) = delete;
		Toolbar& operator=(Toolbar&&) = delete;

		void Init() override;

		static ToolbarPtr Create(RendererRef renderer, const char * id, int height, CreationFlags flags = 0);
		static ToolbarPtr CreateAutoSize(RendererRef renderer, const char * id, CreationFlags flags = 0);

		ToolbarItemPtr AddToolbarItem(const char * id, ImageRef image, const char * name = nullptr);
		void AddSeparator();

		bool HandleEvent(SDL_Event *) override;
		HitResult HitTest(const PointRef) override;

		void Draw() override {};
		void Draw(const RectRef);

		int GetHeight(int clientWidth) const;

	protected:
		Toolbar(RendererRef renderer, const char * id, int height, CreationFlags flags = 0);

		ToolbarItems::const_iterator FindByID(const char * id) const;
		void UpdateSize(ToolbarItemPtr);

		ToolbarItemPtr ItemAt(PointRef pt);
	
		ToolbarItems m_items;
		int m_height;

		struct shared_enabler;
	};
}