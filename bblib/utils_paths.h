#pragma once
#include <bblib/bbstring.h>
//#include "utils_string.h"
#include "paths.h"
#include "unicode.h"
#include <Shlwapi.h>
#include <Pathcch.h>

namespace bb {

inline bool is_slash (TCHAR c) { return c == L'\\' || (c) == L'/'; }

inline bool splitPath (wchar_t const * path, wchar_t * drive, size_t drv_sz, wchar_t * dir, size_t dir_sz, wchar_t * fname, size_t fname_sz, wchar_t * fext, size_t fext_sz)
{
	TCHAR buff_drive[_MAX_DRIVE];
	TCHAR buff_dir[_MAX_DIR];
	TCHAR buff_fname[_MAX_FNAME];
	TCHAR buff_fext[_MAX_EXT];
	errno_t const res = _tsplitpath_s(path, buff_drive, buff_dir, buff_fname, buff_fext);
	if (res == 0)
	{
		_tcsncpy(drive, buff_drive, drv_sz);
		_tcsncpy(dir, buff_dir, dir_sz);
		_tcsncpy(fname, buff_fname, fname_sz);
		_tcsncpy(fext, buff_fext, fext_sz);
		return true;
	}
	return false;
}
inline bool splitPath (bbstring const & path, bbstring & drive, bbstring & dir, bbstring & fname, bbstring & fext)
{
	TCHAR buff_drive[_MAX_DRIVE];
	TCHAR buff_dir[_MAX_DIR];
	TCHAR buff_fname[_MAX_FNAME];
	TCHAR buff_fext[_MAX_EXT];
	errno_t const res = _tsplitpath_s(path.c_str(), buff_drive, buff_dir, buff_fname, buff_fext);
	if (res == 0)
	{
		drive = std::move(bbstring(buff_drive));
		dir = std::move(bbstring(buff_dir));
		fname = std::move(bbstring(buff_fname));
		fext = std::move(bbstring(buff_fext));
		return true;
	}
	return false;
}

inline bool makePath (wchar_t const * drive, wchar_t const * dir, wchar_t const * fname, wchar_t const * fext, wchar_t * path, size_t pathsz)
{
	TCHAR buff_path[_MAX_PATH];
	//BB_ASSERT(pathsz >= _MAX_PATH);
	path[0]='\0';
	errno_t const res = _tmakepath_s(buff_path, drive, dir, fname, fext);
	if (res == 0)
	{
		_tcsncpy(path, buff_path, pathsz);
		return true;
	}
	return false;
}
inline bool makePath (bbstring const & drive, bbstring const & dir, bbstring const & fname, bbstring const & fext, bbstring & path)
{
	TCHAR buff_path[_MAX_PATH];
	errno_t const res = _tmakepath_s(buff_path, drive.c_str(), dir.c_str(), fname.c_str(), fext.c_str());
	if (res == 0)
	{
		path = std::move(bbstring(buff_path));
		return true;
	}
	return false;
}

inline bool combinePath (wchar_t const * filedir, wchar_t const * filename, wchar_t * buff, size_t buffsz)
{
	HRESULT const hr = ::PathCchCombineEx(buff, buffsz, filedir, filename, 0);
	return SUCCEEDED(hr);
}


/* concatenate directory / file */
inline bool joinPath (wchar_t const * filedir, wchar_t const * filename, wchar_t * buff, size_t buffsz)
{
	TCHAR drive0[_MAX_DRIVE];
	TCHAR dir0[_MAX_DIR];
	TCHAR fname0[_MAX_FNAME];
	TCHAR fext0[_MAX_EXT];
	if (!splitPath(filedir, drive0, _MAX_DRIVE, dir0, _MAX_DIR, fname0, _MAX_FNAME, fext0, _MAX_EXT))
		return false;

	TCHAR drive1[_MAX_DRIVE];
	TCHAR dir1[_MAX_DIR];
	TCHAR fname1[_MAX_FNAME];
	TCHAR fext1[_MAX_EXT];
	if (!splitPath(filename, drive1, _MAX_DRIVE, dir1, _MAX_DIR, fname1, _MAX_FNAME, fext1, _MAX_EXT))
		return false;

	return makePath(drive0, dir0, fname1, fext1, buff, buffsz);
}
inline bool joinPath(bbstring const & filedir, bbstring const & filename, bbstring & out_str)
{
	tstring drive, dir, fname, ext;
	if (!splitPath(filedir, drive, dir, fname, ext))
		return false;

	tstring drive2, dir2, fname2, ext2;
	if (!splitPath(filename, drive2, dir2, fname2, ext2))
		return false;

	return makePath(drive, dir, fname2, ext2, out_str);
}

inline bool fileExists (bbstring const & fname)
{
	return fileExists(fname.c_str());
}
inline bool fileExists (TCHAR const * szFileName)
{
	DWORD a = ::GetFileAttributes(szFileName);
	return (DWORD)-1 != a && 0 == (a & FILE_ATTRIBUTE_DIRECTORY);
}

inline bool isAbsolutePath (wchar_t const * path)
{
	BOOL const res = ::PathIsRelative(path);
	return res == FALSE;
}
inline bool isAbsolutePath (bbstring const & path)
{
	return isAbsolutePath(path.c_str());
}

inline void getFileBasename (bbstring const & filename, bbstring & basename)
{
	bbstring drive, dir, fname, ext;
	splitPath(filename, drive, dir, fname, ext);
	basename.append(fname);
	basename.append(ext);
}

inline bool getFileDirectory (bbstring const & path, bbstring & out_str)
{
	bbstring drive, dir, fname, fext;
	if (splitPath(path, drive, dir, fname, fext))
	{
		out_str = drive + dir;
		return true;
	}
	return false;
}
inline bool getFileDirectory (wchar_t const * path, wchar_t * filedir, size_t filedirsz)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR fext[_MAX_EXT];
	if (splitPath(path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT))
	{
		_snwprintf_s(filedir, filedirsz, _TRUNCATE, L"%s%s", drive, dir);
		return true;
	}
	return false;
}

inline void getFileExtension (bbstring const & path, bbstring & ext)
{
	bbstring drive, dir, fname;
	splitPath(path, drive, dir, fname, ext);
}
inline void getFileExtension (wchar_t const * path, wchar_t * ext, size_t extsz)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR fext[_MAX_EXT];
	splitPath(path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT);
	_tcsncpy(ext, fext, extsz);
}

inline bool getExeName (HINSTANCE h, TCHAR * dest, size_t destSize)
{
	if (!dest)
		return false;
// 	TCHAR shortPath[_MAX_PATH];
// 	::GetModuleFileName(h, shortPath, _MAX_PATH);
// 	TCHAR longPath[_MAX_PATH];
// 	::GetLongPathName(shortPath, longPath, _MAX_PATH);
	DWORD length = ::GetModuleFileName(h, dest, destSize);
	if (MAX_PATH > destSize)
		return false;
//	PathRemoveFileSpec(dest);
	return true;
}
inline bool getExeName (TCHAR * dest, size_t destSize)
{
	return getExeName(nullptr, dest, destSize);
}

inline bool getExePath (HINSTANCE h, wchar_t * dir, size_t dirsz)
{
	wchar_t fname[MAX_PATH];
	if (getExeName(h, fname, MAX_PATH))
		return getFileDirectory(fname, dir, dirsz);
	return false;
}
inline bool getExePath (TCHAR * dest, size_t destSize)
{
	return getExePath(nullptr, dest, destSize);
}


/* ------------------------------------------------------------------------- */
/* Function: get_relative_path */
/* get the sub-path, if the path is in the HINSTANCE folder, */
/* In:       path to check */
/* Out:      pointer to subpath or full path otherwise. */
/* ------------------------------------------------------------------------- */
inline void getRelativePath (HINSTANCE h, bbstring const & path, bbstring & rel_path)
{
	wchar_t basedir[MAX_PATH];
	getExePath(h, basedir, MAX_PATH);
	size_t const base_len = _tcslen(basedir);
	if (base_len && 0 == _tcsnicmp(path.c_str(), basedir, base_len))
		rel_path = path.substr(base_len, std::string::npos);
	else
		rel_path = path;
}

}