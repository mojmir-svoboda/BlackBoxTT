#pragma once

struct module; class Menu; struct agent; struct control;

struct agenttype
{
	wchar_t    agenttypenamefriendly[64];
	wchar_t    agenttypename[64];
	int     format;
	bool    writable;
	int     (*func_create) (agent *a, wchar_t *parameterstring);
	int     (*func_destroy) (agent *a);
	int     (*func_message) (agent *a, int tokencount, wchar_t *tokens[]);
	void    (*func_notify) (agent *a, int notifytype, void *messagedata);
	void *  (*func_getdata) (agent *a, int datatype);
	void    (*func_menu_set) (Menu *m, control *c, agent *a, wchar_t *action, int controlformat);
	void    (*func_menu_context) (Menu *m, agent *a);
	void    (*func_notifytype) (int notifytype, void *messagedata);
};

struct agent
{
	wchar_t agentname[256];
	wchar_t agentaction[256];
	int format;
	agenttype * agenttypeptr;        //Pointer to the type of agent
	agent * parentagentptr;          //Pointer to the parent agent (null if it is directly attached to a control)
	control * controlptr;            //Pointer to the control
	void * agentdetails;         //Pointer to details about the agent
	bool writable;
};

int agent_startup ();
int agent_shutdown ();

int agent_create (agent **a, control *c, agent *parentagent, wchar_t *action, wchar_t *agenttypename, wchar_t *parameterstring, int dataformat);
int agent_destroy (agent **a);

void agent_save ();

void agent_notify (agent *a, int notifytype, void *data);
void *agent_getdata (agent *a, int datatype);
int agent_message (int tokencount, wchar_t *tokens[], bool from_core, module* caller);
int agent_message (agent *a, int tokencount, wchar_t *tokens[]);
int agent_controlmessage (control *c, int tokencount, wchar_t *tokens[], int agentcount, agent *agents[], wchar_t *agentnames[], int agenttypes[]);

void agent_menu_set (Menu *m, control *c, agent *a, int controlformat, wchar_t *action);
void agent_menu_context (Menu *m, control *c, agent *a);

void agent_registertype(
	wchar_t *  agenttypenamefriendly,
	wchar_t *  agenttypename,
	int     format,
	bool    writable,
	int     (*func_create) (agent *a, wchar_t *parameterstring),
	int     (*func_destroy) (agent *a),
	int     (*func_message) (agent *a, int tokencount, wchar_t *tokens[]),
	void    (*func_notify) (agent *a, int notifytype, void *messagedata),
	void *  (*func_getdata) (agent *a, int datatype),
	void    (*func_menu_set) (Menu *m, control *c, agent *a, wchar_t *action, int controlformat),
	void    (*func_menu_context) (Menu *m, agent *a),
	void    (*func_notifytype) (int notifytype, void *messagedata)
	);

void agent_unregistertype (agenttype *at);

