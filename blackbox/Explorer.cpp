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
// 
// 			PIDLIST_ABSOLUTE pidl;
// 			HRESULT hresult = ::SHParseDisplayName(name.c_str(), 0, &pidl, SFGAO_FOLDER, 0);
// 			if (SUCCEEDED(hresult))
// 				return GetExplorerItem(pidl, result);

			LPITEMIDLIST  pidl;
			LPSHELLFOLDER pDesktopFolder;
			if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
			{
				OLECHAR olePath[MAX_PATH];
				wcscpy(olePath, name.c_str());
				HRESULT hr = pDesktopFolder->ParseDisplayName(
					NULL,
					NULL,
					olePath,
					NULL,
					&pidl,
					NULL);
				if (FAILED(hr))
				{
					return false;
				}

				SHFILEINFO sfi = {};
				if (SHGetFileInfo(reinterpret_cast<LPCTSTR>(pidl), 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_OPENICON))
				{
					IconId id;
					bbstring icofile(sfi.szDisplayName);
					BlackBox::Instance().AddIconToCache(icofile, sfi.hIcon, id);

					IconId sml_id;
					IconId lrg_id;

					if (id.m_size > 16)
						lrg_id = id;
					/// %##^#$&% 2x icofile, only dummy for test !!! REove !!!
					wchar_t dbg[1024];
					BOOL res = SHGetPathFromIDList(pidl, dbg);
					ExplorerItem ei(pidl, dbg, icofile, sfi.iIcon, sml_id, lrg_id);
					result = std::move(ei);

				}

				// 
				// pidl now contains a pointer to an ITEMIDLIST for .\readme.txt.
				// This ITEMIDLIST needs to be freed using the IMalloc allocator
				// returned from SHGetMalloc().
				// 

				pDesktopFolder->Release();
			}

		}
		return false;
	}

// 	int GetIShellItemSysIconIndex (IShellItem * psi)
// 	{
// 		PIDLIST_ABSOLUTE pidl;
// 		int iIndex = -1;
// 
// 		if (SUCCEEDED(SHGetIDListFromObject(psi, &pidl)))
// 		{
// 			SHFILEINFO sfi{};
// 			if (SHGetFileInfo(reinterpret_cast<LPCTSTR>(pidl), 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX))
// 				iIndex = sfi.iIcon;
// 			CoTaskMemFree(pidl);
// 		}
// 		return iIndex;
// 	}

	int GetPidlSysIconIndex (PIDLIST_ABSOLUTE pidl)
	{
		int iIndex = -1;
		SHFILEINFO sfi = {};
		if (SHGetFileInfo(reinterpret_cast<LPCTSTR>(pidl), 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX))
			iIndex = sfi.iIcon;
		return iIndex;
	}

	bool Explorer::GetExplorerItem (PIDLIST_ABSOLUTE pidl, ExplorerItem & result)
	{
		SHFILEINFO sfi = { };
		if (SHGetFileInfo(reinterpret_cast<LPCTSTR>(pidl), 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_OPENICON))
		{
			IconId id;
			bbstring icofile(sfi.szDisplayName);
			BlackBox::Instance().AddIconToCache(icofile, sfi.hIcon, id);

			IconId sml_id;
			IconId lrg_id;

			if (id.m_size > 16)
				lrg_id = id;
			/// %##^#$&% 2x icofile, only dummy for test !!! REove !!!
			wchar_t tmp_name_to_remove[1024];
			BOOL res = SHGetPathFromIDList(pidl, tmp_name_to_remove);
			ExplorerItem ei(pidl, tmp_name_to_remove, icofile, sfi.iIcon, sml_id, lrg_id);
			result = std::move(ei);
			return true;
		}
		return false;
	}


	bool Explorer::KnownFolderEnumerate (REFKNOWNFOLDERID rfid, std::vector<ExplorerItem> & result)
	{
		PIDLIST_ABSOLUTE ppidl;
		HRESULT hr = SHGetKnownFolderIDList(rfid, 0, nullptr, &ppidl);
		if (hr != S_OK)
			return false;
		bool const retval = FolderEnumerate(ppidl, result);
		ILFree(ppidl);
		return retval;
	}

	bool Explorer::FolderEnumerate (PIDLIST_ABSOLUTE ppidl, std::vector<ExplorerItem> & result)
	{
		LPSHELLFOLDER sfFolder = NULL;
		HRESULT hr = m_shell->BindToObject(ppidl, NULL, IID_IShellFolder, (void**)&sfFolder);
		if (hr != S_OK)
		{
			return false;
		}

		LPENUMIDLIST enumIDL = NULL; // IEnumIDList interface for reading contents
		hr = sfFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumIDL);
		if (S_OK != hr)
		{
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
					ILFree(final_pidl);
				}
			}
			ILFree(pidl);
		}
		enumIDL->Release();
		sfFolder->Release();
		return true;

	}

	//
// 	void PrintKnownFolder(const CComPtr<IKnownFolder>& folder)
// 	{
// 		KNOWNFOLDER_DEFINITION def;
// 		HRESULT hr = folder->GetFolderDefinition(&def);
// 		if (SUCCEEDED(hr)) {
// 			std::wcout << L"Result: " << def.pszName << std::endl;
// 			FreeKnownFolderDefinitionFields(&def);
// 		}
// 		else {
// 			_com_error err(hr);
// 			std::wcout << L"Error while querying GetFolderDefinition: " << err.ErrorMessage() << std::endl;
// 		}
// 	}
// 
// 
// 	class CCoInitialize
// 	{
// 	public:
// 		CCoInitialize() : m_hr(CoInitialize(NULL)) { }
// 		~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
// 		operator HRESULT() const { return m_hr; }
// 	private:
// 		HRESULT m_hr;
// 	};
// 
// 
// 	bool test()
// 	{
// 		CCoInitialize co;
// 		CComPtr<IKnownFolderManager> knownFolderManager;
// 		HRESULT hr = knownFolderManager.CoCreateInstance(CLSID_KnownFolderManager);
// 		if (!SUCCEEDED(hr)) {
// 			_com_error err(hr);
// 			std::wcout << L"Error while creating KnownFolderManager: " << err.ErrorMessage() << std::endl;
// 			return false;
// 		}
// 
// 		CComPtr<IKnownFolder> folder;
// 		hr = knownFolderManager->FindFolderFromPath(L"C:\\Users\\All Users\\Microsoft", FFFP_NEARESTPARENTMATCH, &folder);
// 		if (SUCCEEDED(hr)) {
// 			PrintKnownFolder(folder);
// 		}
// 		else {
// 			_com_error err(hr);
// 			std::wcout << L"Error while querying KnownFolderManager for nearest match: " << err.ErrorMessage() << std::endl;
// 		}
// 
// 		// dispose it.
// 		folder.Attach(NULL);
// 
// 		hr = knownFolderManager->FindFolderFromPath(L"C:\\Users\\All Users\\Microsoft", FFFP_EXACTMATCH, &folder);
// 		if (SUCCEEDED(hr)) {
// 			PrintKnownFolder(folder);
// 		}
// 		else {
// 			_com_error err(hr);
// 			std::wcout << L"Error while querying KnownFolderManager for exact match: " << err.ErrorMessage() << std::endl;
// 		}

// 		hr = knownFolderManager->FindFolderFromPath(L"C:\\Users\\All Users\\Microsoft", FFFP_EXACTMATCH, &folder);

	void Explorer::ScanKnownFolders ()
	{
		HRESULT hr;
		PWSTR pszPath = nullptr;

		IKnownFolderManager * pkfm = nullptr;
		hr = CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pkfm));
		if (SUCCEEDED(hr))
		{
			KNOWNFOLDERID * rgKFIDs = nullptr;
			UINT cKFIDs = 0;			
			hr = pkfm->GetFolderIds(&rgKFIDs, &cKFIDs); // Get the IDs of all known folders
			if (SUCCEEDED(hr))
			{
				IKnownFolder * pkfCurrent = nullptr;
				for (unsigned i = 0; i < cKFIDs; ++i)
				{
					hr = pkfm->GetFolder(rgKFIDs[i], &pkfCurrent);
					if (SUCCEEDED(hr))
					{
						KNOWNFOLDER_DEFINITION kfd;
						hr = pkfCurrent->GetFolderDefinition(&kfd); // Get the non-localized, canonical name for the known folder from KNOWNFOLDER_DEFINITION
						if (SUCCEEDED(hr))
						{
							// Next, get the path of the known folder
							hr = pkfCurrent->GetPath(0, &pszPath);
							if (SUCCEEDED(hr))
							{
								bbstring name(kfd.pszName);
								//bbstring ico(kfd.pszIcon);

// 								if (rgKFIDs[i] == FOLDERID_ControlPanelFolder)
// 									OutputDebugString(0);

								TRACE_MSG(LL_VERBOSE, CTX_BB | CTX_INIT, "known folder list: %ws", name.c_str());
					
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

	void Explorer::OnClickedAt (PIDLIST_ABSOLUTE pidl)
	{
		SHELLEXECUTEINFO info;
		memset(&info, 0, sizeof(info));
		info.cbSize = sizeof(info);
		info.lpIDList = (void*)pidl;
		info.fMask = SEE_MASK_IDLIST;
		info.lpVerb = L"Open";
		info.nShow = SW_SHOWNORMAL;
		BOOL ret = ::ShellExecuteEx(&info);
	}

	bool Explorer::IsFolder (PIDLIST_ABSOLUTE pidl) const
	{
		IShellItem * psi = nullptr;
		HRESULT hr = SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&psi));
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

