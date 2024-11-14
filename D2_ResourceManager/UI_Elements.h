#include <Windows.h>
#include <ShellAPI.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <shlobj.h>
#include <iostream>
#include <atlbase.h>
#include <Commdlg.h>

HWND UI_CreateWindow(HINSTANCE hInstance, TCHAR *ClassName, TCHAR *WindowName, int PosX, int PosY, int Width, int Height, DWORD style);
HWND UI_CreateButton(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID);
HWND UI_CreateTextBox(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID);
HWND UI_CreateLabel(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID);
HWND UI_CreateProgressBar(HWND hWnd, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID, int Range);
void UI_ProgressBarSetMarque(HWND hProgBar, BOOL state, int time);
void UI_ProgressBarSetProgress(HWND hProgBar, int Progress);
void UI_ProgressBarSetRange(HWND hProgBar, int Range);
void UI_SetLabelText(HWND hLabel, TCHAR *Text);
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
BOOL UI_GetFolder (LPCTSTR szTitle, LPTSTR szPath, LPCTSTR szRoot, HWND hWndOwner);
TCHAR *UI_TextBoxGetText(HWND hWnd, TCHAR *Text);
TCHAR *UI_SetElementText(HWND hWnd, TCHAR *Text);
HFONT UI_CreateFont(TCHAR *Family, int size);
void UI_SetElementFont(HWND hWnd, HFONT hFont);
HWND UI_CreateComboBox(HWND hWnd, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID);
HWND UI_CreateGroupBox(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID);