#include "PathManager.h"
#include <tchar.h>
#include <ShlObj.h>

char *PathManager::GetFileNameFromPath(char *Path)
{
	for(int i = strlen(Path) - 1; i >= 0; i--)
	{
		if(Path[i] == '\\' || Path[i] == '/')
		{
			return &Path[i+1];
		}
	}
	return Path;
}

wchar_t *PathManager::GetFileNameFromPath(wchar_t *Path)
{
	for(int i = wcslen(Path) - 1; i >= 0; i--)
	{
		if(Path[i] == '\\' || Path[i] == '/')
		{
			return &Path[i+1];
		}
	}
	return Path;
}

char *PathManager::GetDirectoryFromFilePath(char *Path, char *buf)
{
	for(int i = strlen(Path) - 1; i >= 0; i--)
	{
		if(Path[i] == '\\' || Path[i] == '/')
		{
			strncpy(buf, Path, i+1);
			return buf;
		}
	}
	return 0;
}

wchar_t *PathManager::GetDirectoryFromFilePath(wchar_t *Path, wchar_t *buf)
{
	for(int i = wcslen(Path) - 1; i >= 0; i--)
	{
		if(Path[i] == '\\' || Path[i] == '/')
		{
			wcsncpy(buf, Path, i+1);
			buf[i+1] = 0;
			return buf;
		}
	}
	return 0;
}

wchar_t *PathManager::FixPathCharacters(wchar_t *Path)
{
	int len = wcslen(Path);
	for(int i = 0; i < len; i++)
	{
		if(Path[i] == '\\' || Path[i] == '/')
		{
			Path[i] = '\\';
			if(Path[i+1] != 0 && (Path[i+1] == '\\' || Path[i+1] == '/'))
			{
				for(int j = i+2; j < len; j++)
				{
					Path[j-1] = Path[j];
				}
				Path[len-1] = 0;
				len--;
			}
		}
	}
	return Path;
}

bool PathManager::DirExists( const char *fname )
{
    if( fname == NULL || strlen(fname) == 0 )
    {
        return false;
    }
    DWORD dwAttrs = GetFileAttributesA( fname );
    if(dwAttrs != FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }
    return true;
}

bool PathManager::DirExists( const wchar_t *fname )
{
    if( fname == NULL || wcslen(fname) == 0 )
    {
        return false;
    }
    DWORD dwAttrs = GetFileAttributesW( fname );
    if(dwAttrs != FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }
    return true;
}

bool PathManager::FileExists( const char *fname )
{
    if( fname == NULL || strlen(fname) == 0 )
    {
        return false;
    }
    DWORD ftyp = GetFileAttributesA(fname);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	return true;
}

bool PathManager::FileExists( const wchar_t *fname )
{
    if( fname == NULL || wcslen(fname) == 0 )
    {
        return false;
    }
    DWORD ftyp = GetFileAttributesW(fname);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	return true;
}

BOOL PathManager::RemoveDir(wchar_t *szDir)
{
    BOOL fRet = FALSE;
    if( !szDir) return fRet;
 
    size_t tReplaceLen = 1;
    WIN32_FIND_DATAW fd = {};
 
    // Формируем строку пути для поиска файлов
	wchar_t stPath[512];
    wcscpy(stPath, szDir);
    wcscat(stPath, L"\\*");
 
    HANDLE hFind = FindFirstFileW( stPath, &fd );
	stPath[wcslen(stPath)-1] = 0;
    if( hFind != INVALID_HANDLE_VALUE)
    {
        fRet = TRUE;
        do {
			wcscat(stPath, fd.cFileName);
            // Игнорируем вхождения в каталог
            if( !(wcscmp( fd.cFileName, L".") && wcscmp( fd.cFileName, L"..")) )
			{
				stPath[wcslen(stPath)-wcslen(fd.cFileName)] = 0;
                continue;
			}
            // Формруем полный путь к файлу/каталогу
			
 
            if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                fRet = RemoveDir( stPath ); // Удаляем все потроха найденного каталога
            else
            {
                // Просто удаляем найденный файл
                if( fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    SetFileAttributesW( stPath, FILE_ATTRIBUTE_NORMAL);
                fRet = DeleteFileW( stPath );

            }
			stPath[wcslen(stPath)-wcslen(fd.cFileName)] = 0;
 
        } while( fRet && FindNextFileW( hFind, &fd ));
        FindClose( hFind );
 
        // Заключительный момент - удалить пустой каталог...
        if( fRet) fRet = RemoveDirectoryW( szDir);
    }
    return fRet;
}

BOOL PathManager::RemoveFile(wchar_t *path)
{
	if( path == NULL || wcslen(path) == 0 )
    {
        return FALSE;
    }
    DWORD ftyp = GetFileAttributesW(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return FALSE;  //something is wrong with your path!

	if(ftyp & FILE_ATTRIBUTE_READONLY)
		SetFileAttributesW(path, FILE_ATTRIBUTE_NORMAL);
	return DeleteFileW( path );
}

int PathManager::SaveFile(TCHAR *directory, TCHAR *filename, TCHAR *filter, int *filterId, TCHAR *fileExt)
{
	OPENFILENAME ofn; 
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filename;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.lpstrDefExt = fileExt;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = directory;
	ofn.Flags = OFN_PATHMUSTEXIST;
	if (GetSaveFileName(&ofn)==TRUE)
	{
		if(filterId)
			*filterId = ofn.nFilterIndex;
		if(fileExt)
			_tcscpy(fileExt, &ofn.lpstrFile[ofn.nFileExtension]);
	    return 1;
	}
	else
	{
		filename[0] = 0;
		return 0;
	}
}

int PathManager::SelectFolder(HWND hWnd, TCHAR *folder)
{
	BROWSEINFO info;
	memset(&info, 0, sizeof(info));
	info.ulFlags = BIF_USENEWUI;
	info.hwndOwner = hWnd;
	info.pszDisplayName = 0;

	int result = 0;
	LPITEMIDLIST list = SHBrowseForFolder(&info);
	if(list)
	{
		SHGetPathFromIDList(list,folder);
		result = 1;
	}
	return result;
}