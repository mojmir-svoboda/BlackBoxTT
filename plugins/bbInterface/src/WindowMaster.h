#pragma once
#include <blackbox/BlackBox_compat.h>

struct module; class Menu; struct agent; struct control; struct window; struct StyleItem;

//Define these structures
struct window
{
	HWND hwnd;
	HBITMAP bitmap;

	control *controlptr;

	int x;
	int y;
	int width;
	int height;

	int style;
	StyleItem *styleptr;
	bool has_custom_style;
	
	bool use_custom_font;
	HFONT font;
	wchar_t Fontname[128];
	int FontHeight;
	int FontWeight;

	bool is_bordered;

	bool is_transparent;
	bool is_visible;
	bool is_ontop;
	bool is_slitted;
	bool is_toggledwithplugins;
	bool is_onallworkspaces;
	bool is_detectfullscreen;

	bool is_moving;
	bool is_sizing;
	bool is_autohidden;
	
	bool autohide;
	bool useslit;
	int transparency;
	int makeinvisible;
	int nosavevalue;
	int workspacenumber;

	bool is_button;
	struct ButtonStyleInfo *bstyleptr;
	bool is_slider;
	struct SliderStyleInfo *sstyleptr;
};

struct ButtonStyleInfo
{
	int style;
	StyleItem *styleptr;
	bool has_custom_style;
	bool use_custom_font;
	HFONT font;
	wchar_t Fontname[128];
	int FontHeight;
	int FontWeight;
	int nosavevalue;
};

struct SliderStyleInfo
{
	//bar style
	int style;
	StyleItem *styleptr; // Color,ColorTo,bevel,bevelpos
	bool has_custom_style;

	//inner style
	bool draw_inner;
	int in_style;
	StyleItem *in_styleptr; // Color,ColorTo,bevel,bevelpos
	bool in_has_custom_style;	
};

//Define these functions internally
int window_startup();
int window_shutdown();

int window_create(control *c);
int window_destroy(window **pw);

void window_menu_context(std::shared_ptr<bb::MenuConfig> m, control *c);

void window_save_control(control *c);
void window_save();

LRESULT CALLBACK window_event(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int window_message(int tokencount, wchar_t *tokens[], bool from_core, module* caller);
int window_message_setproperty(control *c, int tokencount, wchar_t *tokens[]);
void style_set_customvalue(StyleItem *styleptr, int style, int nosavevalue);
void reconfigure_customvalue(control *c);

void window_pluginsvisible(bool isvisible);
void window_update(window *w, bool position, bool transparency, bool visibility, bool sticky);

//int window_helper_register(const wchar_t *classname, LRESULT CALLBACK (*callbackfunc)(HWND, UINT, WPARAM, LPARAM));
int window_helper_register(const wchar_t *classname, WNDPROC callbackfunc);
int window_helper_unregister(const wchar_t *classname);
HWND window_helper_create(const wchar_t *classname);
int window_helper_destroy(HWND hwnd);
void window_make_child(window *w, window *pw);

//Global variables
extern wchar_t szWPx   [];
extern wchar_t szWPy   [];
extern wchar_t szWPwidth   [];
extern wchar_t szWPheight  [];
extern wchar_t szWPtransparency    [];
extern wchar_t szWPisvisible   [];
extern wchar_t szWPisontop [];
extern wchar_t szWPissnappy    [];
extern wchar_t szWPisslitted   [];
extern wchar_t szWPistransparent   [];
extern wchar_t szWPistoggledwithplugins    [];
extern wchar_t szWPisonallworkspaces   [];

