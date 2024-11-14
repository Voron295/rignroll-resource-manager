//#include <ShellAPI.h>
#include "UI.h"
//#include <Windows.h>

typedef struct 
{ 
	TCHAR tchHeading[256]; 
    int tchLevel; 
} Heading; 

extern HINSTANCE g_hInstance;

Heading g_rgDocHeadings[100];

HWND CreateTreeView(HWND hwndParent, int style, int x, int y, int sizeX, int sizeY, int id)
{
    HWND hwndTV;    // handle to tree-view control 

    // Ensure that the common control DLL is loaded. 
    InitCommonControls(); 

    hwndTV = CreateWindowEx(0,
                            WC_TREEVIEW,
                            TEXT("Tree View"),
                            WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | style, 
                            x, 
                            y, 
                            sizeX, 
                            sizeY,
                            hwndParent, 
                            (HMENU)id, 
                            g_hInstance, 
                            NULL); 

    return hwndTV;
}

HWND CreateChildWindow(HWND hwndParent, DWORD style, int x, int y, int sizeX, int sizeY, int id, WNDPROC proc)
{
	WNDCLASS w;
	memset(&w,0,sizeof(WNDCLASS));
	w.lpfnWndProc = proc;
	w.hInstance = g_hInstance;
	w.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	w.lpszClassName = "ChildWClass";
	w.hCursor=LoadCursor(NULL,IDC_ARROW); 
	RegisterClass(&w);
	HWND child;
	child=CreateWindowEx(0,"ChildWClass",(LPCTSTR) NULL,
		WS_CHILD | WS_VISIBLE | style,x, y,
		sizeX,sizeY,hwndParent,(HMENU)id,g_hInstance,NULL);
	
	return child;
}

// -------------- WinTree Class --------------------

WinTreeItem::WinTreeItem()
{

}

WinTreeItem::~WinTreeItem()
{
	if(m_sName)
		delete[] m_sName;

	if(m_pChild)
		m_pChild->~WinTreeItem();

	if(m_pNext)
		m_pNext->~WinTreeItem();
}

WinTree::WinTree()
{
	memset(this, 0, sizeof(WinTree));
}

WinTree::~WinTree()
{

}

WinTreeItem *WinTreeItem::GetRoot()
{
	WinTreeItem *parent = this;
	while(parent)
	{
		if(!parent->m_pParent)
			break;
		
		parent = parent->m_pParent;
	}
	return parent;
}

WinTreeItem *WinTreeItem::GetNext()
{
	return m_pNext;
}

int WinTreeItem::GetSelection(Node<WinTreeItem> *node)
{
	int count = 0;
	WinTreeItem *child = m_pChild;
	while(child)
	{
		node = node->GetLast();
		if(TreeView_GetCheckState(m_pTree->m_hWnd, child->m_hItem))
		{
			node->data = child;
			node->next = new Node<WinTreeItem>(0, 0);
			node = node->next;
			count++;
		}
		count += child->GetSelection(node);
		child = child->m_pNext;
	}
	return count;
}

WinTreeItem *WinTreeItem::Create(WinTree *winTree, LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback)
{
	WinTreeItem *item = new WinTreeItem();
	if(!item)
		return 0;

	item->m_pParent = 0;
	item->m_pChild = 0;
	item->m_pNext = 0;
	item->m_dData = data;
	item->m_dLevel = 0;
	item->m_pTree = winTree;
	item->m_pCallback = callback;

	TVITEM tvi; 
    TVINSERTSTRUCT tvins;

	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	tvi.stateMask = TVIS_STATEIMAGEMASK;

    // Set the text of the item. 
    tvi.pszText = lpszItem; 
    tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 

    // Assume the item is not a parent item, so give it a 
    // document image. 
    tvi.iImage = 0; 
    tvi.iSelectedImage = 0; 

    // Save the heading level in the item's application-defined 
    // data area. 
    tvi.lParam = (LPARAM)data; 
	tvi.state = 0;
    tvins.item = tvi; 
    tvins.hInsertAfter = TVI_FIRST; 

    // Set the parent item based on the specified level. 
	tvins.hParent = TVI_ROOT;
    // Add the item to the tree-view control. 
    item->m_hItem = (HTREEITEM)SendMessage(winTree->m_hWnd, TVM_INSERTITEM, 
        0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

    if (item->m_hItem == NULL)
	{
		item->~WinTreeItem();
        return NULL;
	}

	int len = strlen(lpszItem) + 1;
	item->m_sName = new char[len];
	strcpy_s(item->m_sName, len, lpszItem);

	return item;
}

WinTreeItem *WinTreeItem::AddChild(LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback)
{
	if(m_pChild)
	{
		WinTreeItem *next = m_pChild;
		while(next->m_pNext)
			next = next->m_pNext;
		return next->AddNext(lpszItem, data, callback);
	}

	WinTreeItem *item = new WinTreeItem();
	if(!item)
		return 0;

	item->m_pParent = this;
	item->m_pChild = 0;
	item->m_pNext = 0;
	item->m_dData = data;
	item->m_dLevel = m_dLevel + 1;
	item->m_pTree = m_pTree;
	item->m_pCallback = callback;

	TVITEM tvi; 
    TVINSERTSTRUCT tvins;

	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE; 
	tvi.stateMask = TVIS_STATEIMAGEMASK;

    // Set the text of the item. 
    tvi.pszText = lpszItem; 
    tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 

    // Assume the item is not a parent item, so give it a 
    // document image. 
    tvi.iImage = 0; 
    tvi.iSelectedImage = 0; 

    // Save the heading level in the item's application-defined 
    // data area. 
    tvi.lParam = (LPARAM)data; 
	tvi.state = 0;
    tvins.item = tvi; 
    tvins.hInsertAfter = TVI_FIRST; 

    // Set the parent item based on the specified level. 
	tvins.hParent = m_hItem;
    // Add the item to the tree-view control. 
    item->m_hItem = (HTREEITEM)SendMessage(m_pTree->m_hWnd, TVM_INSERTITEM, 
        0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

    if (item->m_hItem == NULL)
	{
		item->~WinTreeItem();
        return NULL;
	}

	int len = strlen(lpszItem) + 1;
	item->m_sName = new char[len];
	strcpy_s(item->m_sName, len, lpszItem);

	m_pChild = item;

	return item;
}

WinTreeItem *WinTreeItem::AddNext(LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback)
{
	WinTreeItem *item = new WinTreeItem();
	if(!item)
		return 0;

	item->m_pParent = m_pParent;
	item->m_pChild = 0;
	item->m_pNext = 0;
	item->m_dData = data;
	item->m_dLevel = m_dLevel;
	item->m_pTree = m_pTree;
	item->m_pCallback = callback;

	TVITEM tvi; 
    TVINSERTSTRUCT tvins;

	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE; 
	tvi.stateMask = TVIS_STATEIMAGEMASK;

    // Set the text of the item. 
    tvi.pszText = lpszItem; 
    tvi.cchTextMax = sizeof(tvi.pszText)/sizeof(tvi.pszText[0]); 

    // Assume the item is not a parent item, so give it a 
    // document image. 
    tvi.iImage = 0; 
    tvi.iSelectedImage = 0; 

    // Save the heading level in the item's application-defined 
    // data area. 
    tvi.lParam = (LPARAM)data; 
    tvins.item = tvi; 
    tvins.hInsertAfter = m_hItem; 
	tvi.state = 0;

    // Set the parent item based on the specified level. 
	if(m_pParent)
		tvins.hParent = m_pParent->m_hItem;
	else
		tvins.hParent = TVI_ROOT;
    // Add the item to the tree-view control. 
    item->m_hItem = (HTREEITEM)SendMessage(m_pTree->m_hWnd, TVM_INSERTITEM, 
        0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

    if (item->m_hItem == NULL)
	{
		item->~WinTreeItem();
        return NULL;
	}

	int len = strlen(lpszItem) + 1;
	item->m_sName = new char[len];
	strcpy_s(item->m_sName, len, lpszItem);

	m_pNext = item;

	return item;
}

WinTreeItem *WinTreeItem::AddToLast(LPTSTR lpszItem, DWORD data, LPWINTREECALLBACK callback)
{
	if(m_pNext)
	{
		WinTreeItem *next = m_pNext;
		while(next->m_pNext)
			next = next->m_pNext;
		return next->AddNext(lpszItem, data, callback);
	}
	return AddNext(lpszItem, data, callback);
}

void WinTreeItem::Destroy()
{
	this->~WinTreeItem();
}

WinTree *WinTree::Create(HWND hwndParent, int style, int x, int y, int sizeX, int sizeY, int id)
{
	WinTree *tree = new WinTree();
	if(!tree)
		return 0;
    // Ensure that the common control DLL is loaded. 
    InitCommonControls(); 

	tree->m_hWndParent = hwndParent;
	tree->m_dId = id;
	tree->m_pItems = 0;
    tree->m_hWnd = CreateWindowEx(0,
                            WC_TREEVIEW,
                            TEXT("Tree View"),
                            WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_CHECKBOXES | style, 
                            x, 
                            y, 
                            sizeX, 
                            sizeY,
                            hwndParent, 
                            (HMENU)id, 
                            g_hInstance, 
                            NULL); 
	if(!tree->m_hWnd)
	{
		tree->~WinTree();
		return 0;
	}

	ShowWindow(tree->m_hWnd,SW_NORMAL);
	UpdateWindow(tree->m_hWnd);

	return tree;
}

WinTreeItem *WinTree::AddItem(LPTSTR lpszItem, DWORD data, DWORD dataType)
{
	WinTreeItem *add;
	if(!m_pItems)
	{
		add = WinTreeItem::Create(this, lpszItem, data);
		m_pItems = add;
	}
	else
	{
		add = m_pItems->AddToLast(lpszItem, data);
	}
	add->m_dDataType = dataType;
	return add;
}

void WinTree::Destroy()
{
	if(m_pItems)
		m_pItems->Destroy();

	DestroyWindow(m_hWnd);
}

void CALLBACK CheckWinTreeItem(WinTreeItem *item)
{
	TreeView_SetCheckState(item->m_pTree->m_hWnd, item->m_hItem, 1);
}

void CALLBACK UnCheckWinTreeItem(WinTreeItem *item)
{
	TreeView_SetCheckState(item->m_pTree->m_hWnd, item->m_hItem, 0);
}

bool WinTree::ProcessEvent(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//if(m_hWnd != hWnd)
		//return 0;

	LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
	if(pNMHDR->hwndFrom == m_hWnd && (pNMHDR->code == NM_DBLCLK || pNMHDR->code == NM_CLICK || pNMHDR->code == NM_RCLICK || pNMHDR->code == TVN_SELCHANGED))
	{
		if(pNMHDR->code == TVN_SELCHANGED)
		{
			WinTreeItem *item = m_pItems->GetByHandle(((LPNMTREEVIEW)lParam)->itemNew.hItem);
			m_pSelected = item;

			if(item->m_pCallback)
				item->m_pCallback(this, item, pNMHDR->code, 0);
		}
		else
		{
			TVHITTESTINFO test;
			GetCursorPos(&test.pt);
			ScreenToClient(hWnd, &test.pt);
			TreeView_HitTest(pNMHDR->hwndFrom, &test);
			if(test.flags & TVHT_ONITEM || test.flags & TVHT_ONITEMSTATEICON)
			{
				WinTreeItem *item = m_pItems->GetByHandle(test.hItem);
				if(test.flags & TVHT_ONITEMSTATEICON)
				{
					if(!TreeView_GetCheckState(m_hWnd, item->m_hItem))
					{
						item->ForAllChildren(CheckWinTreeItem);
					}
					else
					{
						item->ForAllChildren(UnCheckWinTreeItem);
					}
				}
				if(item->m_pCallback)
					item->m_pCallback(this, item, pNMHDR->code, test.flags);
			}
		}
	}
	return 1;
}

WinTreeItem *WinTreeItem::GetByHandle(HTREEITEM item)
{
	//if(m_hItem == item)
		//return this;

	WinTreeItem *result = 0;
	WinTreeItem *next = m_pNext;

	for(next = this; next; next = next->m_pNext)
	{
		if(next->m_hItem == item)
			return next;

		if(next->m_pChild)
		{
			result = next->m_pChild->GetByHandle(item);
			if(result)
				return result;
		}
	}
	return result;
}

DWORD WinTreeItem::GetData(DWORD *type)
{
	if(type)
		*type = m_dDataType;
	return m_dData;
}

int WinTreeItem::GetLevel()
{
	return m_dLevel;
}

void WinTreeItem::SetCallback(LPWINTREECALLBACK callback)
{
	m_pCallback = callback;
}

void WinTreeItem::SetData(DWORD data, DWORD type)
{
	m_dData = data;
	m_dDataType = type;
}

void WinTreeItem::ForAll(LPWINTREEFORALLCALLBACK callback)
{
	callback(this);
	if(m_pChild)
	{
		WinTreeItem *next = m_pChild;
		while(next)
		{
			next->ForAll(callback);
			next = next->m_pNext;
		}
	}
}

void WinTreeItem::ForAllChildren(LPWINTREEFORALLCALLBACK callback)
{
	if(m_pChild)
	{
		WinTreeItem *next = m_pChild;
		while(next)
		{
			next->ForAll(callback);
			next = next->m_pNext;
		}
	}
}