#pragma once
#include "Common.h"
#include "Core\Widget.h"
#include "Core\Rect.h"
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
	enum LabelCreationFlags : CreationFlags
	{
		LCF_MENUITEM = WIN_CUSTOMBASE << 0, // Will replace '&' with underlined characted
	};

	class DllExport Label : public Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Label)

		enum TextAlign : uint8_t {
			TEXT_H_LEFT = 1,
			TEXT_H_RIGHT = 2,
			TEXT_H_CENTER = TEXT_H_LEFT | TEXT_H_RIGHT,

			TEXT_V_TOP = 8,
			TEXT_V_BOTTOM = 16,
			TEXT_V_CENTER = TEXT_V_TOP | TEXT_V_BOTTOM,

			TEXT_FILL_DEFAULT = TEXT_H_CENTER | TEXT_V_CENTER,
			TEXT_SINGLE_DEFAULT = TEXT_H_LEFT | TEXT_V_TOP,
			TEXT_AUTOSIZE_DEFAULT = TEXT_H_LEFT | TEXT_V_TOP,
		};

		virtual ~Label() = default;
		Label(const Label&) = delete;
		Label& operator=(const Label&) = delete;
		Label(Label&&) = delete;
		Label& operator=(Label&&) = delete;

		void Init() override;

		// Single line label at position 'rect'
		static LabelPtr CreateSingle(const char* id, RendererRef renderer, Rect rect, const char* label, FontRef font = nullptr, TextAlign align = TEXT_SINGLE_DEFAULT, CreationFlags flags = 0);

		// Fills whole parent window
		static LabelPtr CreateFill(const char* id, RendererRef renderer, const char* label, FontRef font = nullptr, TextAlign align = TEXT_FILL_DEFAULT, CreationFlags flags = 0);

		// Auto-size, draw at desired position with Draw(Rect)
		static LabelPtr CreateAutoSize(const char* id, RendererRef renderer, const char* label, FontRef font = nullptr, TextAlign align = TEXT_AUTOSIZE_DEFAULT, CreationFlags flags = 0);

		void SetForegroundColor(Color color) override;

		void Draw() override;
		void Draw(const RectRef rect, bool noClip = false);

		void SetText(const char *) override;

		void SetAlign(uint8_t align);
		uint8_t GetAlign() { return m_labelAlign;  }

	protected:
		Label(const char* id, RendererRef renderer, Rect rect, const char* label, FontRef font, TextAlign align, CreationFlags flags);

		void DrawBackground(const CoreUI::RectRef &rect);
		Rect DrawFrame(const CoreUI::RectRef &rect);
		void RenderLabel();
		void DrawUnderline(const size_t &underlinePos);
		void DrawLabel(RectRef rect);

		std::string RemoveAmpersands(size_t & underlinePos); // Returns updated string, position of first ampersand in underlinePos, -1 if not found

		TexturePtr m_labelText;
		Rect m_labelRect;
		TextAlign m_labelAlign;

		struct shared_enabler;
	};
}