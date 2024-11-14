#include <Windows.h>
#include <string.h>

class PathManager
{
public:
	static char *GetFileNameFromPath(char *Path);
	static wchar_t *GetFileNameFromPath(wchar_t *Path);
	static char *GetDirectoryFromFilePath(char *Path, char *buf);
	static wchar_t *GetDirectoryFromFilePath(wchar_t *Path, wchar_t *buf);
	static wchar_t *FixPathCharacters(wchar_t *Path);
	static bool DirExists( const char *fname );
	static bool DirExists( const wchar_t *fname );
	static bool PathManager::FileExists( const char *fname );
	static bool PathManager::FileExists( const wchar_t *fname );
	static BOOL PathManager::RemoveDir(wchar_t *szDir);
	static BOOL PathManager::RemoveFile(wchar_t *path);
	static int SaveFile(TCHAR *directory, TCHAR *filename, TCHAR *filter, int *filterId, TCHAR *fileExt);
	static int SelectFolder(HWND hWnd, TCHAR *folder);
};