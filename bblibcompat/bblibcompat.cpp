#include "bblibcompat.h"
#include <crazyrc/crazyrc.h>

int Settings_snapThreshold = 7;
int Settings_snapPadding = 2;
bool Settings_snapPlugins = true;



inline void _CopyOffsetRect(RECT * dst, RECT const * src, int dx, int dy)
{
	dst->left = src->left + dx;
	dst->right = src->right + dx;
	dst->top = src->top + dy;
	dst->bottom = src->bottom + dy;
}
inline void _OffsetRect(RECT * lprc, int dx, int dy)
{
	lprc->left += dx;
	lprc->right += dx;
	lprc->top += dy;
	lprc->bottom += dy;
}

int BBDrawTextAltW(HDC hDC, LPCWSTR lpString, int nCount, RECT *lpRect, unsigned uFormat, StyleItem* si)
{
	bool const bShadow = (si->validated & V_SHADOWCOLOR) && (si->ShadowColor != (CLR_INVALID));
	if (bShadow)
	{
		// draw shadow
		int const x = si->ShadowX;
		int const y = si->ShadowY;
		SetTextColor(hDC, si->ShadowColor);
		if (si->FontShadow)
		{
			// draw shadow with outline
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					if (!((i | j) & 0x2)) continue;
					RECT rcShadow;
					_CopyOffsetRect(&rcShadow, lpRect, i, j);
					DrawTextW(hDC, lpString, -1, &rcShadow, uFormat);
				}
			}
		}
		else
		{
			RECT rcShadow;
			_CopyOffsetRect(&rcShadow, lpRect, x, y);
			DrawTextW(hDC, lpString, -1, &rcShadow, uFormat);
		}
	}

	bool const bOutline = (si->validated & V_OUTLINECOLOR) && (si->OutlineColor != (CLR_INVALID));
	if (bOutline)
	{
		// draw outline
		SetTextColor(hDC, si->OutlineColor);
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (!(i | j)) continue;
				RECT rcOutline;
				_CopyOffsetRect(&rcOutline, lpRect, i, j);
				DrawTextW(hDC, lpString, -1, &rcOutline, uFormat);
			}
		}
	}
	// draw text
	SetTextColor(hDC, si->TextColor);
	return DrawTextW(hDC, lpString, -1, lpRect, uFormat);
}



// Structures
struct edges { int from1, from2, to1, to2, dmin, omin, d, o, def; };

struct snap_info {
	struct edges *h; struct edges *v;
	bool sizing; bool same_level; int pad; HWND self; HWND parent;
};

// Local fuctions
//ST void snap_to_grid(struct edges *h, struct edges *v, bool sizing, int grid, int pad);
void snap_to_edge(struct edges *h, struct edges *v, bool sizing, bool same_level, int pad);
BOOL CALLBACK SnapEnumProc(HWND hwnd, LPARAM lParam);

void SnapWindowToEdge(WINDOWPOS* wp, LPARAM nDist, UINT flags, ...)
{
	struct edges h;
	struct edges v;
	struct snap_info si;
	int snapdist, padding;
	bool snap_plugins, sizing, snap_workarea;
	HWND self, parent;
	RECT r;
	va_list va;
	//int grid = 0;

	va_start(va, flags);
	snapdist = Settings_snapThreshold;
	padding = Settings_snapPadding;
	snap_plugins = Settings_snapPlugins;
	sizing = 0 != (flags & SNAP_SIZING);
	snap_workarea = 0 == (flags & SNAP_FULLSCREEN);
	if (flags & SNAP_NOPLUGINS)
		snap_plugins = false;
	if (flags & SNAP_TRIGGER)
		snapdist = nDist;
	if (flags & SNAP_PADDING)
		padding = va_arg(va, int);
	if (snapdist < 1)
		return;

	self = wp->hwnd;
	parent = NULL;

	if (WS_CHILD & GetWindowLongPtr(self, GWL_STYLE))
		parent = GetParent(self);

	// ------------------------------------------------------
	// well, why is this here? Because some plugins call this
	// even if they reposition themselves rather than being
	// moved by the user.
	{
		static bool capture;
		if (GetCapture() == self)
			capture = true;
		else if (capture)
			capture = false;
		else
			return;
	}

	// ------------------------------------------------------

	si.h = &h, si.v = &v, si.sizing = sizing, si.same_level = true,
		si.pad = padding, si.self = self, si.parent = parent;

	h.dmin = v.dmin = h.def = v.def = snapdist;
	h.from1 = wp->x;
	h.from2 = h.from1 + wp->cx;
	v.from1 = wp->y;
	v.from2 = v.from1 + wp->cy;

	// ------------------------------------------------------
	// snap to grid

	/*if (grid > 1 && (parent || sizing))
	{
	snap_to_grid(&h, &v, sizing, grid, padding);
	}*/
	//else
	{
		// -----------------------------------------
		if (parent) {

			// snap to siblings
			EnumChildWindows(parent, SnapEnumProc, (LPARAM)&si);

			if (0 == (flags & SNAP_NOPARENT)) {
				// snap to frame edges
				GetClientRect(parent, &r);
				h.to1 = r.left;
				h.to2 = r.right;
				v.to1 = r.top;
				v.to2 = r.bottom;
				snap_to_edge(&h, &v, sizing, false, padding);
			}
		}
		else {

			// snap to top level windows
			if (snap_plugins)
				EnumThreadWindows(GetCurrentThreadId(), SnapEnumProc, (LPARAM)&si);

			// snap to screen edges
			GetMonitorRect(self, &r, snap_workarea ?
				GETMON_WORKAREA | GETMON_FROM_WINDOW : GETMON_FROM_WINDOW);
			h.to1 = r.left;
			h.to2 = r.right;
			v.to1 = r.top;
			v.to2 = r.bottom;
			snap_to_edge(&h, &v, sizing, false, 0);
		}

		// -----------------------------------------
		if (sizing) {
			if (flags & SNAP_CONTENT) { // snap to button icons
				SIZE * psize = va_arg(va, SIZE*);
				h.to2 = (h.to1 = h.from1) + psize->cx;
				v.to2 = (v.to1 = v.from1) + psize->cy;
				snap_to_edge(&h, &v, sizing, false, -2 * padding);
			}

			if (0 == (flags & SNAP_NOCHILDS)) { // snap frame to childs
				si.same_level = false;
				si.pad = -padding;
				si.self = NULL;
				si.parent = self;
				EnumChildWindows(self, SnapEnumProc, (LPARAM)&si);
			}
		}
	}

	// -----------------------------------------
	// adjust the window-pos

	if (h.dmin < snapdist) {
		if (sizing)
			wp->cx += h.omin;
		else
			wp->x += h.omin;
	}

	if (v.dmin < snapdist) {
		if (sizing)
			wp->cy += v.omin;
		else
			wp->y += v.omin;
	}
}

//*****************************************************************************

// this is used in SnapWindowToEdge to avoid plugins snap to menus.
bool Menu_IsA (HWND hwnd)
{
	return false;
	//return (LONG_PTR)Menu::WindowProc == GetWindowLongPtr(hwnd, GWLP_WNDPROC);
}

static BOOL CALLBACK SnapEnumProc(HWND hwnd, LPARAM lParam)
{
	struct snap_info *si = (struct snap_info *)lParam;
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

	if (hwnd != si->self && (style & WS_VISIBLE))
	{
		HWND pw = (style & WS_CHILD) ? GetParent(hwnd) : NULL;
		if (pw == si->parent && false == Menu_IsA(hwnd))
		{
			RECT r;

			GetWindowRect(hwnd, &r);
			r.right -= r.left;
			r.bottom -= r.top;

			if (pw)
				ScreenToClient(pw, (POINT*)&r.left);

			if (false == si->same_level)
			{
				r.left += si->h->from1;
				r.top += si->v->from1;
			}
			si->h->to2 = (si->h->to1 = r.left) + r.right;
			si->v->to2 = (si->v->to1 = r.top) + r.bottom;
			snap_to_edge(si->h, si->v, si->sizing, si->same_level, si->pad);
		}
	}
	return TRUE;
}

//*****************************************************************************
/*
ST void snap_to_grid(struct edges *h, struct edges *v, bool sizing, int grid, int pad)
{
for (struct edges *g = h;;g = v)
{
int o, d;
if (sizing) o = g->from2 - g->from1 + pad; // relative to topleft
else        o = g->from1 - pad; // absolute coords

o = o % grid;
if (o < 0) o += grid;

if (o >= grid / 2)
d = o = grid-o;
else
d = o, o = -o;

if (d < g->dmin) g->dmin = d, g->omin = o;

if (g == v) break;
}
}
*/
//*****************************************************************************
static void snap_to_edge(
struct edges *h,
struct edges *v,
	bool sizing,
	bool same_level,
	int pad)
{
	int o, d, n; struct edges *t;
	h->d = h->def; v->d = v->def;
	for (n = 2;;) // v- and h-edge
	{
		// see if there is any common edge,
		// i.e if the lower top is above the upper bottom.
		if ((v->to2 < v->from2 ? v->to2 : v->from2)
			>= (v->to1 > v->from1 ? v->to1 : v->from1))
		{
			if (same_level) // child to child
			{
				//snap to the opposite edge, with some padding between
				bool f = false;

				d = o = (h->to2 + pad) - h->from1;  // left edge
				if (d < 0) d = -d;
				if (d <= h->d)
				{
					if (false == sizing)
						if (d < h->d) h->d = d, h->o = o;
					if (d < h->def) f = true;
				}

				d = o = h->to1 - (h->from2 + pad); // right edge
				if (d < 0) d = -d;
				if (d <= h->d)
				{
					if (d < h->d) h->d = d, h->o = o;
					if (d < h->def) f = true;
				}

				if (f)
				{
					// if it's near, snap to the corner
					if (false == sizing)
					{
						d = o = v->to1 - v->from1;  // top corner
						if (d < 0) d = -d;
						if (d < v->d) v->d = d, v->o = o;
					}
					d = o = v->to2 - v->from2;  // bottom corner
					if (d < 0) d = -d;
					if (d < v->d) v->d = d, v->o = o;
				}
			}
			else // child to frame
			{
				//snap to the same edge, with some bevel between
				if (false == sizing)
				{
					d = o = h->to1 - (h->from1 - pad); // left edge
					if (d < 0) d = -d;
					if (d < h->d) h->d = d, h->o = o;
				}
				d = o = h->to2 - (h->from2 + pad); // right edge
				if (d < 0) d = -d;
				if (d < h->d) h->d = d, h->o = o;
			}
		}
		if (0 == --n) break;
		t = h; h = v, v = t;
	}

	if (false == sizing)// && false == same_level)
	{
		// snap to center
		for (n = 2;;) // v- and h-edge
		{
			if (v->d < v->dmin)
			{
				d = o = (h->to1 + h->to2) / 2 - (h->from1 + h->from2) / 2;
				if (d < 0) d = -d;
				if (d < h->d) h->d = d, h->o = o;
			}
			if (0 == --n) break;
			t = h; h = v, v = t;
		}
	}

	if (h->d < h->dmin) h->dmin = h->d, h->omin = h->o;
	if (v->d < v->dmin) v->dmin = v->d, v->omin = v->o;
}



static void get_mon_rect (HMONITOR hMon, RECT *s, RECT *w)
{
	if (hMon)
	{
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo(hMon, &mi)) {
			if (w)
				*w = mi.rcWork;
			if (s)
				*s = mi.rcMonitor;
			return;
		}
	}
	if (w) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, w, 0);
	}
	if (s) {
		s->top = s->left = 0;
		s->right = GetSystemMetrics(SM_CXSCREEN);
		s->bottom = GetSystemMetrics(SM_CYSCREEN);
	}
}

HMONITOR GetMonitorRect (void *from, RECT *r, int flags)
{
	HMONITOR hMon = NULL;
	if (from)
	{
		switch (flags & ~GETMON_WORKAREA) {
		case GETMON_FROM_WINDOW:
			hMon = MonitorFromWindow((HWND)from, MONITOR_DEFAULTTONEAREST);
			break;
		case GETMON_FROM_POINT:
			hMon = MonitorFromPoint(*(POINT*)from, MONITOR_DEFAULTTONEAREST);
			break;
		case GETMON_FROM_MONITOR:
			hMon = (HMONITOR)from;
			break;
		}
	}
	if (flags & GETMON_WORKAREA)
		get_mon_rect(hMon, NULL, r);
	else
		get_mon_rect(hMon, r, NULL);

	return hMon;
}




bool SetTransparency(HWND hwnd, BYTE alpha)
{
	//dbg_window(hwnd, "alpha %d", alpha);
	LONG_PTR wStyle1 = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	LONG_PTR wStyle2 = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	//BYTE Alpha = eightScale_up(alpha); // no eightscale up
	BYTE Alpha = alpha;
	if (Alpha < 255)
		wStyle2 |= WS_EX_LAYERED;
	else
		wStyle2 &= ~WS_EX_LAYERED;

	if (wStyle2 != wStyle1)
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, wStyle2);

	if (wStyle2 & WS_EX_LAYERED)
		return 0 != SetLayeredWindowAttributes(hwnd, 0, Alpha, LWA_ALPHA);

	return true;
}

#include "bbrc.h"
//===========================================================================
// API: ReadBool
//===========================================================================

bool ReadBool(const TCHAR * fileName, const TCHAR * szKey, bool bDefault)
{
	bool result = bDefault;
	rc::readBool(fileName, szKey, bDefault, result);
	return result;
// 	const TCHAR * szValue = read_value(fileName, szKey, NULL);
// 	if (szValue)
// 	{
// 		if (!_tcsicmp(szValue, TEXT("true")))
// 			return true;
// 		if (!_tcsicmp(szValue, TEXT("false")))
// 			return false;
// 	}
// 	return bDefault;
}

//===========================================================================
// API: ReadInt
//===========================================================================

int ReadInt (const TCHAR * fileName, const TCHAR * szKey, int nDefault)
{
// 	const TCHAR * szValue = read_value(fileName, szKey, NULL);
// 	return szValue ? _ttoi(szValue) : nDefault;

	int result = nDefault;
	rc::readInt(fileName, szKey, nDefault, result);
	return result;

}

//===========================================================================
// API: ReadString
//===========================================================================

const TCHAR * ReadString(const TCHAR * fileName, const TCHAR * szKey, const TCHAR * szDefault)
{
	const TCHAR * szValue = read_value(fileName, szKey, NULL);
	return szValue ? szValue : szDefault;
}

//===========================================================================
// API: ReadColor
//===========================================================================

// COLORREF ReadColor(const char* fileName, const char* szKey, const char* defaultColor)
// {
// 	const char* szValue = szKey[0] ? read_value(fileName, szKey, NULL) : NULL;
// 	return ReadColorFromString(szValue ? szValue : defaultColor);
// }

//===========================================================================
// API: WriteBool
//===========================================================================

void WriteBool(const TCHAR * fileName, const TCHAR * szKey, bool value)
{
	write_value(fileName, szKey, value ? TEXT("true") : TEXT("false"));
}

//===========================================================================
// API: WriteInt
//===========================================================================

void WriteInt(const TCHAR * fileName, const TCHAR * szKey, int value)
{
	TCHAR buff[32];
	write_value(fileName, szKey, _itot(value, buff, 10));
}

//===========================================================================
// API: WriteString
//===========================================================================

void WriteString (const TCHAR * fileName, const TCHAR * szKey, const TCHAR * value)
{
	write_value(fileName, szKey, value);
}

//===========================================================================
// API: WriteColor
//===========================================================================

void WriteColor (const TCHAR * fileName, const TCHAR * szKey, COLORREF value)
{
	//assert(0); // @TODO
// 	char buff[32];
// 	sprintf(buff, "#%06lx", (unsigned long)switch_rgb(value));
// 	write_value(fileName, szKey, buff);
}

//===========================================================================
// API: ReadValue
const wchar_t * ReadValue (const wchar_t * path, const wchar_t * szKey, long * ptr)
{
	return read_value(path, szKey, ptr);
}
