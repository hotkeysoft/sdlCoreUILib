#pragma once
#include "Common.h"
#include "Core\Widget.h"
#include "Core\Rect.h"
#include "Core\Color.h"
#include "Core\WindowManager.h"
#include <string>
#include <vector>

#ifdef  COREUI_EXPORTS 
/*Enabled as "export" while compiling the dll project*/
#define DllExport __declspec(dllexport)  
#else
/*Enabled as "import" in the Client side for using already created dll file*/
#define DllExport __declspec(dllimport)  
#endif

namespace CoreUI
{
	class DllExport Button : public Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Button)

		enum ButtonEvents : EventCode
		{
			EVENT_BUTTON_CLICKED
		};

		virtual ~Button() = default;
		Button(const Button&) = delete;
		Button& operator=(const Button&) = delete;
		Button(Button&&) = delete;
		Button& operator=(Button&&) = delete;

		void Init() override;

		// Regular button at position & size rect
		static ButtonPtr Create(const char* id, RendererRef renderer, Rect rect, const char* label, ImageRef image = nullptr, FontRef font = nullptr, CreationFlags flags = 0);

		// Auto-size
		static ButtonPtr CreateAutoSize(const char* id, RendererRef renderer, const char* label, ImageRef image = nullptr, FontRef font = nullptr, CreationFlags flags = 0);

		Rect GetClientRect(bool relative = true, bool scrolled = true) const override;

		bool HandleEvent(SDL_Event *) override;
		HitResult HitTest(const PointRef) override;
		void Draw() override;
		void Draw(RectRef);

		void SetText(const char *) override;

	protected:
		Button(const char* id, RendererRef renderer, Rect rect, const char* label, ImageRef image, FontRef font, CreationFlags flags);

		void UpdateButtonSize();
		void CreateLabel();
		LabelPtr m_label;
		bool m_pushed;

		struct shared_enabler;
	};
}