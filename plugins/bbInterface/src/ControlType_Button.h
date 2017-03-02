/*===================================================

	CONTROLTYPE_BUTTON HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_ControlType_Button_h
#define BBInterface_ControlType_Button_h

//Includes
#include "ControlMaster.h"
#include "AgentMaster.h"

//Define these structures
struct controltype_button_details
{
	bool pressed;
	bool hilite; 

	int valign;
	int halign;
	UINT settings;

	bool is_twostate;
	bool is_on;  // when is_twostate

	agent *agents[12];
	wchar_t *caption;
};

//Define these functions internally
int controltype_button_startup();
int controltype_button_shutdown();
int controltype_button_create(control *c);
int controltype_switchbutton_create(control *c);
int controltype_button_destroy(control *c);
LRESULT controltype_button_event(control *c, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void controltype_button_notify(control *c, int notifytype, void *messagedata);
int controltype_button_message(control *c, int tokencount, wchar_t *tokens[]);
void *controltype_button_getdata(control *c, int datatype);
bool controltype_button_getstringdata(control *c, wchar_t *buffer, size_t n, wchar_t const *propertyname);
void controltype_button_menu_context(std::shared_ptr<bb::MenuConfig> m, control *c);
void controltype_switchbutton_menu_context(std::shared_ptr<bb::MenuConfig> m, control *c);
void controltype_button_notifytype(int notifytype, void *messagedata);
#endif
/*=================================================*/
