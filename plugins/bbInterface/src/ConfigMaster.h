#pragma once
#include "Definitions.h"
//Only needed because variables store themselves directly in a listnode.
#include "ListMaster.h"

struct controltype; struct module; struct control;

//Global variables
extern wchar_t *config_path_plugin;
extern wchar_t *config_path_mainscript;
extern wchar_t *config_path_loadscript;
extern FILE * config_file_out;

//Functions
int config_startup();
int config_shutdown();


FILE *config_open(wchar_t const *filename, const wchar_t *mode);
int config_save(wchar_t *filename);
int config_delete(wchar_t *filename);
int config_load(wchar_t const *filename, module* caller, const wchar_t *section = NULL);
int config_write(wchar_t *string);
int config_backup(wchar_t *filename);
wchar_t* config_makepath(wchar_t *buffer, const wchar_t *filename);

void config_printf (const wchar_t *fmt, ...);
void config_printf_noskip (const wchar_t *fmt, ...);
int check_mainscript_filetime(void);
bool check_mainscript_version(void);


bool config_set_long(wchar_t *string, long *valptr);
bool config_set_int(const wchar_t *string, int *valptr);
bool config_set_int_expr(wchar_t *string, int *valptr);
bool config_set_str(wchar_t *string, wchar_t **valptr);
bool config_set_str(wchar_t const * string, bbstring & valptr);
bool config_set_double(wchar_t *string, double *valptr);
bool config_set_double_expr(wchar_t *str, double* value);
bool config_set_double_expr(wchar_t *str, double* value, double min, double max);
bool config_set_long(wchar_t *string, long *valptr, long min, long max);
bool config_set_int(const wchar_t *string, int *valptr, int min, int max);
bool config_set_double(wchar_t *string, double *valptr, double min, double max);
bool config_set_bool(wchar_t *string, bool *valptr);
bool config_isstringzero(const wchar_t *string);

wchar_t *config_get_control_create(controltype *ct);
wchar_t *config_get_control_create_named(controltype *ct, control *c);
wchar_t *config_get_control_create_child(control *c_p, controltype *ct);
wchar_t *config_get_control_create_child_named(control *c_p, controltype *ct, control *c);
wchar_t *config_get_control_saveas(control *c, const wchar_t *filename);
wchar_t *config_get_control_delete(control *c);
wchar_t *config_get_control_clone(control *c);

wchar_t *config_get_control_setagent_s(control *c, const wchar_t *action, const wchar_t *agenttype);
wchar_t *config_get_control_setagent_c(control *c, const wchar_t *action, const wchar_t *agenttype, const wchar_t *value);
wchar_t *config_get_control_setagent_b(control *c, const wchar_t *action, const wchar_t *agenttype, const bool *value);
wchar_t *config_get_control_setagent_i(control *c, const wchar_t *action, const wchar_t *agenttype, const int *value);
wchar_t *config_get_control_setagent_d(control *c, const wchar_t *action, const wchar_t *agenttype, const double *value);
wchar_t *config_get_control_removeagent(control *c, const wchar_t *action);
wchar_t *config_get_control_renamecontrol_s(control *c);
wchar_t *config_get_control_setagentprop_s(control *c, const wchar_t *action, const wchar_t *key);
wchar_t *config_get_control_setagentprop_c(control *c, const wchar_t *action, const wchar_t *key, const wchar_t *value);
wchar_t *config_get_control_setagentprop_b(control *c, const wchar_t *action, const wchar_t *key, const bool *value);
wchar_t *config_get_control_setagentprop_i(control *c, const wchar_t *action, const wchar_t *key, const int *value);
wchar_t *config_get_control_setagentprop_d(control *c, const wchar_t *action, const wchar_t *key, const double *value);
wchar_t *config_get_control_setcontrolprop_s(control *c, const wchar_t *key);
wchar_t *config_get_control_setcontrolprop_c(control *c, const wchar_t *key, const wchar_t *value);
wchar_t *config_get_control_setcontrolprop_b(control *c, const wchar_t *key, const bool *value);
wchar_t *config_get_control_setcontrolprop_i(control *c, const wchar_t *key, const int *value);
wchar_t *config_get_control_setcontrolprop_d(control *c, const wchar_t *key, const double *value);
wchar_t *config_get_control_setwindowprop_s(control *c, const wchar_t *key);
wchar_t *config_get_control_setwindowprop_c(control *c, const wchar_t *key, const wchar_t *value);
wchar_t *config_get_control_setwindowprop_b(control *c, const wchar_t *key, const bool *value);
wchar_t *config_get_control_setwindowprop_i(control *c, const wchar_t *key, const int *value);
wchar_t *config_get_control_setwindowprop_d(control *c, const wchar_t *key, const double *value);
wchar_t *config_get_agent_setagentprop_s(const wchar_t *agenttype, const wchar_t *key);
wchar_t *config_get_agent_setagentprop_c(const wchar_t *agenttype, const wchar_t *key, const wchar_t *value);
wchar_t *config_get_agent_setagentprop_b(const wchar_t *agenttype, const wchar_t *key, const bool *value);
wchar_t *config_get_agent_setagentprop_i(const wchar_t *agenttype, const wchar_t *key, const int *value);
wchar_t *config_get_agent_setagentprop_d(const wchar_t *agenttype, const wchar_t *key, const double *value);

wchar_t *config_get_control_setpluginprop_s(control *c, const wchar_t *key);
wchar_t *config_get_control_setpluginprop_b(control *c, const wchar_t *key, const bool *value);

wchar_t *config_get_plugin_setpluginprop_s(const wchar_t *key);
wchar_t *config_get_plugin_setpluginprop_c(const wchar_t *key, const wchar_t *value);
wchar_t *config_get_plugin_setpluginprop_b(const wchar_t *key, const bool *value);
wchar_t *config_get_plugin_setpluginprop_i(const wchar_t *key, const int *value);
wchar_t *config_get_plugin_setpluginprop_d(const wchar_t *key, const double *value);
wchar_t *config_get_plugin_load_dialog();
wchar_t *config_get_plugin_load(const wchar_t *file);
wchar_t *config_get_plugin_save();
wchar_t *config_get_plugin_saveas();
wchar_t *config_get_plugin_revert();
wchar_t *config_get_plugin_edit();

wchar_t *config_get_module_create();
wchar_t *config_get_module_load_dialog();
wchar_t *config_get_module_load(module *m);
wchar_t *config_get_module_toggle(module *m);
wchar_t *config_get_module_edit(module *m);
wchar_t *config_get_module_setdefault(module *m);

wchar_t *config_get_module_onload_s(module *m);
wchar_t *config_get_module_onunload_s(module *m);
wchar_t *config_get_module_setauthor_s(module *m);
wchar_t *config_get_module_setcomments_s(module *m);
wchar_t *config_get_module_rename_s(module *m);

wchar_t *config_get_control_assigntomodule(control *c, module *m);
wchar_t *config_get_control_detachfrommodule(control *c);
wchar_t *config_get_module_onload(module *m);
wchar_t *config_get_module_onunload(module *m);
wchar_t *config_get_plugin_onload();
wchar_t *config_get_plugin_onunload();

wchar_t *config_get_variable_set(listnode *ln);
wchar_t *config_get_variable_set_static(listnode *ln);

//---- using fully qualified names, here
wchar_t *config_getfull_control_create_child(control *c_p, controltype *ct);
wchar_t *config_getfull_control_create_child_named(control *c_p, controltype *ct, control *c);
wchar_t *config_getfull_control_delete(control *c);
wchar_t *config_getfull_control_saveas(control *c, const wchar_t *filename);
wchar_t *config_getfull_control_renamecontrol_s(control *c);
wchar_t *config_getfull_control_clone(control *c);

wchar_t *config_getfull_control_setagent_s(control *c, const wchar_t *action, const wchar_t *agenttype);
wchar_t *config_getfull_control_setagent_c(control *c, const wchar_t *action, const wchar_t *agenttype, const wchar_t *parameters);
wchar_t *config_getfull_control_setagent_b(control *c, const wchar_t *action, const wchar_t *agenttype, const bool *parameters);
wchar_t *config_getfull_control_setagent_i(control *c, const wchar_t *action, const wchar_t *agenttype, const int *parameters);
wchar_t *config_getfull_control_setagent_d(control *c, const wchar_t *action, const wchar_t *agenttype, const double *parameters);

wchar_t *config_getfull_control_removeagent(control *c, const wchar_t *action);

wchar_t *config_getfull_control_setagentprop_s(control *c, const wchar_t *action, const wchar_t *key);
wchar_t *config_getfull_control_setagentprop_c(control *c, const wchar_t *action, const wchar_t *key, const wchar_t *value);
wchar_t *config_getfull_control_setagentprop_b(control *c, const wchar_t *action, const wchar_t *key, const bool *value);
wchar_t *config_getfull_control_setagentprop_i(control *c, const wchar_t *action, const wchar_t *key, const int *value);
wchar_t *config_getfull_control_setagentprop_d(control *c, const wchar_t *action, const wchar_t *key, const double *value);

wchar_t *config_getfull_control_setcontrolprop_s(control *c, const wchar_t *key);
wchar_t *config_getfull_control_setcontrolprop_c(control *c, const wchar_t *key, const wchar_t *value);
wchar_t *config_getfull_control_setcontrolprop_b(control *c, const wchar_t *key, const bool *value);
wchar_t *config_getfull_control_setcontrolprop_i(control *c, const wchar_t *key, const int *value);
wchar_t *config_getfull_control_setcontrolprop_d(control *c, const wchar_t *key, const double *value);

wchar_t *config_getfull_control_setwindowprop_s(control *c, const wchar_t *key);
wchar_t *config_getfull_control_setwindowprop_c(control *c, const wchar_t *key, const wchar_t *value);
wchar_t *config_getfull_control_setwindowprop_b(control *c, const wchar_t *key, const bool *value);
wchar_t *config_getfull_control_setwindowprop_i(control *c, const wchar_t *key, const int *value);
wchar_t *config_getfull_control_setwindowprop_d(control *c, const wchar_t *key, const double *value);

wchar_t *config_getfull_control_assigntomodule(control *c, module *m);
wchar_t *config_getfull_control_detachfrommodule(control *c);
wchar_t *config_getfull_variable_set_static_s(module *m, listnode *ln);

