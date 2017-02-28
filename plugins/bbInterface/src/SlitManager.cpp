/*===================================================

	SLIT MANAGER CODE - Copyright 2004 grischka

	- grischka@users.sourceforge.net -

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <bblibcompat/winutils.h>

#include "ControlType_Label.h"
#include "SlitManager.h"
#include "SnapWindow.h"
#include "StyleMaster.h"

//=============================================================================

void dbg_window (HWND window, wchar_t *msg)
{
	wchar_t buffer[256];
	GetClassName(window, buffer, 256);
	dbg_printf(L"%s %s", msg, buffer);
}

#undef dolist
#define dolist(_e,_l) for (_e=_l;_e;_e=_e->next)

//=============================================================================

void set_plugin_position(PluginInfo *p)
{
	 SetWindowPos(p->hwnd, NULL,
		 p->xpos, p->ypos, p->width, p->height,
		 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
}

//=============================================================================
PluginInfo *get_plugin(PluginInfo *PI, char *module_name)
{
	PluginInfo *p;
	int l = strlen(module_name);
	dolist (p, PI)
		if (0 == _memicmp(module_name, p->module_name, l)
			&& (0 == p->module_name[l] || '.' == p->module_name[l]))
			break;
	return p;
}

//=============================================================================
bool plugin_setpos(PluginInfo *PI, char *module_name, int x, int y)
{
	PluginInfo *p = get_plugin(PI, module_name);
	if (NULL == p) return false;
	p->xpos = x;
	p->ypos = y;
	set_plugin_position(p);
	return true;
}

//=============================================================================
// well, for instance BBIcons has more than one window,
// so we hide/show all of them

bool plugin_getset_show_state(PluginInfo *PI, char *module_name, int state)
{
	PluginInfo *p = PI;
	bool result = false;
	bool show = state > 0;
	for (;;)
	{
		p = get_plugin(p, module_name);
		if (NULL == p) return 3 == state;

		if (false == result && state >= 2)
			show = !(WS_VISIBLE & GetWindowLongPtr(p->hwnd, GWL_STYLE));

		if (3 == state) return !show;

		ShowWindow(p->hwnd, show ? SW_SHOWNOACTIVATE : SW_HIDE);
		result = true;
		p = p->next;
	}
}

//=============================================================================

void get_sizes(PluginInfo **pp, HWND check)
{
	PluginInfo *p;
	while (NULL != (p = *pp))
	{
		HWND hwnd = p->hwnd;
		if (NULL == check || hwnd == check)
		{
			if (FALSE == IsWindow(hwnd))
			{
				*pp = p->next;
				delete p;
				continue;
			}
			RECT r; GetWindowRect(hwnd, &r);
			p->width  = r.right - r.left;
			p->height = r.bottom - r.top;
		}
		pp = &p->next;
	}
}

//=============================================================================

LRESULT CALLBACK subclass_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool is_locked_frame(control *c);
	extern bool plugin_snapwindows;

	HWND hSlit = GetParent(hwnd);
	control *c = (control *)GetWindowLongPtr(hSlit, 0);
	controltype_label_details *details = (controltype_label_details *) c->controldetails;
	PluginInfo *PI = details->plugin_info;

	PluginInfo *p;
	dolist (p, PI)
		if (p->hwnd == hwnd) goto found;

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

found:
	switch (uMsg)
	{
		case WM_ENTERSIZEMOVE:
			p->is_moving = true;
			break;

		case WM_EXITSIZEMOVE:
			p->is_moving = false;
			break;

		case WM_WINDOWPOSCHANGING:
			if (p->is_moving && false == is_locked_frame(c))
			{
				if (plugin_snapwindows && false == (GetAsyncKeyState(VK_SHIFT) & 0x8000))
					snap_windows((WINDOWPOS*)lParam, false, NULL);
			}
			else
			{
				WINDOWPOS* wp = (WINDOWPOS*)lParam;
				wp->x = p->xpos, wp->y = p->ypos;
			}
			return 0;

		case WM_WINDOWPOSCHANGED:
			{
				WINDOWPOS* wp = (WINDOWPOS*)lParam;
				if (0 == (wp->flags & SWP_NOMOVE))
				{
					p->xpos = wp->x;
					p->ypos = wp->y;
				}
				if (0 == (wp->flags & SWP_NOSIZE))
				{
					p->width = wp->cx;
					p->height = wp->cy;
				}
				style_check_transparency_workaround(hwnd);
			}
			break;

		case WM_NCLBUTTONDOWN:
			if (0x8000 & GetAsyncKeyState(VK_CONTROL))
			{
				SetWindowPos(hwnd, HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOSENDCHANGING);
				UpdateWindow(hwnd);
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
			break;

		case WM_NCHITTEST:
			if (0x8000 & GetAsyncKeyState(VK_SHIFT))
				return HTTRANSPARENT;

			if (0x8000 & GetAsyncKeyState(VK_CONTROL))
				return HTCAPTION;
			break;

		case WM_NCDESTROY:
			SendMessage(hSlit, SLIT_REMOVE, 0, (LPARAM)hwnd);
			break;
	}
	return CallWindowProc(p->wp, hwnd, uMsg, wParam, lParam);
}

//=============================================================================
int plugin_get_displayname(const wchar_t *src, wchar_t *buff)
{
	const wchar_t *s = src;
	const wchar_t *e = s + wcslen(s);
	int len = 0;
	// start after the last slash
	while (e>s && e[-1]!= L'\\' && e[-1]!= L'/')
		e--, len++;
	wcscpy(buff, e);

	// cut off ".dll", if present
	while (len)
		if (buff[--len]== '.')
		{ 
			buff[len] = 0;
			break;
		}
	return len;
}

//=============================================================================
void get_unique_modulename(PluginInfo *PI, HMODULE hMO, wchar_t *buffer, size_t buff_sz)
{
	wchar_t temp[200] = { 0 };
	*buffer = 0;
	wchar_t module_name[200];
	GetModuleFileName(hMO, module_name, sizeof module_name);
	int len = plugin_get_displayname(module_name, temp);

	PluginInfo *p; int n = 1;
	dolist (p, PI)
	{
		if (0 == _wcsicmp(temp, p->module_name)
			&& (0 == p->module_name[len] || L'.' == p->module_name[len]))
		{
			if (1 == n)
				swprintf(p->module_name, 96, L"%s.%d", temp, n);
			n++;
		}
	}
	if (1 == n)
		wcscpy(buffer, temp);
	else
		swprintf(buffer, buff_sz, L"%s.%d", temp, n);

	//dbg_printf("loaded: <%s> <%s>", temp, buffer);
}

//=============================================================================

int SlitWndProc(control *c, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	controltype_label_details *details = (controltype_label_details *) c->controldetails;
	PluginInfo **pp = &details->plugin_info;

	switch(uMsg)
	{
		case SLIT_ADD:
			//dbg_window ((HWND) lParam, "add");
			{
				PluginInfo *p = new PluginInfo;
				ZeroMemory(p, sizeof *p);

				p->hwnd = (HWND)lParam;
				p->hMO = (HMODULE)GetClassLongPtr(p->hwnd, -16 /*GCL_HMODULE*/);

				if (wParam >= 0x400)
					wcscpy(p->module_name, (const wchar_t *)wParam);
				else
					get_unique_modulename(*pp, p->hMO, p->module_name, 96);

				//dbg_printf("module: %x %s",  p->hMO, p->module_name);

				while (*pp) pp = &(*pp)->next;
				*pp = p, p->next = NULL;

				SetParent(p->hwnd, hwnd);
				SetWindowLongPtr(p->hwnd, GWL_STYLE, (GetWindowLongPtr(p->hwnd, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);
				p->wp = (WNDPROC)SetWindowLongPtr(p->hwnd, -4 /*GWL_WNDPROC*/, (LONG_PTR)subclass_WndProc);

				get_sizes(pp, (HWND)lParam);
				set_plugin_position(p);
				break;
			}
			break;

		case SLIT_REMOVE:
			//dbg_window ((HWND) lParam, "remove");
			while (*pp)
			{
				if ((*pp)->hwnd == (HWND)lParam)
				{
					PluginInfo *p = *pp;
					if (IsWindow(p->hwnd))
					{
						SetWindowLongPtr(p->hwnd, -4 /*GWL_WNDPROC*/, (LONG_PTR)p->wp);
						SetParent(p->hwnd, NULL);
						SetWindowLongPtr(p->hwnd, GWL_STYLE, (GetWindowLongPtr(p->hwnd, GWL_STYLE) & ~WS_CHILD) | WS_POPUP);
					}
					*pp = p->next;
					delete p;
					break;
				}
				pp = &(*pp)->next;
			}
			break;

		case SLIT_UPDATE:
			//dbg_window ((HWND) lParam, "update");
			//though, some plugins dont pass their hwnd with SLIT_UPDATE
			get_sizes(pp, (HWND)lParam);
			break;

		//=============================================
		default:
			break;
	}
	return 0;
}

//============================================================================
// load/unloadPlugins

ModuleInfo * m_loadPlugin(HWND hSlit, const wchar_t *file_name)
{
	const char *errormsg = NULL;

	HMODULE hMO = LoadLibrary(file_name);
	if (NULL == hMO)
	{
		errormsg = "The plugin you are trying to load does not exist or cannot be loaded.";
	}
	else
	{
		try
		{
			int result = 0;

			int (*beginPlugin)(HINSTANCE hMainInstance);
			int (*beginSlitPlugin)(HINSTANCE hMainInstance, HWND hBBSlit);
			int (*beginPluginEx)(HINSTANCE hMainInstance, HWND hBBSlit);
			int (*endPlugin)(HINSTANCE hMainInstance);

			*(FARPROC*)&beginPlugin     = GetProcAddress(hMO, "beginPlugin");
			*(FARPROC*)&beginSlitPlugin = GetProcAddress(hMO, "beginSlitPlugin");
			*(FARPROC*)&beginPluginEx   = GetProcAddress(hMO, "beginPluginEx");
			*(FARPROC*)&endPlugin       = GetProcAddress(hMO, "endPlugin");

			if (NULL == endPlugin)
				errormsg = "This plugin doesn't have a 'endPlugin'."
					"\nProbably it is not a plugin designed for bb4win.";
			else
			if (beginSlitPlugin)
				result = beginSlitPlugin(hMO, hSlit);
			else
			if (beginPluginEx)
				result = beginPluginEx(hMO, hSlit);
			else
			if (beginPlugin)
				result = beginPlugin(hMO);
			else
				errormsg = "This plugin doesn't have an 'beginPlugin'.";

			if (NULL == errormsg)
			{
				if (0 == result)
				{
					ModuleInfo *m = new ModuleInfo;
					m->hMO = hMO;
					m->endPlugin = endPlugin;
					*(FARPROC*)&m->pluginInfo = GetProcAddress(hMO, "pluginInfo");
					wcscpy(m->file_name, file_name);
					plugin_get_displayname(file_name, m->module_name);
					return m;
				}
				errormsg = "This plugin signaled an error on loading.";
			}
		}
		catch(...)
		{
			errormsg = "This plugin crashed on loading.";
		}

		FreeLibrary(hMO);
	}

	wchar_t message[MAX_PATH + 512];
	swprintf (message, MAX_PATH + 512, L"%s\n%s", file_name, errormsg);
	MBoxErrorValue(message);
	return NULL;
}

//============================================================================
void m_unloadPlugin(ModuleInfo *m)
{
	m->endPlugin(m->hMO);
	FreeLibrary(m->hMO);
}

//============================================================================
const wchar_t * check_relative_path(const wchar_t *filename)
{
	// get relative path to blackbox process
	wchar_t bb_path[MAX_PATH];
	GetBlackboxPath(bb_path, MAX_PATH);
	int len = wcslen(bb_path);

	if (0 == _memicmp(bb_path, filename, len))
		return filename + len;
	return filename;
}

//============================================================================
ModuleInfo *loadPlugin(ModuleInfo **pm, HWND hSlit, const wchar_t *file_name)
{
	ModuleInfo *m = m_loadPlugin(hSlit, check_relative_path(file_name));
	if (m)
	{
		for (; *pm; pm = &(*pm)->next);
		*pm = m, m->next = NULL;
	}
	return m;
}

//============================================================================
bool unloadPlugin(ModuleInfo **pm, const wchar_t *module_name)
{
	ModuleInfo *m; bool result = false;
	for (; NULL != (m = *pm);)
	{
		if (NULL == module_name || 0 == _wcsicmp(module_name, m->module_name))
		{
			m_unloadPlugin(m);
			*pm = m->next;
			delete m;
			result = true;
		}
		else
		{
			pm = &m->next;
		}
	}
	return result;
}

//=============================================================================

void aboutPlugins(ModuleInfo *m0, const wchar_t *ctrl)
{
	wchar_t buff[5000];
	int x = 0;
	ModuleInfo *m;
	for (m = m0; m; m= m->next)
	{
		x += m->pluginInfo
			? swprintf(buff+x, 5000-x, L"%s %s by %s (%s)\t\n", m->pluginInfo(PLUGIN_NAME), m->pluginInfo(PLUGIN_VERSION), m->pluginInfo(PLUGIN_AUTHOR), m->pluginInfo(PLUGIN_RELEASE))
			: swprintf(buff+x, 5000-x, L"%s\t\n", m->file_name)
			;
	}

	wchar_t caption[256];
	swprintf(caption, 256, L"%s - About Plugins", ctrl);
	MessageBox(NULL, buff, caption, MB_OK|MB_SYSTEMMODAL);
}

//=============================================================================
