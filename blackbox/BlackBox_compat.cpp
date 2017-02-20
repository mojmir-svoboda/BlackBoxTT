/* ==========================================================================

This file is part of the bbLean source code
Copyright © 2001-2003 The Blackbox for Windows Development Team
Copyright © 2004-2009 grischka

http://bb4win.sourceforge.net/bblean
http://developer.berlios.de/projects/bblean

bbLean is free software, released under the GNU General Public License
(GPL version 2). For details see:

http://www.fsf.org/licenses/gpl.html

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

========================================================================== */
#include "BlackBox_compat.h"

//===========================================================================
// API: MakeNamedMenu
// Purpose:         Create or refresh a Menu
// In: HeaderText:  the menu title
// In: IDString:    An unique string that identifies the menu window
// In: popup        true: menu is to be shown, false: menu is to be refreshed
// Out: Menu *:     A pointer to a Menu structure (opaque for the client)
// Note:            A menu once it has been created must be passed to
//                  either 'MakeSubMenu' or 'ShowMenu'.
//===========================================================================

Menu *MakeNamedMenu (wchar_t const * HeaderText, wchar_t const * IDString, bool popup)
{
	return nullptr;
// 	Menu *pMenu = NULL;
// 	if (IDString)
// 		pMenu = Menu::find_named_menu(IDString);
// 
// 	if (pMenu) {
// 		pMenu->incref();
// 		pMenu->SaveState();
// 		pMenu->DeleteMenuItems();
// 		if (HeaderText)
// 			replace_str(&pMenu->m_pMenuItems->m_pszTitle, NLS1(HeaderText));
// 	} else {
// 		pMenu = new Menu(NLS1(HeaderText));
// 		pMenu->m_IDString = new_str(IDString);
// 	}
// 	pMenu->m_bPopup = popup;
// 	//dbg_printf("MakeNamedMenu (%d) %x %s <%s>", popup, pMenu, HeaderText, IDString);
// 	return pMenu;
}

//===========================================================================
// API: MakeMenu
// Purpose: as above, for menus that dont need refreshing
//===========================================================================
Menu *MakeMenu (wchar_t const * HeaderText)
{
	return MakeNamedMenu(HeaderText, NULL, true);
}

//===========================================================================
// API: DelMenu
// Purpose: obsolete
//===========================================================================

void DelMenu (Menu *PluginMenu)
{
	// Nothing here. We just dont know wether 'PluginMenu' still
	// exists. The pointer may be invalid or even belong to
	// a totally different memory object.
}

//===========================================================================
// API: ShowMenu
// Purpose: Finalizes creation or refresh for the menu and its submenus
// IN: PluginMenu - pointer to the toplevel menu
//===========================================================================

void ShowMenu (Menu *PluginMenu)
{
// 	if (NULL == PluginMenu)
// 		return;
// 	// dbg_printf("ShowMenu(%d) %x %s", PluginMenu->m_bPopup, PluginMenu, PluginMenu->m_pMenuItems->m_pszTitle);
// 
// 	if (PluginMenu->m_bPopup) {
// 		// just to signal e.g. BBSoundFX
// #ifndef BBXMENU
// 		PostMessage(BBhwnd, BB_MENU, BB_MENU_SIGNAL, 0);
// #endif
// 		PluginMenu->ShowMenu();
// 	} else {
// 		PluginMenu->Redraw(1);
// 		PluginMenu->decref();
// 	}
}

//===========================================================================
// API: MakeSubmenu
//===========================================================================

MenuItem* MakeSubmenu (Menu *ParentMenu, Menu *ChildMenu, wchar_t const * Title)
{
	return nullptr;
// 	//dbg_printf("MakeSubmenu %x %s - %x %s", ParentMenu, ParentMenu->m_pMenuItems->m_pszTitle, ChildMenu, Title);
// 	if (Title)
// 		Title = NLS1(Title);
// 	else
// 		Title = ChildMenu->m_pMenuItems->m_pszTitle;
// 	return ParentMenu->AddMenuItem(new FolderItem(ChildMenu, Title));
}

//===========================================================================
// API: MakeMenuItem
//===========================================================================

MenuItem *MakeMenuItem(Menu *PluginMenu, wchar_t const * Title, wchar_t const * Cmd, bool ShowIndicator)
{
	return nullptr;
// 	//dbg_printf("MakeMenuItem %x %s", PluginMenu, Title);
// 	return PluginMenu->AddMenuItem(new CommandItem(Cmd, NLS1(Title), ShowIndicator));
}

//===========================================================================
// API: MakeMenuItemInt
//===========================================================================

MenuItem *MakeMenuItemInt(Menu *PluginMenu, wchar_t const * Title, wchar_t const * Cmd, int val, int minval, int maxval)
{
	return nullptr;
// 	return helper_menu(PluginMenu, Title, MENU_ID_INT,
// 		new IntegerItem(Cmd, val, minval, maxval));
}

//===========================================================================
// API: MakeMenuItemString
//===========================================================================

MenuItem *MakeMenuItemString(Menu *PluginMenu, wchar_t const * Title, wchar_t const * Cmd, wchar_t const * init_string)
{
	return nullptr;
// 	return helper_menu(PluginMenu, Title, MENU_ID_STRING,
// 		new StringItem(Cmd, init_string));
}

//===========================================================================
// API: MakeMenuNOP
//===========================================================================

MenuItem* MakeMenuNOP(Menu *PluginMenu, wchar_t const * Title)
{
	return nullptr;
// 	/* BlackboxZero 1.8.2012 - For separator graident? */
// 	MenuItem *pItem;
// 	if (Title && Title[0]) {
// 		pItem = new MenuItem(NLS1(Title));
// 	} else {
// 		pItem = new SeparatorItem();
// 	}
// 	pItem->m_bNOP = true;
// 	return PluginMenu->AddMenuItem(pItem);
}

//===========================================================================
// API: MakeMenuGrip
//===========================================================================

MenuItem* MakeMenuGrip(Menu *PluginMenu, LPCSTR Title)
{
	return nullptr;
// 	if ( Settings_menusGripEnabled )
// 		return PluginMenu->AddMenuItem(new MenuGrip(Title));
// 	return 0;
}


//===========================================================================
// API: MakeMenuItemPath
//===========================================================================

MenuItem* MakeMenuItemPath (Menu *ParentMenu, wchar_t const * Title, wchar_t const * path, wchar_t const * Cmd)
{
	return nullptr;
// 	MenuItem * pItem = new SpecialFolderItem(NLS1(Title), path, NULL, Cmd);
// 	return ParentMenu->AddMenuItem(pItem);
}

MenuItem* MakeMenuInsertPath (Menu *ParentMenu, wchar_t const * Title, wchar_t const * path, wchar_t const * Cmd)
{
	return nullptr;
// 	MenuItem * items = NULL;
// 	pidl_node * p = get_folder_pidl_list(path);
// 	ParentMenu->AddFolderContents(p, Cmd);
// 	delete_pidl_list(&p);
// 	return NULL;
}

//===========================================================================
// API: MenuExists
//===========================================================================

bool MenuExists(wchar_t const * IDString_part)
{
	return false;
/*	return NULL != Menu::find_named_menu(IDString_part, true);*/
}

//===========================================================================
// API: MenuItemOption - set some options for a menuitem
//===========================================================================

void MenuItemOption(MenuItem *pItem, int option, ...)
{
// 	va_list vl;
// 	if (NULL == pItem)
// 		return;
// 	va_start(vl, option);
// 	switch (option) {
// 
// 		// disabled text style
// 	case BBMENUITEM_DISABLED:
// 		pItem->m_bDisabled = true;
// 		if (NULL == pItem->m_pszRightCommand) /* hack for the taskmenus */
// 			pItem->m_bNOP = true;
// 		break;
// 
// 		// set checkmark
// 	case BBMENUITEM_CHECKED:
// 		pItem->m_bChecked = true;
// 		break;
// 
// 		// set a command for left click
// 	case BBMENUITEM_LCOMMAND:
// 		replace_str(&pItem->m_pszCommand, va_arg(vl, wchar_t const *));
// 		break;
// 
// 		// set a command for right click
// 	case BBMENUITEM_RCOMMAND:
// 		replace_str(&pItem->m_pszRightCommand, va_arg(vl, wchar_t const *));
// 		break;
// 
// 		// set a command for right click
// 	case BBMENUITEM_RMENU:
// 	{
// 		Menu *pSub = pItem->m_pRightmenu = va_arg(vl, Menu*);
// 		pSub->m_MenuID = MENU_ID_RMENU;
// 		break;
// 	}
// 
// 	// set a special value and display text for the integer items
// 	case BBMENUITEM_OFFVAL:
// 	{
// 		IntegerItem* IntItem = (IntegerItem*)pItem->get_real_item();
// 		if (IntItem && IntItem->m_ItemID == MENUITEM_ID_INT) {
// 			const char *p;
// 			int n;
// 			if (NULL == IntItem)
// 				break;
// 			n = va_arg(vl, int);
// 			p = va_arg(vl, wchar_t const *);
// 			IntItem->m_offvalue = n;
// 			IntItem->m_offstring = p ? p : NLS0("off");
// 		}
// 		break;
// 	}
// 
// 	// set a flag that the checkmarks are updated each time
// 	case BBMENUITEM_UPDCHECK:
// 		pItem->m_ItemID |= MENUITEM_UPDCHECK;
// 		break;
// 
// 		// set a special justify mode (DT_LEFT/DT_CENTER/DT_RIGHT)
// 	case BBMENUITEM_JUSTIFY:
// 		pItem->m_Justify = va_arg(vl, int);
// 		break;
// 
// 		//#ifdef BBOPT_MENUICONS
// 		// set an icon for this item by "path\to\icon[,#iconid]"
// 	case BBMENUITEM_SETICON:
// 		if ( Settings_menu.iconSize ) /* BlackboxZero 1.3.2012 */
// 			replace_str(&pItem->m_pszIcon, va_arg(vl, wchar_t const *));
// 		break;
// 
// 		// set an icon for this item by HICON
// 	case BBMENUITEM_SETHICON:
// 		if ( Settings_menu.iconSize ) /* BlackboxZero 1.3.2012 */
// 			pItem->m_hIcon = CopyIcon(va_arg(vl, HICON));
// 		break;
// 		//#endif
// 	}
}

//===========================================================================
// API: MenuOption - set some special features for a individual menu
//===========================================================================

void MenuOption(Menu *pMenu, int flags, ...)
{
// 	va_list vl;
// 	int pos;
// 	if (NULL == pMenu)
// 		return;
// 
// 	va_start(vl, flags);
// 
// 	pos = flags & BBMENU_POSMASK;
// 	pMenu->m_flags |= (flags & ~BBMENU_POSMASK) | pos;
// 
// 	if (pos == BBMENU_XY) {
// 		pMenu->m_pos.left = va_arg(vl, int),
// 			pMenu->m_pos.top = va_arg(vl, int);
// 	} else if (pos == BBMENU_RECT)
// 		pMenu->m_pos = *va_arg(vl, RECT*);
// 
// 	if (flags & BBMENU_MAXWIDTH)
// 		pMenu->m_maxwidth = va_arg(vl, int);
// 
// 	if (flags & BBMENU_HWND)
// 		pMenu->m_hwndRef = va_arg(vl, HWND);
// 
// 	if (flags & BBMENU_ISDROPTARGET)
// 		pMenu->m_bIsDropTarg = true;
// 
// 	if (flags & BBMENU_SORT)
// 		Menu::Sort(&pMenu->m_pMenuItems->next, item_compare);
}

