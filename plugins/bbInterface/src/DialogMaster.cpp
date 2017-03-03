/*===================================================

	DIALOG MASTER CODE

===================================================*/

// Global Include
#include <blackbox/plugin/bb.h>
#include <commdlg.h>

//Includes
#include "PluginMaster.h"
#include "Definitions.h"

//Define these variables
wchar_t *dialog_filename;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//dialog_startup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int dialog_startup()
{
	dialog_filename = new wchar_t[MAX_PATH];
	return 0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//dialog_shutdown
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int dialog_shutdown()
{
	delete[] dialog_filename;
	dialog_filename = nullptr;
	return 0;
}

#include <windows.h>
#include <shobjidl.h> 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//dialog_openfile
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
wchar_t *dialog_file_open (const wchar_t * filter, const wchar_t * title, const wchar_t * defaultpath, const wchar_t * defaultextension)
{
	dialog_filename[0] = L'\0';

	IFileOpenDialog * dlg = nullptr;
	HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&dlg));
	if (SUCCEEDED(hr))
	{
		hr = dlg->SetDefaultExtension(defaultextension);

		if (SUCCEEDED(dlg->Show(NULL)))
		{
			IShellItem * pItem = nullptr;
			if (SUCCEEDED(dlg->GetResult(&pItem)))
			{
				PWSTR pszFilePath = nullptr;
				if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
				{
					wcscpy(dialog_filename, pszFilePath);
					CoTaskMemFree(pszFilePath);
					return dialog_filename;
				}
				pItem->Release();
			}
		}
		dlg->Release();
	}
	return nullptr;
}
wchar_t *dialog_file_save (const wchar_t * filter, const wchar_t * title, const wchar_t * defaultpath, const wchar_t * defaultextension)
{
	dialog_filename[0] = L'\0';

	IFileOpenDialog * dlg = nullptr;
	HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&dlg));
	if (SUCCEEDED(hr))
	{
		hr = dlg->SetDefaultExtension(defaultextension);

		if (SUCCEEDED(dlg->Show(NULL)))
		{
			IShellItem * pItem = nullptr;
			if (SUCCEEDED(dlg->GetResult(&pItem)))
			{
				PWSTR pszFilePath = nullptr;
				if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath)))
				{
					wcscpy(dialog_filename, pszFilePath);
					CoTaskMemFree(pszFilePath);
					return dialog_filename;
				}
				pItem->Release();
			}
		}
		dlg->Release();
	}
	return nullptr;
}
wchar_t *dialog_file (const wchar_t * filter, const wchar_t * title, const wchar_t * defaultpath, const wchar_t * defaultextension, bool save)
{
	if (save)
		return dialog_file_save(filter, title, defaultpath, defaultextension);
	else
		return dialog_file_open(filter, title, defaultpath, defaultextension);

// 	OPENFILENAME ofn = { 0 };
// 	ofn.lStructSize = sizeof(ofn);
// 	ofn.hwndOwner = NULL;
// 	ofn.lpstrFile = dialog_filename;
// 	//
// 	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
// 	// use the contents of szFile to initialize itself.
// 	//
// 	ofn.lpstrFile[0] = L'\0';
// 	ofn.nMaxFile = MAX_PATH;
// 	ofn.lpstrFilter = filter;
// 	ofn.nFilterIndex = 1;
// 	ofn.lpstrFileTitle = NULL;
// 	ofn.nMaxFileTitle = 0;
// 	ofn.lpstrInitialDir = defaultpath;
// 	ofn.lpstrTitle = title;
// 	ofn.lpstrDefExt = defaultextension;
// 
// 	// Display the Open dialog box. 
// 
// 	if (save)
// 	{
// 		ofn.Flags = OFN_PATHMUSTEXIST;
// 		if (GetSaveFileName(&ofn)) return dialog_filename;
// 	}
// 	else
// 	{       
// 		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS;
// 		if (GetOpenFileName(&ofn))
// 			return dialog_filename;
// 		else
// 		{
// 			DWORD res = CommDlgExtendedError();
// 			switch (res)
// 			{
// 				default:
// 					break;
// 			}
// 		}
// 	}

	return nullptr;
}


/*=================================================*/

