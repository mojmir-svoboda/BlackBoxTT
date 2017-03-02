/*===================================================

	AGENTTYPE_TGA CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <string.h>

//Parent Include
#include "AgentType_TGA.h"

//Includes
#include "PluginMaster.h"
#include "AgentMaster.h"
#include "Definitions.h"
#include "ConfigMaster.h"
#include "StyleMaster.h"
#include "DialogMaster.h"
#include "MessageMaster.h"
#include "MenuMaster.h"
#include "Shellapi.h"

//Local functions
HBITMAP agenttype_tga_loadtga(const wchar_t *filename);

//Local structs
#pragma pack(push, 1)
struct TGAHeader
{
	BYTE    length;
	BYTE    maptype;
	BYTE    type;
	WORD    mapstart;
	WORD    CMapLength;
	BYTE    mapdepth;
	WORD    offset_x;
	WORD    offset_y;
	WORD    width;
	WORD    height;
	BYTE    depth;
	BYTE    descriptor;
};
#pragma pack(pop)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//controltype_button_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_tga_startup()
{
	//Register this type with the ControlMaster
	agent_registertype(
		L"TGA",                          //Friendly name of agent type
		L"TGA",                          //Name of agent type
		CONTROL_FORMAT_IMAGE,               //Control type
		true,
		&agenttype_tga_create,          
		&agenttype_tga_destroy,
		&agenttype_tga_message,
		&agenttype_tga_notify,
		&agenttype_tga_getdata,
		&agenttype_tga_menu_set,
		&agenttype_tga_menu_context,
		&agenttype_tga_notifytype
		);

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_tga_shutdown()
{
	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_create
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_tga_create(agent *a, wchar_t *parameterstring)
{
	//If the browse option is chosen
	if (!_wcsicmp(parameterstring, L"*browse*"))
	{       
		parameterstring = dialog_file(L"TGA Images\0*.tga\0", L"Select TGA", config_path_plugin, L".tga", false);
		if (!parameterstring)
		{
			//message_override = true;
			return 2;
		}

		//If we have an absolute path, and only a relative path is necessary
		int lenpath = wcslen(config_path_plugin);
		if (!_wcsnicmp(config_path_plugin, parameterstring, lenpath-1))
		{
			wcscpy(parameterstring, &parameterstring[lenpath]);           
		}
	}

	//Load the tga itself
	HANDLE image;
	image = agenttype_tga_loadtga(parameterstring);
	if (!image)
	{
		wchar_t *temp = new wchar_t[MAX_PATH*2];
		wcscpy(temp, config_path_plugin);
		wcscat(temp, parameterstring);
		image = agenttype_tga_loadtga(temp);
		delete[] temp;
		if (!image)
		{
			return 1;
		}
	}
	
	//Create the details
	agenttype_tga_details *details = new agenttype_tga_details;
	a->agentdetails = (void *) details;
	details->image = image;

	//Get the size
	BITMAP tga; 
	GetObject(details->image, sizeof(tga), &tga);
	details->height = tga.bmHeight;
	details->width = tga.bmWidth;

	//Copy the parameter string
	details->filename = new_string(parameterstring); 

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_destroy
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_tga_destroy(agent *a)
{
	//Delete the details if possible
	if (a->agentdetails)
	{
		agenttype_tga_details *details =(agenttype_tga_details *) a->agentdetails;
		if (details->image) DeleteObject(details->image);
		free_string(&details->filename);
		delete details;
		a->agentdetails = NULL;
	}

	//No errors
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int agenttype_tga_message(agent *a, int tokencount, wchar_t *tokens[])
{
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_notify
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_tga_notify(agent *a, int notifytype, void *messagedata)
{
	//Get the agent details
	agenttype_tga_details *details = (agenttype_tga_details *) a->agentdetails;

	styledrawinfo *di;
	int xpos, ypos;

	switch(notifytype)
	{
		case NOTIFY_DRAW:           
			di = (styledrawinfo *) messagedata;
			xpos = ((di->rect.right - di->rect.left) / 2) - (details->width / 2);
			ypos = ((di->rect.bottom - di->rect.top) / 2) - (details->height / 2);  
			style_draw_image_alpha(di->buffer, xpos, ypos, details->width, details->height, details->image);
			break;

		case NOTIFY_SAVE_AGENT:
			//Write existance
			config_write(config_get_control_setagent_c(a->controlptr, a->agentaction, a->agenttypeptr->agenttypename, details->filename));
			break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_getdata
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void *agenttype_tga_getdata(agent *a, int datatype)
{
	agenttype_tga_details *details = (agenttype_tga_details *) a->agentdetails;
	switch (datatype)
	{
		case DATAFETCH_CONTENTSIZES: 
			return &details->width;
	}

	return NULL;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_menu_set
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_tga_menu_set(std::shared_ptr<bb::MenuConfig> m, control *c, agent *a,  wchar_t *action, int controlformat)
{
	make_menuitem_cmd(m, L"Browse...", config_getfull_control_setagent_c(c, action, L"TGA", L"*browse*"));
	make_menuitem_nop(m, L"(Uncompressed 32bpp TGAs only!");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_menu_context
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_tga_menu_context(std::shared_ptr<bb::MenuConfig> m, agent *a)
{
	make_menuitem_nop(m, L"No options available.");
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//agenttype_tga_notifytype
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void agenttype_tga_notifytype(int notifytype, void *messagedata)
{

}

//##################################################
//agenttype_tga_loadtga
//##################################################
HBITMAP agenttype_tga_loadtga(const wchar_t *filename)
{
	//Variables
	TGAHeader header;
	DWORD readword = 0;
	HBITMAP hbitmap = NULL;
	void *bits = NULL;

	//Open the file
	HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) return NULL;

	//Read the file header
	ReadFile(handle, & header, sizeof(header), & readword, NULL);
	if (
		(header.length!=0)
		|| (header.maptype!=0)
		|| (header.type!=2)
		|| (header.depth!=32)
		|| (header.descriptor!=8)
		)
	{
		CloseHandle(handle);
		return NULL;
	}

	//Create the bitmap
	BITMAPINFO bmp = { { sizeof(BITMAPINFOHEADER), header.width, header.height, 1, 32 } };    
	hbitmap = CreateDIBSection(NULL, & bmp, DIB_RGB_COLORS, & bits, NULL, 0);
	if (!hbitmap) {CloseHandle(handle); return NULL;}

	//Read the rest of the file
	ReadFile(handle, bits, header.width * header.height * 4, & readword, NULL);
	
	//Close the file
	CloseHandle(handle);

	//Process the pixels
	for (int y=0; y<header.height; y++)
	{
		BYTE * pixel = (BYTE *) bits + header.width * 4 * y;

		for (int x = 0; x < header.width; x++)
		{
			pixel[0] = pixel[0] * pixel[3] / 255; 
			pixel[1] = pixel[1] * pixel[3] / 255; 
			pixel[2] = pixel[2] * pixel[3] / 255; 
			pixel += 4;
		}
	}

	//Return the bitmap
	return hbitmap;
}


/*=================================================*/
