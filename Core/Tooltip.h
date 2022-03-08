#pragma once
#include "CoreUI.h"
#include "Window.h"

namespace CoreUI
{
	class DllExport Tooltip
	{
	public:
		static const std::string GetId() { return "_tooltip"; }
		static Tooltip& Get();

		void Show(WidgetRef owner, Point pos, const char* text);
		void Hide(WidgetRef owner);

	protected:
		WindowPtr m_wnd;
		WidgetRef m_owner = nullptr;
	};

	constexpr auto TOOLTIP = &Tooltip::Get;
}