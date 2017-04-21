#include "Explorer.h"
#include <common.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <logging.h>
#include "BlackBox.h"

namespace bb {

	Explorer::Explorer ()
		: m_allocator(nullptr)
		, m_shell(nullptr)
	{ }

	bool Explorer::Init ()
	{
		TRACE_MSG(LL_INFO, CTX_BB | CTX_INIT, "Initializing explorer");
		HRESULT hr = ::SHGetMalloc(&m_allocator);

		if (::SHGetDesktopFolder(&m_shell) != NO_ERROR)
			return false;

		bool r = true;
		ScanKnownFolders();
		r &= InitControlPanel();
		r &= InitStartMenu();

		return r;
	}

	bool Explorer::Done ()
	{
		TRACE_MSG(LL_INFO, CTX_BB, "Terminating explorer");
		m_controlPanel.clear();
		m_startMenu.clear();
		if (m_shell)
		{
			m_shell->Release();
			m_shell = nullptr;
		}
		if (m_allocator)
		{
			m_allocator->Release();
			m_allocator= nullptr;
		}
		return true;
	}

	void Explorer::HideExplorer (ExplorerConfig const & cfg)
	{
	}

	void Explorer::ShowExplorer ()
	{
	}

	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb761742%28v=vs.85%29.aspx
	bool Explorer::InitControlPanel ()
	{
		REFKNOWNFOLDERID rfid = FOLDERID_ControlPanelFolder;
		return KnownFolderEnumerate(rfid, m_controlPanel);
	}
	bool Explorer::InitStartMenu ()
	{
		REFKNOWNFOLDERID rfid = FOLDERID_StartMenu;
		return KnownFolderEnumerate(rfid, m_startMenu);
	}

	bool Explorer::KnownFolderEnumerate (bbstring const & name, std::vector<ExplorerItem> & result)
	{
		auto it = m_knownFolders.find(name);
		if (it != m_knownFolders.end())
		{
			KnwnFldr const & info = it->second;
			return KnownFolderEnumerate(info.m_rfid, result);
		}
		return false;
	}

	bool Explorer::IsKnownFolder (bbstring const & name) const
	{
		auto it = m_knownFolders.find(name);
		if (it != m_knownFolders.end())
			return true;
		return false;
	}

	bool Explorer::KnownFolder (bbstring const & name, std::vector<ExplorerItem> & result)
	{
		auto it = m_knownFolders.find(name);
		if (it != m_knownFolders.end())
		{
			KnwnFldr const & info = it->second;

			//LPWSTR tmp = info.
			// info.m_rfid --> ExplorerItem
			//return KnownFolderEnumerate(info.m_rfid, result);
		}
		return false;
	}

	bool Explorer::GetExplorerItem (bbstring const & name, ExplorerItem & result)
	{
		auto it = m_knownFolders.find(name);
		if (it != m_knownFolders.end())
		{
			KnwnFldr const & info = it->second;

			//LPWSTR tmp = info.
			// info.m_rfid --> ExplorerItem
			//return KnownFolderEnumerate(info.m_rfid, result);

			bbstring ico;
			//

			//void Explorer::ScanKnownFolders()
			{
				HRESULT hr;
				PWSTR pszPath = nullptr;

				// Create an IKnownFolderManager instance
				IKnownFolderManager * pkfm = nullptr;
				hr = CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
				if (SUCCEEDED(hr))
				{
					IKnownFolder * pkfCurrent = nullptr;
					//hr = pkfm->FindFolderFromPath(name.c_str(), FFFP_EXACTMATCH, &pkfCurrent);
					hr = pkfm->GetFolderByName(name.c_str(), &pkfCurrent);
					if (SUCCEEDED(hr))
					{
						// Get the non-localized, canonical name for the known folder from KNOWNFOLDER_DEFINITION
						KNOWNFOLDER_DEFINITION kfd;
						hr = pkfCurrent->GetFolderDefinition(&kfd);
						if (SUCCEEDED(hr))
						{
							// Next, get the path of the known folder
							hr = pkfCurrent->GetPath(0, &pszPath);
							if (SUCCEEDED(hr))
							{
								//bbstring name(kfd.pszName);
								//ico = bbstring(kfd.pszIcon);
								LPWSTR icofile = kfd.pszIcon;

								if (icofile)
								{

									PIDLIST_ABSOLUTE pidl;
									HRESULT hr = SHGetKnownFolderIDList(info.m_rfid, 0, nullptr, &pidl);
									if (hr != S_OK)
										return false;


									WORD wIndex = 0;
									if (HICON exico = ExtractAssociatedIconW(NULL, kfd.pszIcon, &wIndex))
									{
										//DBG(L"extracted icon!");
										IconId sml_id;
										IconId lrg_id;
										IconId id;
										BlackBox::Instance().AddIconToCache(name, exico, id);

										if (id.m_size > 16)
											lrg_id = id;
										ExplorerItem ei(pidl, name, bbstring(kfd.pszIcon), id.m_index, sml_id, lrg_id);
										result = std::move(ei);
									}
								}
								CoTaskMemFree(pszPath);
							}
							FreeKnownFolderDefinitionFields(&kfd);
						}
						pkfCurrent->Release();
					}
				}
				pkfm->Release();
			}

			//return GetExplorerItem(pidl, result);
		}
		else
		{

		}
		return false;
	}

	bool Explorer::GetExplorerItem (PIDLIST_ABSOLUTE pidl, ExplorerItem & result)
	{
		SHFILEINFOW sfi = { 0 };



// 		DWORD = SHGetFileInfo((LPCTSTR)pidl,
// 			-1,
// 			&sfi,
// 			sizeof(sfi),
// 			SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_OPENICON);
// 
// 			if (SUCCEEDED(hr))
// 			{
// 				//sfi.iIcon
// 				
// 			}
// 		LPSHELLFOLDER sfFolder = NULL;
// 		HRESULT hr = m_shell->BindToObject(ppidl, NULL, IID_IShellFolder, (void**)&sfFolder);
// 		if (hr != S_OK)
// 		{
// 			m_allocator->Free(ppidl);
// 			return false;
// 		}
// 
// // 		LPENUMIDLIST enumIDL = NULL; // IEnumIDList interface for reading contents
// // 		hr = sfFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumIDL);
// // 		if (S_OK != hr)
// // 		{
// // 			m_allocator->Free(ppidl);
// // 			return false;
// // 		}
// // 
// // 		PITEMID_CHILD pidl;
// // 		while (enumIDL->Next(1, &pidl, NULL) == S_OK)
// // 		{
// // 			STRRET strDispName;
// // 
// 			STRRET strDispName;
// 			HRESULT hr2 = sfFolder->GetDisplayNameOf(ppidl, SHGDN_NORMAL, &strDispName);
// 			if (hr2 == S_OK)
// 			{
// 				IExtractIcon * iico = nullptr;
// 				if (S_OK == sfFolder->GetUIObjectOf(NULL, 1, (PCITEMID_CHILD*)&ppidl, IID_IExtractIcon, NULL, (void**)&iico))
// 				{
// 					//PIDLIST_ABSOLUTE final_pidl = ILCombine(ppidl, pidl);
// 					wchar_t icofile[1024];
// 					int idx = -1;
// 					UINT flags = -1;
// 					HRESULT hr_ico = iico->GetIconLocation(GIL_ASYNC/* | GIL_DEFAULTICON*/, icofile, 1024, &idx, &flags);
// 					IconId sml_id;
// 					IconId lrg_id;
// 
// 					wchar_t tmp_name[1024];
// 					StrRetToBuf(&strDispName, ppidl, tmp_name, 1024);
// 					bbstring name(tmp_name);
// 					if (hr_ico == E_PENDING)
// 					{
// 						TRACE_MSG(LL_ERROR, CTX_BB | CTX_INIT, "PENDING! %ws icofile=%ws idx=%i", tmp_name, icofile, idx);
// 					}
// 					else
// 					{
// 						HICON lrg = nullptr;
// 						HICON sml = nullptr;
// 						HRESULT hr_ex_ico = iico->Extract(icofile, idx, &lrg, &sml, MAKELONG(32, 16));
// 						//DBG(L"%ws icofile=%ws idx=%i", tmp_name, icofile, idx);
// 						if (hr_ex_ico == S_OK)
// 						{
// 							BlackBox::Instance().AddIconToCache(name, sml, sml_id);
// 							BlackBox::Instance().AddIconToCache(name, lrg, lrg_id);
// 
// 							ExplorerItem ei(ppidl, bbstring(tmp_name), bbstring(icofile), idx, sml_id, lrg_id);
// 							result = std::move(ei);
// 
// 						}
// 						else
// 						{
// 							WORD wIndex = 0;
// 							if (HICON exico = ExtractAssociatedIconW(NULL, icofile, &wIndex))
// 							{
// 								//DBG(L"extracted icon!");
// 								IconId id;
// 								BlackBox::Instance().AddIconToCache(name, exico, id);
// 
// 								if (id.m_size > 16)
// 									lrg_id = id;
// 								ExplorerItem ei(ppidl, bbstring(tmp_name), bbstring(icofile), idx, sml_id, lrg_id);
// 								result = std::move(ei);
// 							}
// 						}
// 					}
// 				}
// 			}
// // 			ILFree(ppidl);
// // 		}
// 		m_allocator->Free(ppidl);
// 		//ILFree(enumIDL);
// 		//ILFree(sfFolder);
		return true;
	}


	bool Explorer::KnownFolderEnumerate (REFKNOWNFOLDERID rfid, std::vector<ExplorerItem> & result)
	{
		PIDLIST_ABSOLUTE ppidl;
		HRESULT hr = SHGetKnownFolderIDList(rfid, 0, nullptr, &ppidl);
		if (hr != S_OK)
			return false;
		return FolderEnumerate(ppidl, result);
	}

	bool Explorer::FolderEnumerate (PIDLIST_ABSOLUTE ppidl, std::vector<ExplorerItem> & result)
	{
		LPSHELLFOLDER sfFolder = NULL;
		HRESULT hr = m_shell->BindToObject(ppidl, NULL, IID_IShellFolder, (void**)&sfFolder);
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
						TRACE_MSG(LL_ERROR, CTX_BB | CTX_INIT, "PENDING! %ws icofile=%ws idx=%i", tmp_name, icofile, idx);
					}
					else
					{
						HICON lrg = nullptr;
						HICON sml = nullptr;
						HRESULT hr_ex_ico = iico->Extract(icofile, idx, &lrg, &sml, MAKELONG(32, 16));
						//DBG(L"%ws icofile=%ws idx=%i", tmp_name, icofile, idx);
						if (hr_ex_ico == S_OK)
						{
							BlackBox::Instance().AddIconToCache(name, sml, sml_id);
							BlackBox::Instance().AddIconToCache(name, lrg, lrg_id);

							result.emplace_back(final_pidl, bbstring(tmp_name), bbstring(icofile), idx, sml_id, lrg_id);
							
						}
						else
						{
							WORD wIndex = 0;
							if (HICON exico = ExtractAssociatedIconW(NULL, icofile, &wIndex))
							{
								//DBG(L"extracted icon!");
								IconId id;
								BlackBox::Instance().AddIconToCache(name, exico, id);

								if (id.m_size > 16)
									lrg_id = id;
								result.emplace_back(final_pidl, bbstring(tmp_name), bbstring(icofile), idx, sml_id, lrg_id);
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

	void Explorer::ScanKnownFolders ()
	{
		HRESULT hr;
		PWSTR pszPath = nullptr;

		// Create an IKnownFolderManager instance
		IKnownFolderManager * pkfm = nullptr;
		hr = CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
		if (SUCCEEDED(hr))
		{
			KNOWNFOLDERID * rgKFIDs = nullptr;
			UINT cKFIDs = 0;
			// Get the IDs of all known folders
			hr = pkfm->GetFolderIds(&rgKFIDs, &cKFIDs);
			if (SUCCEEDED(hr))
			{
				IKnownFolder * pkfCurrent = nullptr;
				// Enumerate the known folders. rgKFIDs[i] has the KNOWNFOLDERID
				for (unsigned i = 0; i < cKFIDs; ++i)
				{
					hr = pkfm->GetFolder(rgKFIDs[i], &pkfCurrent);
					if (SUCCEEDED(hr))
					{
						// Get the non-localized, canonical name for the known folder from KNOWNFOLDER_DEFINITION
						KNOWNFOLDER_DEFINITION kfd;
						hr = pkfCurrent->GetFolderDefinition(&kfd);
						if (SUCCEEDED(hr))
						{
							// Next, get the path of the known folder
							hr = pkfCurrent->GetPath(0, &pszPath);
							if (SUCCEEDED(hr))
							{
								bbstring name(kfd.pszName);
								//bbstring ico(kfd.pszIcon);

								if (rgKFIDs[i] == FOLDERID_StartMenu)
									OutputDebugString(0);

								///if (name == L"Start Menu"),KB
					
								//bbstring path(kfd.pszRelativePath);GH


								//ExplorerItem (LPITEMIDLIST pidl, bbstring const & name, bbstring const & ico, int icoidx, IconId sml_id, IconId lrg_id)
								//m_knownFolders[name] = KnwnFldr { *rgKFIDs, ExplorerItem { } };
								m_knownFolders[name] = KnwnFldr { rgKFIDs[i] };
								CoTaskMemFree(pszPath);
							}
							FreeKnownFolderDefinitionFields(&kfd);
						}
						pkfCurrent->Release();
					}
				}
				CoTaskMemFree(rgKFIDs);
			}
			pkfm->Release();
		}
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

	bool Explorer::IsFolder (Pidl const & pidl) const
	{
		IShellItem *psi;
		HRESULT hr = SHCreateItemFromIDList(pidl.m_pidl, IID_PPV_ARGS(&psi));
		if (SUCCEEDED(hr))
		{
			DWORD dwAttr = 0;
			hr = psi->GetAttributes(SFGAO_STREAM | SFGAO_FOLDER, &dwAttr);
			if (SUCCEEDED(hr))
			{
					bool const is_dir = dwAttr & SFGAO_FOLDER;
					return is_dir;
				}
			psi->Release();
		}
		return false;
	}
}

