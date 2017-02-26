/*===================================================

	MESSAGE MASTER HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_MessageMaster_h
#define BBInterface_MessageMaster_h

//Includes
#include "AgentMaster.h"
#include "ControlMaster.h"
#include "Definitions.h"

//Global variables
//extern bool message_override;

//Define these functions internally

int message_startup();
int message_shutdown();
void message_interpret(const wchar_t *message, bool from_core, module* caller);

extern HWND message_window;
void shell_exec(const wchar_t *command, const wchar_t *arguments = NULL, const wchar_t *workingdir = NULL);
wchar_t *message_preprocess(wchar_t *buffer, module* defmodule = currentmodule);

#endif
/*=================================================*/
