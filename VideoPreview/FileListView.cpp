#include "stdafx.h"
#pragma hdrstop
#include "Resource.h"
#include "Options.h"
#include "ProcessingItem.h"

// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "VideoPreview.h"
#endif

#include "VideoPreviewDoc.h"
#include "FileListView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CProcessingItemList ProcessingItemList;

static CString GetItemStateText(int item_state)
{
    switch(item_state)
    {
    case PIS_READY: return _T("Ready to process");                
    case PIS_DONE: return _T("Done");                 
    case PIS_FAILED: return _T("Failed");                
    }

    if(PIS_MIN_PROCESSING <= item_state && item_state <= PIS_MAX_PROCESSING)
    {
        CString result;
        result.Format(_T("%u%%"), item_state);
        return result;
    }

    return _T("Unknown");
}

//sorting callbacks
static int CALLBACK FileListViewSortBySource(LPARAM param1, LPARAM param2, LPARAM ascending)
{
    ASSERT(param1 && param2);
    if(NULL == param1 || NULL == param2) return 0;

    const CProcessingItem* left = reinterpret_cast<const CProcessingItem*>(param1);
    const CProcessingItem* right = reinterpret_cast<const CProcessingItem*>(param2);
    const int result = ::_tcsicmp(left->SourceFileName, right->SourceFileName);
    return ascending ? result : -result;
}
static int CALLBACK FileListViewSortByState(LPARAM param1, LPARAM param2, LPARAM ascending)
{
    ASSERT(param1 && param2);
    if(NULL == param1 || NULL == param2) return 0;

    const CProcessingItem* left = reinterpret_cast<const CProcessingItem*>(param1);
    const CProcessingItem* right = reinterpret_cast<const CProcessingItem*>(param2);
    const int result = static_cast<int>(left->State) - static_cast<int>(right->State);
    return ascending ? result : -result;
}

// CFileListView
IMPLEMENT_DYNCREATE(CFileListView, CListView)

BEGIN_MESSAGE_MAP(CFileListView, CListView)
    ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CFileListView construction/destruction
CFileListView::CFileListView()
{
	// TODO: add construction code here

}
CFileListView::~CFileListView()
{
}
BOOL CFileListView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}
void CFileListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
    CListCtrl& listCtrl = GetListCtrl();

    //TODO: style LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS ?
    listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    listCtrl.SetView(LV_VIEW_DETAILS);

    int i = 0;
    i = listCtrl.InsertColumn(FLV_COLUMN_SOURCE_FILE, _T("Source File"), LVCFMT_LEFT, 100);
    i = listCtrl.InsertColumn(FLV_COLUMN_STATE, _T("State"), LVCFMT_LEFT, 100);
    i = listCtrl.InsertColumn(FLV_COLUMN_DESCRIPTION, _T("Description"), LVCFMT_LEFT, 100);

    //TODO: last column autowidth
    CRect rect;
    listCtrl.GetClientRect(&rect);
    const int client_width = rect.Width();

    //column widths
    int column_width1 = Options.ColumnWidth1;
    int column_width2 = Options.ColumnWidth2;
    int column_width3 = Options.ColumnWidth3;
    if(column_width1 <= 0) column_width1 = 10;
    if(column_width2 <= 0) column_width2 = 10;
    if(column_width3 <= 0) column_width3 = 10;
    listCtrl.SetColumnWidth(0, column_width1);
    listCtrl.SetColumnWidth(1, column_width2);
    listCtrl.SetColumnWidth(2, column_width3);

    CHeaderCtrl* headerCtrl = listCtrl.GetHeaderCtrl();
    headerCtrl->ShowWindow(SW_SHOW);

    UpdateItems();
}
void CFileListView::OnDestroy()
{
    CListCtrl& listCtrl = GetListCtrl();
    Options.ColumnWidth1 = listCtrl.GetColumnWidth(0);
    Options.ColumnWidth2 = listCtrl.GetColumnWidth(1);
    Options.ColumnWidth3 = listCtrl.GetColumnWidth(2);   

    CListView::OnDestroy();
}
void CFileListView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}
void CFileListView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}
BOOL CFileListView::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    //TODO:
    //message is WM_NOTIFY;
    //control id?
    if(message != WM_NOTIFY) return FALSE;

    LPNMHDR nmhdr = reinterpret_cast<LPNMHDR>(lParam);
    if(NM_RCLICK == nmhdr->code)
    {
        LPNMITEMACTIVATE event_data = reinterpret_cast<LPNMITEMACTIVATE>(nmhdr);

        //TODO:
        //ShowContextMenu(event_data->ptAction);
    }
    else if(NM_DBLCLK == nmhdr->code)
    {
        //TODO:
        
    }
    else if(LVN_COLUMNCLICK == nmhdr->code) 
    {
        LPNMLISTVIEW nmlv = reinterpret_cast<LPNMLISTVIEW>(lParam);
        OnColumnClick(nmlv->iSubItem);
    }
    else 
        return FALSE;

    return TRUE;
}
void CFileListView::Sort()
{
    CListCtrl& list_ctrl = GetListCtrl();
    if(FLV_COLUMN_SOURCE_FILE == Options.SortedColumn)
        list_ctrl.SortItems(FileListViewSortBySource, Options.SortOrder);
    else if(FLV_COLUMN_STATE == Options.SortedColumn)
        list_ctrl.SortItems(FileListViewSortByState, Options.SortOrder);
}
void CFileListView::OnColumnClick(int column_index)
{
    if(column_index != FLV_COLUMN_SOURCE_FILE && column_index != FLV_COLUMN_STATE)
        return;
    if(column_index == Options.SortedColumn)
        Options.SortOrder = !Options.SortOrder;
    else
        Options.SortedColumn = column_index;
    Sort();
}
void CFileListView::UpdateItems()
{
    CListCtrl& list_ctrl = GetListCtrl();
    list_ctrl.DeleteAllItems();
    for(CProcessingItemList::const_iterator i = ProcessingItemList.begin(); i != ProcessingItemList.end(); ++i)
        AddItem(i->second);
}
void CFileListView::UpdateItem(PProcessingItem item)
{
    ASSERT(item.get());
    const LPARAM data = reinterpret_cast<LPARAM>(item.get());
    if(NULL == data) 
        return;

    LVFINDINFO lvfi;
    ::ZeroMemory(&lvfi, sizeof(lvfi));
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = data;

    CListCtrl& list_ctrl = GetListCtrl();
    int item_index = list_ctrl.FindItem(&lvfi);
    if(item_index < 0) return;

    //TODO:
    LPCTSTR str = GetItemStateText(item->State);
    list_ctrl.SetItemText(item_index, FLV_COLUMN_STATE, GetItemStateText(item->State)); 
}
void CFileListView::AddItem(PProcessingItem item)
{
    ASSERT(item.get());
    if(NULL == item.get()) return;
    
    CListCtrl& list_ctrl = GetListCtrl();

    LVITEM lvi;
    ::ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.pszText = const_cast<LPTSTR>((LPCTSTR)item->SourceFileName);
    lvi.lParam = reinterpret_cast<LPARAM>(item.get());
    
    const int item_index = list_ctrl.InsertItem(&lvi);
    list_ctrl.SetItemText(item_index, FLV_COLUMN_STATE, GetItemStateText(item->State)); 
}
void CFileListView::RemoveItem(PProcessingItem item)
{
    ASSERT(item.get());
    const LPARAM data = reinterpret_cast<LPARAM>(item.get());
    if(NULL == data) 
        return;

    LVFINDINFO lvfi;
    ::ZeroMemory(&lvfi, sizeof(lvfi));
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = data;

    CListCtrl& list_ctrl = GetListCtrl();
    const int item_index = list_ctrl.FindItem(&lvfi);
    if(item_index < 0) return;
    list_ctrl.DeleteItem(item_index);
}
void CFileListView::RemoveCompletedItems(PProcessingItem item)
{
    CListCtrl& list_ctrl = GetListCtrl();
    for(int index = -1; ; ++index)
    {
        const int found = list_ctrl.GetNextItem(index, LVNI_ALL); 
        if(found < 0) 
            break;

        LVITEM lvi;
        ::ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = found;
        lvi.mask = LVIF_PARAM;
        if(0 == list_ctrl.GetItem(&lvi)) 
            continue;

        CProcessingItemList::iterator i = ProcessingItemList.find(reinterpret_cast<CProcessingItem*>(lvi.lParam));
        PProcessingItem pi = i->second;
        if(PIS_READY == pi->State)
            list_ctrl.DeleteItem(found);
    }
}
PProcessingItem CFileListView::GetUnprocessedItem()
{
    CListCtrl& list_ctrl = GetListCtrl();
    for(int index = -1; ; ++index)
    {
        const int found = list_ctrl.GetNextItem(index, LVNI_ALL); 
        if(found < 0) 
            break;

        LVITEM lvi;
        ::ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = found;
        lvi.mask = LVIF_PARAM;
        if(0 == list_ctrl.GetItem(&lvi)) 
            continue;

        CProcessingItemList::iterator i = ProcessingItemList.find(reinterpret_cast<CProcessingItem*>(lvi.lParam));
        PProcessingItem pi = i->second;
        if(PIS_READY == pi->State)
            return pi;
    }
    return NULL;
}
// CFileListView diagnostics
#ifdef _DEBUG
void CFileListView::AssertValid() const
{
	CListView::AssertValid();
}

void CFileListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CVideoPreviewDoc* CFileListView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVideoPreviewDoc)));
	return (CVideoPreviewDoc*)m_pDocument;
}
#endif //_DEBUG

// CFileListView message handlers
