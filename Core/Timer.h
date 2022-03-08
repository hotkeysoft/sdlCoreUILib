#pragma once
#include "CoreUI.h"

namespace CoreUI
{
	class Widget;

	class Timer
	{
	public:
		DECLARE_EVENT_CLASS_NAME(Timer)

		Timer(bool oneShot = false, Widget* owner = nullptr) :
			m_oneShot(oneShot),
			m_widget(owner)
		{
		}

		Widget* GetWidget() const { return m_widget; }
		bool IsOneShot() const { return m_oneShot; }

		Uint32 GetID() const { return m_id; }
		void SetID(Uint32 id) { m_id = id; }

	private:
		Uint32 m_id = Uint32(-1);
		Widget* m_widget = nullptr;
		bool m_oneShot = false;
	};
}

