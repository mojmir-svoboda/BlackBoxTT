#include "MenuConfig.h"
#include <blackbox/BlackBox.h>
#include <blackbox/Explorer.h>
#include <blackbox/ExplorerItem.h>
#include <blackbox/gfx/MenuWidget.h>

namespace bb {

	void MenuConfigItemFile::InitFromExplorer ()
	{
		BlackBox::Instance().GetExplorer().GetExplorerItem(m_fileName, m_fileItem);
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "file pidl=0x%llx name=%ws", m_fileItem.m_pidl.m_pidl, m_fileName.c_str());
	}

	std::shared_ptr<bb::MenuConfig> MenuConfigItemFolder::CreateSubMenuFromFolder () const
	{
		std::shared_ptr<bb::MenuConfig> sub = std::make_shared<bb::MenuConfig>();
		sub->m_id = m_folderName;
		sub->m_widgetType = MenuWidget::c_type;
		sub->m_displayName = m_folderName;

		// @TODO: perf
		std::vector<ExplorerItem> items;
		if (m_knownFolder)
			BlackBox::Instance().GetExplorer().KnownFolderEnumerate(m_folderName, items);
		else
			BlackBox::Instance().GetExplorer().FolderEnumerate(m_folderItem.m_pidl.m_pidl, items);

		for (ExplorerItem & it : items)
		{
			if (BlackBox::Instance().GetExplorer().IsFolder(it.m_pidl.m_pidl))
			{
				std::shared_ptr<bb::MenuConfigItem> fld = std::make_shared<bb::MenuConfigItemFolder>();
				MenuConfigItemFolder * f = static_cast<MenuConfigItemFolder *>(fld.get());
				f->m_name = it.m_name;
				f->m_folderName = it.m_name; // @TODO: more likely to use some explorer pidl->name fn?
				f->m_folderItem = std::move(it);

				sub->m_items.push_back(fld);
			}
			else
			{
				std::shared_ptr<bb::MenuConfigItemFile> fld = std::make_shared<bb::MenuConfigItemFile>();
				MenuConfigItemFile * f = static_cast<MenuConfigItemFile *>(fld.get());
				f->m_name = it.m_name;
				f->m_fileName = it.m_name;
				f->m_fileItem = std::move(it);

				sub->m_items.push_back(fld);
			}
		}
		return sub;
	}

	void MenuConfigItemFolder::InitFromExplorer ()
	{
		m_knownFolder = BlackBox::Instance().GetExplorer().IsKnownFolder(m_folderName);
		TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "ItemFolder %ws is known folder, expanding (folder=%ws)", m_name.c_str(), m_folderName.c_str());

		if (!m_knownFolder)
		{
			BlackBox::Instance().GetExplorer().GetExplorerItem(m_folderName, m_folderItem);
			TRACE_MSG(LL_DEBUG, CTX_BB | CTX_GFX, "folder pidl=0x%llx name=%ws", m_folderItem.m_pidl.m_pidl, m_folderName.c_str());
		}

		m_menu = CreateSubMenuFromFolder();
	}

}
