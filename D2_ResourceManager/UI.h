#ifndef UI_CLASS
#define UI_CLASS

#include <Windows.h>
#include <CommCtrl.h>
#include "CNode.h"

HWND CreateTreeView(HWND hwndParent, int style, int x, int y, int sizeX, int sizeY, int id);
HWND CreateChildWindow(HWND hwndParent, DWORD style, int x, int y, int sizeX, int sizeY, int id, WNDPROC proc);
HTREEITEM AddItemToTree(HWND hwndTV, LPTSTR lpszItem, int nLevel);

typedef class WinTree WINTREE;
typedef class WinTreeItem WINTREEITEM;
typedef void (CALLBACK *LPWINTREECALLBACK)(WINTREE *, WINTREEITEM *, DWORD, DWORD);
typedef void (CALLBACK *LPWINTREEFORALLCALLBACK)(WINTREEITEM *);

class WinTreeItem
{
private:
	WinTreeItem *m_pParent;
	WinTreeItem *m_pNext;
	WinTreeItem *m_pChild;
	int			m_dLevel;
	LPTSTR		m_sName;
	DWORD		m_dData;
	DWORD		m_dDataType;
	LPWINTREECALLBACK m_pCallback;

	WinTreeItem();
	~WinTreeItem();

public:
	WinTree		*m_pTree;
	HTREEITEM	m_hItem;
	static WinTreeItem *Create(WinTree *winTree, LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback = 0);
	void Destroy();
	WinTreeItem *GetRoot();
	WinTreeItem *AddChild(LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback = 0);
	WinTreeItem *AddNext(LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback = 0);
	WinTreeItem *AddToLast(LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback = 0);
	void SetCallback(LPWINTREECALLBACK callback);
	void ForAll(LPWINTREEFORALLCALLBACK callback);
	void ForAllChildren(LPWINTREEFORALLCALLBACK callback);

	WinTreeItem *GetChild();
	WinTreeItem *GetParent();
	WinTreeItem *GetNext();
	WinTreeItem *GetByHandle(HTREEITEM item);
	int GetSelection(Node<WinTreeItem> *node);
	DWORD GetData(DWORD *type = 0);
	void SetData(DWORD data, DWORD type = 0);
	int GetLevel();

	friend WINTREE;

};

class WinTree
{
private:
	HWND	m_hWndParent;
	int		m_dId;
	WinTreeItem *m_pSelected;

	WinTree();
	~WinTree();

public:
	HWND	m_hWnd;
	WinTreeItem *m_pItems;

	static WinTree *Create(HWND hwndParent, int style, int x, int y, int sizeX, int sizeY, int id);
	void Destroy();
	WinTreeItem *AddItem(LPTSTR lpszItem, DWORD data, DWORD dataType = 0);
	WinTreeItem *GetSelected();
	bool ProcessEvent(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	friend WINTREEITEM;
};



#endif