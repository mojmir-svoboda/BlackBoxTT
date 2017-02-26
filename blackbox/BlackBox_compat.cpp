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
#include "BlackBox.h"
#include "gfx/MenuWidget.h"
#include "menu/MenuConfig.h"
#include "gfx/utils_widgets.h"
//===========================================================================
// API: MakeNamedMenu
// Purpose:         Create or refresh a bb::MenuConfig
// In: headerText:  the menu title
// In: id:    An unique string that identifies the menu window
// In: popup        true: menu is to be shown, false: menu is to be refreshed
// Out: std::shared_ptr<bb::MenuConfig>:     A pointer to a bb::MenuConfig structure (opaque for the client)
// Note:            A menu once it has been created must be passed to
//                  either 'MakeSubMenu' or 'ShowMenu'.
//===========================================================================

std::shared_ptr<bb::MenuConfig> MakeNamedMenu (wchar_t const * headerText, wchar_t const * id, bool popup)
{
	std::shared_ptr<bb::MenuConfig> m(new bb::MenuConfig());
	m->m_widgetType = bb::MenuWidget::c_type;
	m->m_id = id;
	m->m_titlebar = headerText;
	m->m_show = true;
	return m;
}

// 	std::shared_ptr<bb::MenuConfig>pMenu = NULL;
// 	if (IDString)
// 		pMenu = bb::MenuConfig::find_named_menu(IDString);
// 
// 	if (pMenu) {
// 		pMenu->incref();
// 		pMenu->SaveState();
// 		pMenu->DeleteMenuItems();
// 		if (HeaderText)
// 			replace_str(&pMenu->m_pMenuItems->m_pszTitle, NLS1(HeaderText));
// 	} else {
// 		pMenu = new bb::MenuConfig(NLS1(HeaderText));
// 		pMenu->m_IDString = new_str(IDString);
// 	}
// 	pMenu->m_bPopup = popup;
// 	//dbg_printf("MakeNamedMenu (%d) %x %s <%s>", popup, pMenu, HeaderText, IDString);
// 	return pMenu;
//}

//===========================================================================
// API: MakeMenu
// Purpose: as above, for menus that dont need refreshing
//===========================================================================
std::shared_ptr<bb::MenuConfig> MakeMenu (wchar_t const * HeaderText)
{
	static int menu_counter = 0;
	wchar_t tmp[256];
	swprintf(tmp, 256, L"Menu_%i", menu_counter++);
	return MakeNamedMenu(HeaderText, tmp, true);
}

//===========================================================================
// API: DelMenu
//===========================================================================

void DelMenu (std::shared_ptr<bb::MenuConfig> pluginMenu)
{
	bb::BlackBox & bb = bb::BlackBox::Instance();

	if (bb::GuiWidget * w = bb.GetGfx().FindWidget(pluginMenu->m_id.c_str()))
	{
		bb::GfxWindow * r = w->m_gfxWindow->GetRoot();
		r->SetDestroyTree();
		pluginMenu.reset();
	}
}

//===========================================================================
// API: ShowMenu
// Purpose: Finalizes creation or refresh for the menu and its submenus
// IN: pluginMenu - pointer to the toplevel menu
//===========================================================================

void ShowMenu (std::shared_ptr<bb::MenuConfig> pluginMenu)
{
	bb::BlackBox & bb = bb::BlackBox::Instance();

	bool update = false;
	bb::GuiWidget * w = bb.GetGfx().FindWidget(pluginMenu->m_id.c_str());
	if (!w)
	{
		w = bb.GetGfx().MkWidgetFromConfig(*pluginMenu);
	}
	else
	{
		update = true;
	}

	Assert(w && w->GetWidgetTypeName() == bb::MenuWidget::c_type);
	bb::MenuWidget * menu = static_cast<bb::MenuWidget *>(w);
	if (update)
	{
		//menu->GetConfig().clear();
		//menu->GetConfig() = *pluginMenu;
	}

 	moveWidgetToPointerPos(menu);
 	menu->Show(true);

// 	if (NULL == pluginMenu)
// 		return;
// 	// dbg_printf("ShowMenu(%d) %x %s", pluginMenu->m_bPopup, pluginMenu, pluginMenu->m_pMenuItems->m_pszTitle);
// 
// 	if (pluginMenu->m_bPopup) {
// 		// just to signal e.g. BBSoundFX
// #ifndef BBXMENU
// 		PostMessage(BBhwnd, BB_MENU, BB_MENU_SIGNAL, 0);
// #endif
// 		pluginMenu->ShowMenu();
// 	} else {
// 		pluginMenu->Redraw(1);
// 		pluginMenu->decref();
// 	}
}

void UpdateMenu (std::shared_ptr<bb::MenuConfig> pluginMenu)
{
	bb::BlackBox & bb = bb::BlackBox::Instance();

	bool update = false;
	bb::GuiWidget * w = bb.GetGfx().FindWidget(pluginMenu->m_id.c_str());
	if (!w)
	{
		//w = bb.GetGfx().MkWidgetFromConfig(*pluginMenu);
		//ShowMenu(pluginMenu);
	}
	else
	{
		update = true;
		Assert(w && w->GetWidgetTypeName() == bb::MenuWidget::c_type);
		bb::MenuWidget * menu = static_cast<bb::MenuWidget *>(w);
		if (update)
		{
			menu->GetConfig().clear();
			menu->GetConfig() = *pluginMenu;
		}

	}

	//moveWidgetToPointerPos(menu);
	//w->Show(true);
}

//===========================================================================
// API: MakeSubmenu
//===========================================================================

bb::MenuConfigItem * MakeSubmenu (std::shared_ptr<bb::MenuConfig> parentMenu, std::shared_ptr<bb::MenuConfig> childMenu, wchar_t const * title)
{
	std::shared_ptr<bb::MenuConfigItemSubMenu> item(new bb::MenuConfigItemSubMenu(title, childMenu));
	parentMenu->m_items.push_back(item);
	return item.get();
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

bb::MenuConfigItem * MakeMenuItem (std::shared_ptr<bb::MenuConfig> menu, wchar_t const * title, wchar_t const * cmd)
{
	std::shared_ptr<bb::MenuConfigItemBroam> item(new bb::MenuConfigItemBroam(title, cmd));
	menu->m_items.push_back(item);
	return item.get();
}

bb::MenuConfigItem * MakeMenuItemBool (std::shared_ptr<bb::MenuConfig> menu, const wchar_t * title, const wchar_t * cmd, bool checked)
{
	std::shared_ptr<bb::MenuConfigItemBroamBool> item(new bb::MenuConfigItemBroamBool(title, cmd, checked));
	menu->m_items.push_back(item);
	return item.get();
}

//===========================================================================
// API: MakeMenuItemInt
//===========================================================================

bb::MenuConfigItem * MakeMenuItemInt (std::shared_ptr<bb::MenuConfig> menu, wchar_t const * title, wchar_t const * cmd, int val, int minval, int maxval)
{
	std::shared_ptr<bb::MenuConfigItemBroamInt> item(new bb::MenuConfigItemBroamInt(title, cmd, minval, val, maxval));
	menu->m_items.push_back(item);
	return item.get();
}

//===========================================================================
// API: MakeMenuItemString
//===========================================================================

bb::MenuConfigItem *MakeMenuItemString(std::shared_ptr<bb::MenuConfig>pluginMenu, wchar_t const * Title, wchar_t const * Cmd, wchar_t const * init_string)
{
	return nullptr;
// 	return helper_menu(pluginMenu, Title, MENU_ID_STRING,
// 		new StringItem(Cmd, init_string));
}

//===========================================================================
// API: MakeMenuNOP
//===========================================================================

bb::MenuConfigItem* MakeMenuNOP(std::shared_ptr<bb::MenuConfig> menu, wchar_t const * title)
{
	std::shared_ptr<bb::MenuConfigItemSeparator> item(new bb::MenuConfigItemSeparator(title ? title : L""));
	menu->m_items.push_back(item);
	return item.get();
// 	/* BlackboxZero 1.8.2012 - For separator graident? */
// 	bb::MenuConfigItem *pItem;
// 	if (Title && Title[0]) {
// 		pItem = new bb::MenuConfigItem(NLS1(Title));
// 	} else {
// 		pItem = new SeparatorItem();
// 	}
// 	pItem->m_bNOP = true;
// 	return pluginMenu->AddMenuItem(pItem);
}

//===========================================================================
// API: MakeMenuGrip
//===========================================================================

bb::MenuConfigItem* MakeMenuGrip(std::shared_ptr<bb::MenuConfig>pluginMenu, LPCSTR Title)
{
	return nullptr;
// 	if ( Settings_menusGripEnabled )
// 		return pluginMenu->AddMenuItem(new MenuGrip(Title));
// 	return 0;
}


//===========================================================================
// API: MakeMenuItemPath
//===========================================================================

bb::MenuConfigItem* MakeMenuItemPath (std::shared_ptr<bb::MenuConfig>ParentMenu, wchar_t const * Title, wchar_t const * path, wchar_t const * Cmd)
{
	return nullptr;
// 	bb::MenuConfigItem * pItem = new SpecialFolderItem(NLS1(Title), path, NULL, Cmd);
// 	return ParentMenu->AddMenuItem(pItem);
}

bb::MenuConfigItem* MakeMenuInsertPath (std::shared_ptr<bb::MenuConfig>ParentMenu, wchar_t const * Title, wchar_t const * path, wchar_t const * Cmd)
{
	return nullptr;
// 	bb::MenuConfigItem * items = NULL;
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
/*	return NULL != bb::MenuConfig::find_named_menu(IDString_part, true);*/
}

//===========================================================================
// API: MenuItemOption - set some options for a menuitem
//===========================================================================

void MenuItemOption(bb::MenuConfigItem *pItem, int option, ...)
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
// 		std::shared_ptr<bb::MenuConfig>pSub = pItem->m_pRightmenu = va_arg(vl, bb::MenuConfig*);
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

void MenuOption(std::shared_ptr<bb::MenuConfig>pMenu, int flags, ...)
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
// 		bb::MenuConfig::Sort(&pMenu->m_pMenuItems->next, item_compare);
}

