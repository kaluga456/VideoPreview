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
    case PIS_READY: return _T("Waiting");                
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
END_MESSAGE_MAP()

// CFileListView construction/destruction
CFileListView::CFileListView()
{
}
CFileListView::~CFileListView()
{
}
BOOL CFileListView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	return CListView::PreCreateWindow(cs);
}
void CFileListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

    PopupMenu.LoadMenu(IDR_POPUP_MENU);

    //set view styles
    CListCtrl& listCtrl = GetListCtrl();
    const LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    ::SetWindowLong(m_hWnd, GWL_STYLE, style | LVS_SHOWSELALWAYS);
    listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    listCtrl.SetView(LV_VIEW_DETAILS);

    //init columns
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
void CFileListView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
    CPoint client_pt = point;
    ScreenToClient(&client_pt);

    //disable context menu on header control
    CHeaderCtrl* headerCtrl = GetListCtrl().GetHeaderCtrl();
    _HD_HITTESTINFO hti;
    ::ZeroMemory(&hti, sizeof(hti));
    hti.pt = client_pt;
    headerCtrl->HitTest(&hti);
    if(hti.flags & HHT_ONHEADER) //header control click
        return;

    CMenu* pm = PopupMenu.GetSubMenu(0);
    const BOOL result = pm->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON /*| TPM_RETURNCMD*/, point.x, point.y, GetParent());
#endif
}
CProcessingItem* CFileListView::FindItem(const CPoint& point)
{
    LVHITTESTINFO lvhti;
    ::ZeroMemory(&lvhti, sizeof(lvhti));
    lvhti.pt = point;
    GetListCtrl().HitTest(&lvhti);
    if(lvhti.iItem < 0) return NULL;
    return FindItem(lvhti.iItem);
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
        AddItem(i->second.get());

    Sort();
}
void CFileListView::UpdateItem(const CProcessingItem* item)
{
    const int item_index = FindItem(item);
    item_index < 0 ? AddItem(item) : UpdateItem(item, item_index);
}
void CFileListView::UpdateItem(const CProcessingItem* item, int item_index)
{
    ASSERT(item_index >= 0);
    CListCtrl& list_ctrl = GetListCtrl();
    list_ctrl.SetItemText(item_index, FLV_COLUMN_STATE, GetItemStateText(item->State)); 
    list_ctrl.SetItemText(item_index, FLV_COLUMN_DESCRIPTION, item->ResultString);
}
void CFileListView::AddItem(const CProcessingItem* item)
{
    ASSERT(item);
    if(NULL == item) return;
    
    CListCtrl& list_ctrl = GetListCtrl();

    LVITEM lvi;
    ::ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.pszText = const_cast<LPTSTR>((LPCTSTR)item->SourceFileName);
    lvi.lParam = reinterpret_cast<LPARAM>(item);
    
    const int item_index = list_ctrl.InsertItem(&lvi);
    UpdateItem(item, item_index);
}
void CFileListView::RemoveItem(const CProcessingItem* item)
{
    if(item && item->IsActive()) return;
    const int item_index = FindItem(item);
    if(item_index < 0) return;
    GetListCtrl().DeleteItem(item_index);
}
PProcessingItem CFileListView::GetUnprocessedItem()
{
    CListCtrl& list_ctrl = GetListCtrl();
    for(int index = -1;; ++index)
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

int CFileListView::FindItem(const CProcessingItem* item)
{
    ASSERT(item);
    const LPARAM data = reinterpret_cast<LPARAM>(item);
    if(NULL == data) 
        return -1;

    LVFINDINFO lvfi;
    ::ZeroMemory(&lvfi, sizeof(lvfi));
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = data;
    return GetListCtrl().FindItem(&lvfi);
}
CProcessingItem* CFileListView::FindItem(int item_index)
{
    return reinterpret_cast<CProcessingItem*>(GetListCtrl().GetItemData(item_index));
}
CProcessingItem* CFileListView::GetFocusedItem(int* index /*= NULL*/)
{
    const int found = GetListCtrl().GetNextItem(-1, LVNI_FOCUSED); 
    if(found < 0) return NULL;

    if(index) *index = found;
    return FindItem(found);
}
