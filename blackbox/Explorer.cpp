#include "Explorer.h"
#include <Shlwapi.h>
#include <shellapi.h>
#include <logging.h>
#include "BlackBox.h"

namespace bb {

	Explorer::Explorer ()
		: m_allocator(nullptr)
		, m_shell(nullptr)
	{ }

	bool Explorer::Init ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_EXPLORER | CTX_INIT, "Initializing explorer");
		HRESULT hr = ::SHGetMalloc(&m_allocator);

		if (::SHGetDesktopFolder(&m_shell) != NO_ERROR)
			return false;

		bool r = true;
		r &= InitControlPanel();

		return r;
	}

	bool Explorer::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_EXPLORER, "Terminating explorer");
		m_shell->Release();
		m_allocator->Release();
		return true;
	}

	void Explorer::HideExplorer (ExplorerConfig const & cfg)
	{
	}

	void Explorer::ShowExplorer ()
	{
	}

	bool Explorer::InitControlPanel ()
	{
		// https://msdn.microsoft.com/en-us/library/windows/desktop/bb761742%28v=vs.85%29.aspx
		REFKNOWNFOLDERID rfid = FOLDERID_ControlPanelFolder;
		PIDLIST_ABSOLUTE ppidl;
		HRESULT hr = SHGetKnownFolderIDList(rfid, 0, nullptr, &ppidl);
		if (hr != S_OK)
			return false;

		LPSHELLFOLDER sfFolder = NULL;
		hr = m_shell->BindToObject(ppidl, NULL, IID_IShellFolder, (void**)&sfFolder);
		if (hr != S_OK)
		{
			m_allocator->Free(ppidl);
			return false;
		}

		LPENUMIDLIST enumIDL = NULL; // IEnumIDList interface for reading contents
		hr = sfFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumIDL);
		if (S_OK != hr)
		{
			m_allocator->Free(ppidl);
			return false;
		}

		PITEMID_CHILD pidl;
		while (enumIDL->Next(1, &pidl, NULL) == S_OK)
		{
			STRRET strDispName;

			if (sfFolder->GetDisplayNameOf(pidl, SHGDN_NORMAL, &strDispName) == S_OK)
			{
				IExtractIcon * iico = nullptr;
				if (S_OK == sfFolder->GetUIObjectOf(NULL, 1, (PCITEMID_CHILD*)&pidl, IID_IExtractIcon, NULL, (void**)&iico))
				{
					PIDLIST_ABSOLUTE final_pidl = ILCombine(ppidl, pidl);
					wchar_t icofile[1024];
					int idx = -1;
					UINT flags = -1;
					HRESULT hr_ico = iico->GetIconLocation(GIL_ASYNC/* | GIL_DEFAULTICON*/, icofile, 1024, &idx, &flags);
					IconId sml_id;
					IconId lrg_id;

					wchar_t tmp_name[1024];
					StrRetToBuf(&strDispName, pidl, tmp_name, 1024);
					bbstring name(tmp_name);
					if (hr_ico == E_PENDING)
					{
						TRACE_MSG(LL_ERROR, CTX_INIT | CTX_EXPLORER, "PENDING! %ws icofile=%ws idx=%i", tmp_name, icofile, idx);
					}
					else
					{
						HICON lrg = nullptr;
						HICON sml = nullptr;
						HRESULT hr_ex_ico = iico->Extract(icofile, idx, &lrg, &sml, MAKELONG(32, 16));
						//DBG(L"%ws icofile=%ws idx=%i", tmp_name, icofile, idx);
						if (hr_ex_ico == S_OK)
						{
							BlackBox::Instance().m_gfx.AddIconToCache(name, sml, sml_id);
							BlackBox::Instance().m_gfx.AddIconToCache(name, lrg, lrg_id);

							ExplorerItem ei(final_pidl, bbstring(tmp_name), bbstring(icofile), idx, sml_id, lrg_id);
							m_controlPanel.push_back(std::move(ei));
							
						}
						else
						{
							WORD wIndex = 0;
							if (HICON exico = ExtractAssociatedIconW(NULL, icofile, &wIndex))
							{
								//DBG(L"extracted icon!");
								IconId id;
								BlackBox::Instance().m_gfx.AddIconToCache(name, exico, id);

								if (id.m_size > 16)
									lrg_id = id;
								ExplorerItem ei(final_pidl, bbstring(tmp_name), bbstring(icofile), idx, sml_id, lrg_id);
								m_controlPanel.push_back(std::move(ei));
							}
						}
					}
			
				}

//					IQueryInfo * pLink = nullptr;
//					if (SUCCEEDED(sfFolder->GetUIObjectOf(NULL, 1, (PCITEMID_CHILD*)&pidl, IID_IQueryInfo, NULL, (void**)&pLink)))
//					{
//						TCHAR * pwszTip = nullptr;
//						pLink->GetInfoTip(0, &pwszTip);
//						if (pwszTip)
//						{
//							SHFree(pwszTip);
//						}
//						//pLink->Release();						
//					}
			}
			ILFree(pidl);
		}
		m_allocator->Free(ppidl);
		//ILFree(enumIDL);
		//ILFree(sfFolder);
		return true;
	}

	void Explorer::OnClickedAt (Pidl const & pidl)
	{
		SHELLEXECUTEINFO info;
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		info.lpIDList = (void*)pidl.m_pidl;
		info.fMask = SEE_MASK_IDLIST;
		info.lpVerb = L"Open";
		info.nShow = SW_SHOWNORMAL;
		BOOL ret = ::ShellExecuteEx(&info);
	}
}

