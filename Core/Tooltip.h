#pragma once
#include "Common.h"
#include "Window.h"

namespace CoreUI
{
	class DllExport Tooltip
	{
	public:
		static Tooltip& Get();

		void Show(WidgetRef owner, Point pos, const char* text);
		void Hide(WidgetRef owner);

	protected:
		WindowPtr m_wnd;
		WidgetRef m_owner = nullptr;
	};

	constexpr auto TOOLTIP = &Tooltip::Get;
}