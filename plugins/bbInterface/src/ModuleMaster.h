#pragma once
#include <blackbox/BlackBox_compat.h>

struct list; struct listnode;

#define MODULE_ACTION_ONLOAD 0
#define MODULE_ACTION_ONUNLOAD 1
#define MODULE_ACTION_COUNT 2

struct module
{
	//Info fields - the first one is necessary.
	wchar_t name[64];
	wchar_t *author;
	wchar_t *comments;

	wchar_t *filepath;       //path to the module file
	bool enabled;

	list *controllist; // list of controls associated with module
	list *controllist_parentsonly; // list of controls associated with module
	list *variables; // list of variables associated with module

	wchar_t *actions[MODULE_ACTION_COUNT];
};

extern module *currentmodule;
extern module globalmodule;

//Define these functions internally
int module_startup();
int module_shutdown();

module* module_create(wchar_t *filepath);
module* module_create_new(wchar_t *filename);
int module_destroy(module *m, bool remove_from_list);
int module_toggle(module *m);
int module_toggle(wchar_t *modulename);

int module_message(int tokencount, wchar_t *tokens[], bool from_core, module* caller);
bool module_state(wchar_t *modulename);

std::shared_ptr<bb::MenuConfig>  module_menu_modulelist();
std::shared_ptr<bb::MenuConfig>  module_menu_editmodules();
std::shared_ptr<bb::MenuConfig>  module_menu_setactivemodule();

void module_save_list();
void module_save_all();
module* module_get(wchar_t* key);


extern list *modulelist;
