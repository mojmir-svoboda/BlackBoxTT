/*===================================================

	DIALOG MASTER HEADERS

===================================================*/

//Multiple definition prevention
#ifndef BBInterface_DialogMaster_h
#define BBInterface_DialogMaster_h

//Includes
#include "Definitions.h"

//Define these structures

//Define these functions internally
int dialog_startup();
int dialog_shutdown();
wchar_t *dialog_file(const wchar_t *filter, const wchar_t *title, const wchar_t *defaultpath, const wchar_t *defaultextension, bool save);

#endif
/*=================================================*/
