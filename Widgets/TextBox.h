#pragma once
#include "CoreUI.h"
#include "Core/Widget.h"
#include "Core/Rect.h"
#include "Core/WindowManager.h"
#include <string>
#include <vector>

namespace CoreUI
{
	class DllExport TextBox : public Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(TextBox)

		enum TextBoxEvents : EventCode
		{
			EVENT_TEXTBOX_CHANGED, // Text changed
			EVENT_TEXTBOX_END_EDIT // Enter or Escape key in single line control (data2: 1 for enter, 0 for escape)
		};

		virtual ~TextBox() = default;
		TextBox(const TextBox&) = delete;
		TextBox& operator=(const TextBox&) = delete;
		TextBox(TextBox&&) = delete;
		TextBox& operator=(TextBox&&) = delete;

		void Init() override;

		// Single line text box at 'rect' position
		static TextBoxPtr CreateSingleLine(const char* id, RendererRef renderer, Rect rect, const char* text);

		// Will fill the parent window
		static TextBoxPtr CreateFill(const char* id, RendererRef renderer, const char* text);

		bool HandleEvent(SDL_Event *) override;
		HitResult HitTest(const PointRef) override;
		void Draw() override;

		void SetText(const char *) override;
		std::string GetText() const override;

		void MoveCursor(int x, int y);
		void MoveCursorRel(int16_t deltaX, int16_t deltaY);
		void Insert(const char * text);
		void InsertAt(const char * text, size_t line = std::numeric_limits<size_t>::max(), size_t col = std::numeric_limits<size_t>::max());
		void InsertLine(const char * text = nullptr, size_t at = std::numeric_limits<size_t>::max());
		void DeleteLine(size_t at);
		void Backspace();
		void Delete();
		void Return();
		void MovePage(int16_t deltaY);

		Point CursorAt(PointRef);

	protected:
		struct TextLine
		{
			TextLine() = default;
			TextLine(const char * str) : text(str ? str : "") {}
			TextLine(std::string str) : text(str) {}
			const bool operator==(const std::string& rhs) const { return text == rhs; }
			const bool operator!=(const std::string& rhs) const { return text != rhs; }

			std::string text;
			TexturePtr texture;
			Rect rect;
		};
		using TextLines = std::vector<TextLine>;

		TextBox(const char* id, RendererRef renderer, Rect rect, const char* text, CreationFlags flags);

		void RenderText();
		void SplitLines();
		void RenderLines();
		void DrawText(RectRef rect);
		Rect DrawFrame(const CoreUI::RectRef &rect);
		void DrawBackground(const CoreUI::RectRef &rect);
		void DrawCursor(RectRef rect);

		int FindStringPos(const std::string & in, int pos, int len, int fraction);
		int GetLineWidth(); // Single line mode

		void ScrollCursorIntoView();
		void ScrollX(int fieldWidth, int16_t offset);

		TextLines m_lines;
		int m_lineHeight;
		int m_charWidth;
		Rect m_textRect;

		// Single line mode
		int m_xOffset;
		int m_lineWidth;

		Point m_currentPos;
		Point m_caretPos;

		static Uint32 m_blinkTimerID;
		bool m_blink;

		struct shared_enabler;
	};
}