#pragma once
#include "CoreUI.h"
#include "Color.h"
#include "Point.h"
#include "Rect.h"
#include "ResourceManager.h"
#include <string>
#include <ostream>

#define STRINGIZE( a ) #a
#define DECLARE_EVENT_CLASS_NAME(className)			\
    static const char* EventClassName() { return STRINGIZE(className); }	\
    virtual const char* GetClassName() { return EventClassName(); }

namespace CoreUI
{
	struct HitResult
	{
		HitResult(HitZone z = HIT_NOTHING) : zone(z), target(nullptr) {}
		HitResult(HitZone z, WidgetRef t) : zone(z), target(t) {}

		bool operator==(const HitResult& rhs) const { return zone == rhs.zone && target == rhs.target; }
		bool operator==(const HitZone& rhs) const { return zone == rhs; }
		operator HitZone() const { return zone; }
		explicit operator bool() const { return zone != HIT_NOTHING; }

		HitZone zone;
		WidgetRef target;
	};

	union WidgetTag
	{
		int i;
		void* o;
	};

	class DllExport Widget
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Widget)

		virtual ~Widget() = default;
		Widget(const Widget&) = delete;
		Widget& operator=(const Widget&) = delete;
		Widget(Widget&&) = delete;
		Widget& operator=(Widget&&) = delete;

		virtual const std::string &GetId() const { return m_id; }

		virtual void Init() {};
				
		CreationFlags GetFlags() { return m_flags; }

		// Activation
		virtual void SetActive() { if (m_parent) m_parent->SetActive(); }
		virtual void SetFocus(WidgetRef focus, WidgetRef parent= nullptr);
		virtual void ClearFocus();
		void SetFocus() { SetFocus(this); }
		bool IsFocused() { return m_focused; }

		virtual std::string GetText() const { return m_text; }
		virtual void SetText(const char *text) { m_text = text ? text : ""; }

		virtual ImageRef GetImage() const { return m_image; }
		virtual void SetImage(ImageRef image) { m_image = image;  }

		virtual Rect GetClientRect(bool relative = true, bool scrolled = true) const;
		virtual Rect GetRect(bool relative = true, bool scrolled = true) const;
		virtual void SetRect(RectRef rect) { m_rect = rect?(*rect):Rect(); }

		// Margins & Padding
		virtual Dimension GetMargin() { return m_margin; }
		virtual void SetMargin(Dimension margin) { m_margin = margin; }

		virtual Dimension GetPadding() { return m_padding; }
		virtual void SetPadding(Dimension padding) { m_padding = padding; }

		// Font
		virtual const FontRef GetFont() const { return m_font; }
		virtual void SetFont(FontRef font);

		// Borders
		virtual Dimension GetShrinkFactor() const { return m_padding + m_margin + Dimension(m_showBorder ? m_borderWidth : 0); }

		virtual void SetBorder(bool show) { m_showBorder = show; }
		virtual bool GetBorder() { return m_showBorder; }
		
		virtual Color GetBorderColor() { return m_borderColor; }
		virtual void SetBorderColor(Color color) { m_borderColor = std::move(color); }

		virtual uint8_t GetBorderWidth() { return m_borderWidth; }
		virtual void SetBorderWidth(uint8_t width) { m_borderWidth = width; }

		// Colors
		virtual Color GetForegroundColor() { return m_foregroundColor; }
		virtual void SetForegroundColor(Color color) { m_foregroundColor = std::move(color); }

		virtual Color GetBackgroundColor() { return m_backgroundColor; }
		virtual void SetBackgroundColor(Color color) { m_backgroundColor = std::move(color); }

		virtual Color GetSelectedFgColor() { return m_selectedFgColor; }
		virtual void SetSelectedFgColor(Color color) { m_selectedFgColor = std::move(color); }

		virtual Color GetSelectedBgColor() { return m_selectedBgColor; }
		virtual void SetSelectedBgColor(Color color) { m_selectedBgColor = std::move(color); }

		// Tooltip
		virtual std::string GetTooltip() const { return m_tooltip; }
		virtual void SetTooltip(const char* str = nullptr) { m_tooltip = str ? str : ""; }

		// Size & Position
		virtual Dimension GetMinSize() { return m_minSize; }
		virtual void SetMinSize(Dimension minSize) { m_minSize = minSize; }

		virtual bool MoveRel(PointRef rel);
		virtual bool MovePos(PointRef pos);
		virtual bool MoveRect(RectRef rect);
		virtual bool ResizeRel(PointRef rel);
		virtual bool Resize(PointRef size);

		// Parent
		virtual bool HasParent() const { return m_parent != nullptr; }
		virtual const WidgetRef GetParent() const { return m_parent; }
		virtual const WindowRef GetParentWindow() const { return m_parentWindow; }
		virtual void SetParent(WidgetRef parent);

		// Events
		virtual bool HandleEvent(SDL_Event*);
		virtual HitResult HitTest(const PointRef) { return HitZone::HIT_NOTHING; }

		virtual PointRef GetScrollPos() { return &m_scrollPos; }

		virtual void Draw() = 0;

		virtual std::string ToString() const { return ""; }

		virtual WidgetTag GetTag() const { return m_tag; }
		virtual void SetTag(int tag) { m_tag.i = tag; }
		virtual void SetTag(void * tag) { m_tag.o = tag; }

	protected:
		Widget(const char* id, RendererRef renderer, WidgetRef parent, Rect rect,
			const char* text, ImageRef image = nullptr, FontRef font = nullptr, CreationFlags flags = 0);
		Widget(const char* id);

		Uint32 GetEventClassId();

		// Helper to generate widget id based on parent in the form: "{parentId}-id"
		std::string MakeChildWidgetId(const char* id) const;

		void PostEvent(EventCode code, void * data2 = nullptr);

		void SetDrawColor(const CoreUI::Color & col);
		void DrawFilledRect(const RectRef pos, const CoreUI::Color & col);
		void DrawRect(const RectRef pos, const CoreUI::Color & col, int borderWidth = 1);
		void DrawButton(const RectRef pos, const CoreUI::Color & col, ImageRef image, bool raised, int thickness = 1);
		void Draw3dFrame(const RectRef pos, bool raised, const CoreUI::Color & col = Color::C_LIGHT_GREY);
		void DrawReliefBox(const RectRef pos, const CoreUI::Color & col, bool raised);

		TexturePtr SurfaceToTexture(SDL_Surface* surf, bool writable = false);
		TexturePtr CloneTexture(TexturePtr source, Color background = Color::C_TRANSPARENT);

		void UpdateParentWindow();

		const std::string m_id;
		Uint32 m_eventClassId;
		CreationFlags m_flags;
		RendererRef m_renderer;
		std::string m_text;
		ImageRef m_image;
		WidgetRef m_parent;
		WindowRef m_parentWindow;
		Rect m_rect;
		FontRef m_font;
		Point m_scrollPos;
		bool m_focused;
		WidgetTag m_tag;

		// Tooltips
		std::string m_tooltip;
		Uint32 m_tooltipTimer = (Uint32)-1;

		// Borders
		bool m_showBorder;
		Color m_borderColor;

		// Colors
		Color m_backgroundColor;
		Color m_foregroundColor;
		Color m_selectedFgColor;
		Color m_selectedBgColor;

		Dimension m_padding;
		Dimension m_margin;
		Dimension m_minSize;
		uint8_t m_borderWidth;

		static uint8_t constexpr m_buttonSize = 24;
	};

	inline std::ostream & operator << (std::ostream & os, const Widget& widget)
	{
		os << widget.ToString();
		return os;
	}
}