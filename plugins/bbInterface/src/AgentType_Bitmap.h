/*===================================================

	AGENTTYPE_BITMAP HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_AgentType_Bitmap_h
#define BBInterface_AgentType_Bitmap_h

//Includes
#include "AgentMaster.h"

//Define these structures
struct agenttype_bitmap_details
{
	int width;
	int height;
	wchar_t *filename;

	bool is_icon;
	int scale;
	int valign;
	int halign;
	wchar_t *absolute_path;
};

//Define these functions internally

int agenttype_bitmap_startup();
int agenttype_bitmap_shutdown();

int     agenttype_bitmap_create(agent *a, wchar_t *parameterstring);
int     agenttype_bitmap_destroy(agent *a);
int     agenttype_bitmap_message(agent *a, int tokencount, wchar_t *tokens[]);
void    agenttype_bitmap_notify(agent *a, int notifytype, void *messagedata);
void*   agenttype_bitmap_getdata(agent *a, int datatype);
void    agenttype_bitmap_menu_set(Menu *m, control *c, agent *a,  wchar_t *action, int controlformat);
void    agenttype_bitmap_menu_context(Menu *m, agent *a);
void    agenttype_bitmap_notifytype(int notifytype, void *messagedata);

void    agenttype_icon_menu_set(Menu *m, control *c, agent *a,  wchar_t *action, int controlformat);
int     agenttype_icon_create(agent *a, wchar_t *parameterstring);

#endif
/*=================================================*/
