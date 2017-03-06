#pragma once
#include "platform_win.h"
#include <bblib/bbstring.h>
#include <algorithm>

namespace bb {
	struct Gui;
	struct GuiWidget;

	struct GfxWindow
	{
		HWND m_hwnd { nullptr };
		bool m_destroy { false };
		bbstring m_clName { };
		bbstring m_wName { };
		GfxWindow * m_parent { nullptr };
		std::vector<GfxWindow *> m_children;

		GfxWindow () { }
		virtual ~GfxWindow () { }
		virtual void NewFrame () = 0;
		virtual void DrawUI () = 0;
		virtual void Render () = 0;
		virtual bool Done () = 0;

		virtual void Show (bool on) = 0;
		virtual bool Visible () const = 0;

		virtual Gui * GetGui () = 0;
		virtual Gui const * GetGui () const = 0;
		virtual GuiWidget * FindWidget (wchar_t const * widgetId) = 0;

		bbstring const & GetName () const { return m_wName; }
		void SetDestroy (bool destroy) { m_destroy = destroy; }

		bool MoveWindow (int x, int y)
		{
			RECT r;
			::GetWindowRect(m_hwnd, &r);
			{
				int const width = r.right - r.left;
				int const height = r.bottom - r.top;
				::MoveWindow(m_hwnd, x, y, width, height, false);
				return true;
			}
			return false;
		}
		void SetParent (GfxWindow * parent) { m_parent = parent; }
		GfxWindow * GetParent () { return m_parent; }
		GfxWindow const * GetParent () const { return m_parent; }
		bool AddChild (GfxWindow * child) { m_children.push_back(child); return true; }
		bool RemoveChild (GfxWindow * child) { m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end()); return true; }
		GfxWindow * GetRoot ()
		{
			GfxWindow * root = this;
			while (GfxWindow * parent = root->GetParent())
			{
				root = parent;
			}
			return root;
		}
		template<class FnT>
		void ForEach (FnT && fn)
		{
			fn(this);
			for (GfxWindow * ch : m_children)
				ch->ForEach(fn);
		}
		void SetDestroyTree ()
		{
			if (m_parent)
				m_parent->RemoveChild(this);

			this->ForEach(
				[] (bb::GfxWindow * w)
				{
					w->SetDestroy(true);
				});
		}

		void SetDestroyChildren ()
		{
			for (GfxWindow * ch : m_children)
				ch->ForEach(
					[] (bb::GfxWindow * w)
					{
						w->SetDestroy(true);
					});
			m_children.clear();
		}
	};
}
