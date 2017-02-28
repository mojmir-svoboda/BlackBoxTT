/*===================================================

	CONTROL MASTER HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_ControlMaster_h
#define BBInterface_ControlMaster_h

//Pre-defined structures
struct controltype;
struct control;
struct controlchild;

//Includes
#include "WindowMaster.h"
#include "AgentMaster.h"
#include "ListMaster.h"
#include "ModuleMaster.h"

//Cirular dependency. Whoah. This surely needs some redesign.
struct module;

//Definitions

//Define these structures
struct controltype
{
	wchar_t    controltypename[64];
	bool    can_parentless;
	bool    can_parent;
	bool    can_child;
	wchar_t    id;
	int     (*func_create)(control *c);
	int     (*func_destroy)(control *c);
	LRESULT (*func_event)(control *c, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void    (*func_notify)(control *c, int notifytype, void *messagedata);
	int     (*func_message)(control *c, int tokencount, wchar_t *tokens[]);
	void*   (*func_getdata)(control *c, int datatype);
	bool    (*func_getstringvalue)(control *c, wchar_t *buffer, wchar_t *propertyname);
	void    (*func_menu_context)(std::shared_ptr<bb::MenuConfig> m, control *c);
	void    (*func_notifytype)(int notifytype, void *messagedata);
};

struct control
{
	wchar_t controlname[64];       //UNIQUE name of the control

	controltype *controltypeptr;    //Pointer to the type of control
	control *parentptr;         //Pointer to the parent control
	window *windowptr;              //Pointer to the control's window
	module *moduleptr;			//Pointer to the module the control is associated to

	void *controldetails;       //Pointer to details about the control

	controlchild *firstchild;
	controlchild *lastchild;
	controlchild *mychildnode;
};

struct controlchild
{
	control *controlptr;
	controlchild *nextchildptr;
	controlchild *prevchildptr;
};

enum
{
	CONTROL_ID_FRAME = 1,
	CONTROL_ID_LABEL,
	CONTROL_ID_BUTTON,
	CONTROL_ID_SWITCHBUTTON,
	CONTROL_ID_SLIDER
};


//Define these functions internally
int control_startup();
int control_shutdown();

extern module* currentmodule;

int control_create(wchar_t *controlname, wchar_t *controltypename, wchar_t *controlparentname, bool include_parent, module* parentmodule);
int control_destroy(control *c, bool remove_from_list, bool save_last);
int control_rename(control *c, wchar_t *newname);
bool control_make_childof(control *c, const wchar_t *parentname);
bool control_make_parentless(control *c);
control* control_get(const wchar_t *name, module* caller = currentmodule);


LRESULT control_event(control *c, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int control_message(int tokencount, wchar_t *tokens[], bool from_core, module* caller);
void control_notify(control *c, int notifytype, void *messagedata);

void control_pluginsvisible(bool arevisible);
void control_invalidate(void);

void control_menu_create(std::shared_ptr<bb::MenuConfig> m, control *c, bool createchild);
void control_menu_context(std::shared_ptr<bb::MenuConfig> m, control *c);
void control_menu_settings(std::shared_ptr<bb::MenuConfig> m, control *c);

bool control_getstringdata(control *c, wchar_t *buffer, wchar_t *propertyname);

void control_save();

void control_checklast();

void control_registertype(
	wchar_t    *controltypename,
	bool    can_parentless,
	bool    can_parent,
	bool    can_child,
	wchar_t    id,
	int     (*func_create)(control *c),
	int     (*func_destroy)(control *c),
	LRESULT (*func_event)(control *c, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam),
	void    (*func_notify)(control *c, int notifytype, void *messagedata),
	int     (*func_message)(control *c, int tokencount, wchar_t *tokens[]),
	void*   (*func_getdata)(control *c, int datatype),
	bool    (*func_getstringvalue)(control *c, wchar_t *buffer, wchar_t *propertyname),
	void    (*func_menu_context)(std::shared_ptr<bb::MenuConfig> m, control *c),
	void    (*func_notifytype)(int notifytype, void *messagedata)
	);
void control_unregistertype(controltype *ct);


struct token_check { const wchar_t *key; size_t id; int args; };
int token_check(struct token_check *t, int *curtok, int tokencount, wchar_t *tokens[]);
int get_string_index (const wchar_t *key, const wchar_t **string_list);
wchar_t *new_string(const wchar_t *);
void free_string(wchar_t **s);
wchar_t *extract_string(wchar_t *d, const wchar_t *s, int n);
wchar_t* unquote(wchar_t *d, const wchar_t *s);

void variables_startup(void);
void variables_shutdown(void);
void variables_save(void);
const wchar_t *variables_get(const wchar_t *key, const wchar_t *deflt, module* defmodule);
void variables_set(bool is_static, const wchar_t *key, const wchar_t *val, module* defmodule = currentmodule);
void variables_set(bool is_static, const wchar_t *key, int val, module* defmodule = currentmodule);
wchar_t *get_dragged_file(wchar_t *buffer, WPARAM wParam);

void controls_clickraise(void);
void controls_updatetasks(void);
list* control_getcontrollist(void);

#endif
/*=================================================*/
