#include "UI_Elements.h"

HWND UI_CreateWindow(HINSTANCE hInstance, TCHAR *ClassName, TCHAR *WindowName, int PosX, int PosY, int Width, int Height, DWORD style)
{
	HWND hWnd;
	hWnd = CreateWindow(ClassName, WindowName, style, PosX, PosY, Width, Height, NULL, NULL, hInstance, NULL);
	return hWnd;
}

HWND UI_CreateButton(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID)
{
	HWND hBtn;
	hBtn = CreateWindow(TEXT("BUTTON"), Text, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP | style, PosX, PosY, Width, Height, hWnd, (HMENU)ElementID, NULL, NULL);
	return hBtn;
}

HWND UI_CreateTextBox(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID)
{
	HWND hEdit;
	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT") , Text, WS_VISIBLE | WS_CHILD | style, PosX, PosY, Width, Height, hWnd, (HMENU)ElementID, NULL, NULL);
	return hEdit;
}

TCHAR *UI_TextBoxGetText(HWND hWnd, TCHAR *Text)
{
	SendMessage(hWnd,WM_GETTEXT,80,LPARAM(Text));
	return Text;
}

TCHAR *UI_SetElementText(HWND hWnd, TCHAR *Text)
{
	SetWindowText(hWnd, Text);
	return Text;
}

HWND UI_CreateLabel(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID)
{
	HWND hLabel;	
	hLabel = CreateWindow(TEXT("STATIC"), Text, WS_VISIBLE | WS_CHILD | SS_LEFT | style, PosX, PosY, Width, Height, hWnd, (HMENU)ElementID, NULL, NULL);
	return hLabel;
}

void UI_SetLabelText(HWND hLabel, TCHAR *Text)
{
	//SendMessage(hLabel, , (WPARAM)Text, 0);
	SetWindowText(hLabel, Text);
	InvalidateRect(hLabel, NULL, FALSE);
	UpdateWindow(hLabel);
}

HWND UI_CreateProgressBar(HWND hWnd, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID, int Range)
{
	HWND hProgBar;
	hProgBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | style,
                  PosX, PosY, Width, Height, hWnd, (HMENU)ElementID, NULL, NULL);
	SendMessage(hProgBar, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0,Range));
	SendMessage(hProgBar, PBM_SETSTEP, (WPARAM)1, 0);
	return hProgBar;
}

void UI_ProgressBarSetMarque(HWND hProgBar, BOOL state, int time)
{
	if(state)
	{
		LONG style = GetWindowLong(hProgBar, GWL_STYLE);
		if(!(style & PBS_MARQUEE))
		{
			SetWindowLong(hProgBar, GWL_STYLE, style | PBS_MARQUEE);
		}
		SendMessage(hProgBar, PBM_SETMARQUEE, (WPARAM)1, time);
	}
	else
	{
		LONG style = GetWindowLong(hProgBar, GWL_STYLE);
		if(style & PBS_MARQUEE)
			SetWindowLong(hProgBar, GWL_STYLE, style ^ PBS_MARQUEE);
	}
}

void UI_ProgressBarSetProgress(HWND hProgBar, int Progress)
{
	SendMessage(hProgBar, PBM_SETPOS, Progress, 0);
}

void UI_ProgressBarSetRange(HWND hProgBar, int Range)
{
	SendMessage(hProgBar, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0,Range));
}
/*
//Позволяет взять имя открываемого файла в стандартном диалоге Windows
BOOL UI_FileOpenDlg(HWND hWnd, LPTSTR pstrFileName, LPTSTR Filter, DWORD Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, LPTSTR InitDir = NULL, LPTSTR pstrTitleName=NULL)
{
	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = pstrFileName ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 512;
	ofn.lpstrFilter = Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = InitDir;
	ofn.Flags = Flags;
	return GetOpenFileName (&ofn);
}

static int CALLBACK BrowseCallbackProc (HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    TCHAR szPath[_MAX_PATH];
    switch (uMsg) {
    case BFFM_INITIALIZED:
        if (lpData)
            SendMessage(hWnd,BFFM_SETSELECTION,TRUE,lpData);
        break;
    case BFFM_SELCHANGED:
        SHGetPathFromIDList(LPITEMIDLIST(lParam),szPath);
        SendMessage(hWnd, BFFM_SETSTATUSTEXT, NULL, LPARAM(szPath));
        break;
    }
    return 0;
}

BOOL UI_GetFolder (LPCTSTR szTitle, LPTSTR szPath, LPCTSTR szRoot, HWND hWndOwner)
{
    if (szPath == NULL)
        return false;

    bool result = false;

    LPMALLOC pMalloc;
    if (::SHGetMalloc(&pMalloc) == NOERROR) {
        BROWSEINFO bi;
        ::ZeroMemory(&bi,sizeof bi);
        bi.ulFlags   = BIF_RETURNONLYFSDIRS;

        // дескриптор окна-владельца диалога
        bi.hwndOwner = hWndOwner;

        // добавление заголовка к диалогу
        bi.lpszTitle = szTitle;

        // отображение текущего каталога
        bi.lpfn      = BrowseCallbackProc;
        bi.ulFlags  |= BIF_STATUSTEXT;

        // установка каталога по умолчанию
        bi.lParam    = LPARAM(szPath);

        // установка корневого каталога
        if (szRoot != NULL) {
            IShellFolder *pDF;
            if (SHGetDesktopFolder(&pDF) == NOERROR) {
                LPITEMIDLIST pIdl = NULL;
                ULONG        chEaten;
                ULONG        dwAttributes;

                USES_CONVERSION;
                LPOLESTR oleStr = T2OLE((LPTSTR)szRoot);

                pDF->ParseDisplayName(NULL,NULL,oleStr,&chEaten,&pIdl,&dwAttributes);
                pDF->Release();

                bi.pidlRoot = pIdl;
            }
        }

        LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
        if (pidl != NULL) {
            if (::SHGetPathFromIDList(pidl,szPath))
                result = true;
            pMalloc->Free(pidl);
        }
        if (bi.pidlRoot != NULL)
            pMalloc->Free((void*)bi.pidlRoot);
        pMalloc->Release();
    }
    return result;
}
*/
HFONT UI_CreateFont(TCHAR *Family, int size)
{
	return CreateFont (size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_DONTCARE, Family);
}

void UI_SetElementFont(HWND hWnd, HFONT hFont)
{
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

HWND UI_CreateComboBox(HWND hWnd, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID)
{
	HWND hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""), 
		 CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | style,
		 PosX, PosY, Width, Height, hWnd, (HMENU)ElementID, NULL,
		 NULL);
	return hWndComboBox;
}

HWND UI_CreateGroupBox(HWND hWnd, TCHAR *Text, int PosX, int PosY, int Width, int Height, DWORD style, int ElementID)
{
	HWND hWndGroupBox = UI_CreateButton(hWnd, Text, PosX, PosY, Width, Height, style | BS_GROUPBOX, ElementID);
	return hWndGroupBox;
}
/*
void UI_ComboBoxAddRow(HWND hWnd, TCHAR *row)
{
	SendMessage(hWnd,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) row); 
}

void UI_ComboBoxSetActiveItem(HWND hWnd, int row)
{
	SendMessage(hWnd, CB_SETCURSEL, (WPARAM)row, (LPARAM)0);
}*/