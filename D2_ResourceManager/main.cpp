//���������� ����������
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
//#pragma comment(lib,"glaux.lib")
//#pragma comment(lib,"glut32.lib")

// ������
#pragma comment(lib,"shell32")
#pragma comment(lib,"comctl32.lib")
//#include <ShellAPI.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//���������� ������������ �����
#include <windows.h>
#include <windowsx.h>
#include "OpenGL.h"
#include <iostream>
//#include "UI.h"
#include "KingStructures.h"
//#include "CNode.h"
#include "Targa.h"
#include "PathManager.h"
#include <tchar.h>

//���������� ����������
HINSTANCE g_hInstance = NULL;      //���������� ����������
HWND g_hWnd = NULL;            //���������� ����
HWND g_hGlWnd = NULL;
int g_iWindowWidth = 920;        //������ ����
int g_iWindowHeight = 600;        //������ ����
int g_iGlWidth = 600;
int g_iGlHeight = 450;
bool g_bApplicationState = true;    //��������� ���������� (true - ��������/false - �� ��������)
extern ResTexture *currentTex;
#define ID_FIRSTCHILD 3001
#define ID_BUTTON_TEX_EXPORT 3002

#include "UI_Elements.h"

HWND g_hPos;
HWND g_hCamPos;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int iCmdShow); //����� ������ ����������
long WINAPI WndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);//���������� ���������
long WINAPI GroupProc(HWND hWNd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void Shutdown();                          //����������� ������
void CALLBACK OnWinTreeTextureEvent(WinTree *tree, WinTreeItem *item, DWORD eventId);


long WINAPI ChildProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	return DefWindowProc(hWnd,iMsg,wParam,lParam);  //���� ���� ��� ��� ������ ���������, ����� ��� ������������ �������
}

WinTree *g_pTree = 0;
extern Node<CModel> *currentGlModel;

WNDPROC GroupProcOrigin;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int iCmdShow)
{  
	CoInitialize(0);
	g_hInstance = GetModuleHandle(NULL);
	currentGlModel = new Node<CModel>(0, 0);
	
	WNDCLASSEX wc;
	wc.cbSize      = sizeof(WNDCLASSEX);        //������ ���������
	wc.style           = CS_HREDRAW|CS_VREDRAW;      //����� ������ ����
	wc.lpfnWndProc    = WndProc;              //������� ��������� ���������
	wc.cbClsExtra    = 0;                //���������� ���������� ������ ��� �������� ����������
	wc.cbWndExtra      = 0;                //���������� ���������� ������ ��� �������� ����������
	wc.hInstance    = g_hInstance;            //���������� ����������
	wc.hIcon           = LoadIcon(NULL,IDI_APPLICATION);  //��������� ����������� ������
	wc.hCursor         = LoadCursor(0,IDC_ARROW);      //��������� ����������� ������
	wc.hbrBackground   = (HBRUSH)GetStockObject(LTGRAY_BRUSH);//���� ����� ��������� � ����� ����
	wc.lpszMenuName    = 0;                //�� ���������� ����
	wc.lpszClassName   = "KingResourceManager";            //�������� ������
	wc.hIconSm       = LoadIcon(NULL,IDI_APPLICATION);  //��������� ����������� ������
	
	if(!RegisterClassEx(&wc))                //������������ ����� � Windows
	{
		Shutdown();                    //����������� ������
		MessageBox(NULL,"Can`t register window class","Error",MB_OK|MB_ICONERROR); //������� ���������
		return 0;                    //��������� ������ ����������
	}

	g_hWnd = CreateWindowEx(              //������� ����
		WS_EX_APPWINDOW|WS_EX_WINDOWEDGE,        //����������� ����� ����
		"KingResourceManager",                    //�������� ������ ����
		"�������� �������� \"������������� 2\" by BoPoH (vk.com/db2_game)",    //�������� ����
		WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS,//����� ����
		0,                      //������� ���� �� ��� �
		0,                      //������� ���� �� ��� �
		g_iWindowWidth,                //������ ����
		g_iWindowHeight,              //������ ����
		NULL,                    //��� ���� ������� ����
		NULL,                    //���� ����
		g_hInstance,                //���������� ����������
		NULL);                    //�������������� �������� �� ����������

	if(g_hWnd == NULL)                //���� �� ������� ����
	{
		Shutdown();
		MessageBox(NULL,"Can`t create window","Error",MB_OK|MB_ICONERROR);//������� ���������
		return 0;                  //��������� ������ ����������
	}
	g_hGlWnd = CreateChildWindow(g_hWnd, WS_BORDER, 300, 0, g_iGlWidth, g_iGlHeight, 0, ChildProc);
	ShowWindow(g_hGlWnd,SW_NORMAL);
	UpdateWindow(g_hGlWnd);

	WinTree *tree = WinTree::Create(g_hWnd, TVS_HASBUTTONS | TVS_LINESATROOT, 0, 0, 300, 560, ID_FIRSTCHILD);
	g_pTree = tree;

	HFONT hMainFont = UI_CreateFont("Tahoma", 13);
	HWND hGroup3D = UI_CreateGroupBox(g_hWnd, "3D Params", 320, 460, 230, 70, 0, 0);
	GroupProcOrigin = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hGroup3D, GWLP_WNDPROC));
	SetWindowLongPtr(hGroup3D, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GroupProc));

	g_hPos = UI_CreateLabel(hGroup3D, "PosX: 0.0\nPosY: 0.0\nPosZ: 0.0", 10, 15, 100, 40, 0, 3010);
	g_hCamPos = UI_CreateLabel(hGroup3D, "CamPosX: 0.0\nCamPosY: 0.0\nCamPosZ: 0.0", 100, 15, 120, 40, 0, 3011);
	
	UI_SetElementFont(hGroup3D, hMainFont);
	UI_SetElementFont(g_hPos, hMainFont);
	UI_SetElementFont(g_hCamPos, hMainFont);

	HWND hGroupTextures = UI_CreateGroupBox(g_hWnd, "Textures", 580, 460, 230, 70, 0, 0);
	SetWindowLongPtr(hGroupTextures, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(GroupProc));
	
	HWND hWndComboBox = UI_CreateComboBox(hGroupTextures, 10, 15, 100, 15, 0, 0);
	UI_SetElementFont(hGroupTextures, hMainFont);
	UI_SetElementFont(hWndComboBox, hMainFont);
	//ComboBox_AddString(hWndComboBox, "RGB565");
	//ComboBox_AddString(hWndComboBox, "RGBA4444");
	ComboBox_SetCurSel(hWndComboBox, 0);
	HWND hWndExportButton = UI_CreateButton(hGroupTextures, "Export Selected", 10, 40, 100, 20, 0, ID_BUTTON_TEX_EXPORT);
	UI_SetElementFont(hWndExportButton, hMainFont);
	
	//item->AddNext("fuck you", 0);

	if(!InitOpenGL(g_hGlWnd))    //���� �� ������ ���������������� OpenGL
	{
		Shutdown();
		MessageBox(NULL,"Can`t create OpenGL!","Error",MB_OK|MB_ICONERROR);//������� ���������
		return 0;                  //��������� ������ ����������
	}
	LoadFolderB3D();
	LoadFolderRES();

	/*ResTexture *tex = new ResTexture();
	FILE *f = fopen("menublk.fon", "rb");
	tex->Load(f, "matrix.fon");
	fclose(f);
	currentTex = tex;*/
	
	ShowWindow(g_hWnd,SW_SHOW);            //���������� ����  
	UpdateWindow(g_hWnd);              //��������� ����
	SetFocus(g_hWnd);                //������������� ����� �� ���� ����
	SetForegroundWindow(g_hWnd);          //������������� ��������� ���� ���� ��������

	MSG msg;
	ZeroMemory(&msg,sizeof(msg));

	while(g_bApplicationState)            //�������� ����������� ���� ��������� ���������
	{
		if(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))//�������� ���������
		{
			TranslateMessage(&msg);          //������������ ���������
			DispatchMessage(&msg);          //������������ ���������
		}
		else
			DrawFrame();              //���� ��������� ���� ������ �����
	}

	Shutdown();                    //����������� ������
	return 0;                    //��������� ������ ����������
}

extern char tmp[512];

long WINAPI GroupProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
	case WM_CTLCOLORSTATIC:
        {
			HDC hdcStatic = (HDC) wParam;
			SetTextColor(hdcStatic, RGB(0,0,0));
			SetBkMode(hdcStatic, TRANSPARENT);
			//SetBkColor(hdcStatic, RGB(255, 255, 255));
			/*if (hbrBkgnd == NULL)
			{
				hbrBkgnd = CreateSolidBrush(RGB(200,200,200));
			}*/
			return (INT_PTR)(HBRUSH)GetStockObject(LTGRAY_BRUSH);
        }

	case WM_CTLCOLORBTN:
		{
			//HDC hdcStatic = (HDC) wParam;
			//SetBkMode(hdcStatic, TRANSPARENT);
			return (INT_PTR)(HBRUSH)GetStockObject(LTGRAY_BRUSH);
		}

	case WM_CTLCOLOREDIT:
		{
			return (INT_PTR)(HBRUSH)GetStockObject(LTGRAY_BRUSH);
		}

	case WM_COMMAND:
		{
			switch(wParam)
			{
			case ID_BUTTON_TEX_EXPORT:
				{
					static TCHAR savePath[512];
					Node<WinTreeItem> *selection = new Node<WinTreeItem>(0, 0);
					WinTreeItem *item = g_pTree->m_pItems;
					int count = 0;
					while(item)
					{
						DWORD type;
						item->GetData(&type);
						if(type == 1)
						{
							count += item->GetSelection(selection);
						}
						else if (type == 2)
						{
							if (TreeView_GetCheckState(item->m_pTree->m_hWnd, item->m_hItem))
							{
								_tcscpy(savePath, ((B3D *)item->GetData())->m_sName);
								_tcscat(savePath, ".obj");
								TCHAR ext[32];
								_tcscpy(ext, TEXT("obj"));
								if (PathManager::SaveFile(0, savePath, TEXT("OBJ model file (*.obj)\0*.obj\0All\0*.*\0"), 0, ext))
									((B3D *)item->GetData())->SaveToObj(savePath);
							}
						}
						item = item->GetNext();
					}
					if(count > 0)
					{
						if(count == 1)
						{
							_tcscpy(savePath, ((ResTexture *)selection->data->GetData())->GetName());
							_tcscat(savePath, ".tga");
							TCHAR ext[32];
							_tcscpy(ext, TEXT("tga"));
							if(PathManager::SaveFile(0, savePath, TEXT("Targa Image (*.tga)\0*.tga\0All\0*.*\0"), 0, ext))
								((ResTexture *)selection->data->GetData())->SaveToFile(savePath);

						}
						else if(count > 1)
						{
							if(PathManager::SelectFolder(g_hWnd, savePath))
							{
								while(selection)
								{
									if(selection->data)
									{
										ResTexture *tex = (ResTexture*)selection->data->GetData();
										sprintf(tmp, "%s\\%s.tga", savePath, tex->GetName());
										tex->SaveToFile(tmp);
									}
									selection = selection->next;
								}
							}
						}
					}
					break;
				}
			}
			break;
		}
	}
	return CallWindowProc(GroupProcOrigin, hWnd, iMsg, wParam, lParam);
}

long WINAPI WndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
  switch(iMsg)
  {
	  case WM_CTLCOLORSTATIC:
        {
			HDC hdcStatic = (HDC) wParam;
			SetTextColor(hdcStatic, RGB(0,0,0));
			SetBkMode(hdcStatic, TRANSPARENT);
			//SetBkColor(hdcStatic, RGB(255, 255, 255));
			/*if (hbrBkgnd == NULL)
			{
				hbrBkgnd = CreateSolidBrush(RGB(200,200,200));
			}*/
			return (INT_PTR)(HBRUSH)GetStockObject(LTGRAY_BRUSH);
        }

  case WM_CREATE:
	  {
		  return 1;
	  }

  case WM_NOTIFY:
	  {
		  if(g_pTree)
			g_pTree->ProcessEvent(hWnd, iMsg, wParam, lParam);
		  break;
	  }
    case WM_DESTROY:              //���� �������� ��������� � ���������� ����
    {
		g_bApplicationState = false;      //������������� ��������� ���������� � false (��� ������ ��� ���� ��������� ��������� ������������)
		return 0;					    //������� ������� ��� �� ��� ��������� ����������
    }
  }

  return DefWindowProc(hWnd,iMsg,wParam,lParam);  //���� ���� ��� ��� ������ ���������, ����� ��� ������������ �������
}

void Shutdown()
{
	ReleaseOpenGL();


	if(!DestroyWindow(g_hWnd))            //���� �� ���������� ��������� ����
		g_hWnd = NULL;                //������������� ���������� ���� � ����

	if(!UnregisterClass("KingResourceManager",g_hInstance))  //���� �� ���������� ������� ���� ����������������� ����
		g_hInstance = NULL;              //������������� ���������� ���������� � ����

	if(g_pTree)
		g_pTree->Destroy();
}

void SetCurrentTextureData(ResTexture *tex)
{

}

void CALLBACK OnWinTreeTextureEvent(WINTREE *tree, WINTREEITEM *item, DWORD eventId, DWORD flags)
{
	if(!item)
		return;

	if(item->GetLevel() < 1)
		return;

	if(item->GetData() == 0)
		return;

	if(eventId == NM_DBLCLK)
	{
		if(currentTex)
			currentTex->glRelease();
		//currentGlModel = 0;

		currentTex = (ResTexture *)item->GetData();

		SetCurrentTextureData(currentTex);
	}
}