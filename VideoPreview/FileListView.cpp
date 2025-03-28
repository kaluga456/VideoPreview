#include "stdafx.h"
//#include "app_error.h"
#pragma hdrstop
#include "VPError.h"
#include "ClipboardFiles.h"
#include "VideoFileTypes.h"
#include "Resource.h"
#include "Settings.h"
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

extern bool IsProcessSelected; //true process only selected items

static CString GetItemStateText(const CProcessingItem* pi)
{
    INT_PTR item_state = pi->State;
    switch(item_state)
    {
    case PIS_WAIT: 
    {
        if(PTS_NONE == ProcessingState || (IsProcessSelected && false == pi->Selected))
            return _T("");
        return _T("Waiting...");
    }
    case PIS_DONE: return _T("Done");                 
    case PIS_FAILED: return _T("Failed");                
    }

    if(PIS_MIN_PROCESSING <= item_state && item_state <= PIS_MAX_PROCESSING)
    {
        CString result;
        result.Format(_T("%u%%"), static_cast<int>(item_state));
        return result;
    }
    return _T("Unknown");
}

//sorting callbacks
static int CALLBACK FileListViewSortBySource(LPARAM param1, LPARAM param2, LPARAM ascending)
{
    ASSERT(param1 && param2);
    if(0 == param1 || 0 == param2) return 0;

    const CProcessingItem* left = reinterpret_cast<const CProcessingItem*>(param1);
    const CProcessingItem* right = reinterpret_cast<const CProcessingItem*>(param2);
    const int result = ::_tcsicmp(left->SourceFileName, right->SourceFileName);
    return ascending ? result : -result;
}
static int CALLBACK FileListViewSortByState(LPARAM param1, LPARAM param2, LPARAM ascending)
{
    ASSERT(param1 && param2);
    if(0 == param1 || 0 == param2) return 0;

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
    //ON_WM_SIZE()
END_MESSAGE_MAP()

// CFileListView construction/destruction
CFileListView::CFileListView() : DropEffect(DROPEFFECT_NONE)
{
    Accel = ::LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(IDA_FILE_LIST));
}
CFileListView::~CFileListView()
{
    ::DestroyAcceleratorTable(Accel);
}
BOOL CFileListView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	return CListView::PreCreateWindow(cs);
}
void CFileListView::OnDestroy()
{
    CListCtrl& listCtrl = GetListCtrl();
    Settings.ColumnWidth1 = listCtrl.GetColumnWidth(0);
    Settings.ColumnWidth2 = listCtrl.GetColumnWidth(1);
    Settings.ColumnWidth3 = listCtrl.GetColumnWidth(2);   

    CListView::OnDestroy();
}
void CFileListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

    //TODO: From your view class's function that handles the WM_CREATE message
    OleDropTarget.Register(this);

    theApp.GetContextMenuManager()->AddMenu(_T("Context Menu"), IDR_POPUP_MENU);

    //set view styles
    CListCtrl& listCtrl = GetListCtrl();
    const LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
    ::SetWindowLong(m_hWnd, GWL_STYLE, style | LVS_SHOWSELALWAYS);
    listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    listCtrl.SetView(LV_VIEW_DETAILS);

    //init columns
    int i = 0;
    i = listCtrl.InsertColumn(FLV_COLUMN_VIDEO, _T("Video"), LVCFMT_LEFT, 100);
    i = listCtrl.InsertColumn(FLV_COLUMN_STATE, _T("State"), LVCFMT_LEFT, 100);
    i = listCtrl.InsertColumn(FLV_COLUMN_RESULT, _T("Result"), LVCFMT_LEFT, 100);

    //TODO: last column autowidth
    CRect rect;
    listCtrl.GetClientRect(&rect);
    const int client_width = rect.Width();

    //column widths
    const int column_width1 = Settings.ColumnWidth1 <= 0 ? 10 : Settings.ColumnWidth1;
    const int column_width2 = Settings.ColumnWidth2 <= 0 ? 10 : Settings.ColumnWidth2;
    const int column_width3 = client_width - column_width1 - column_width2;
    listCtrl.SetColumnWidth(FLV_COLUMN_VIDEO, column_width1);
    listCtrl.SetColumnWidth(FLV_COLUMN_STATE, column_width2);
    listCtrl.SetColumnWidth(FLV_COLUMN_RESULT, column_width3);

    CHeaderCtrl* headerCtrl = listCtrl.GetHeaderCtrl();
    headerCtrl->ShowWindow(SW_SHOW);

    UpdateItems();
}
void CFileListView::OnSize(UINT nType, int cx, int cy)
{
    CListView::OnSize(nType, cx, cy);

    //TEST:
 //   const int top_size = 24;
	//CRect cr;
	//GetClientRect(cr);
 //   SetWindowPos(nullptr, cr.left, cr.top + top_size, cr.Width(), cr.Height() - top_size, SWP_NOACTIVATE | SWP_NOZORDER);

    //TODO: last column autosize
    //CListCtrl& listCtrl = GetListCtrl();
    //const int column_width1 = listCtrl.GetColumnWidth(0);
    //const int column_width2 = listCtrl.GetColumnWidth(1);
    //const int column_width3 = listCtrl.GetColumnWidth(FLV_COLUMN_RESULT);
    //const int space = cx - column_width1 - column_width2;
    //listCtrl.SetColumnWidth(FLV_COLUMN_RESULT, space);
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

    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_MENU, point.x, point.y, this);
#endif
}
BOOL CFileListView::PreTranslateMessage(MSG* msg)
{
    if(FALSE == ::TranslateAccelerator(m_hWnd, Accel, msg))
        return FALSE;

    //Delete
    if(VK_DELETE == msg->wParam)
    {
        ::SendMessage(GetParent()->m_hWnd, WM_COMMAND, ID_CMD_REMOVE_SELECTED, 0);
        return TRUE;
    }

    //Ctrl+A
    if(0x41 == msg->wParam)
    {
        CListCtrl& lc = GetListCtrl();
        const int count = lc.GetItemCount();
        for(int i = 0; i < count; ++i)
            lc.SetItemState(i, LVNI_SELECTED, LVIS_SELECTED);
        return TRUE;
    }

    //Ctrl+V
    if(0x56 == msg->wParam)
    {
        CClipboardFiles cf(m_hWnd);
        if(FileList.AddFiles(&cf))
        {
            UpdateItems();
            UpdateDialogControls(this, FALSE);
        }   
        return TRUE;
    }
    return FALSE;
}
DROPEFFECT CFileListView::OnDragEnter(COleDataObject* ole_object, DWORD grfKeyState, CPoint point)
{
	ASSERT(DropEffect == DROPEFFECT_NONE);
	
    HGLOBAL hObjDesc = ole_object->GetGlobalData(CF_HDROP);
    if(nullptr == hObjDesc)
        DropEffect = DROPEFFECT_NONE;
    else
        DropEffect = DROPEFFECT_COPY;
	
	return DropEffect;
}
DROPEFFECT CFileListView::OnDragOver(COleDataObject* ole_object, DWORD grfKeyState, CPoint point)
{
    return DropEffect;
}
BOOL CFileListView::OnDrop(COleDataObject* ole_object, DROPEFFECT dropEffect, CPoint point)
{
    if(dropEffect != DROPEFFECT_COPY)
    {
        DropEffect = DROPEFFECT_NONE;
        return FALSE;
    }

    DropEffect = DROPEFFECT_NONE;

    COleObjectFiles oof(ole_object);
    if(FileList.AddFiles(&oof))
    {
        UpdateItems();
        UpdateDialogControls(GetParent(), FALSE);
    }
    return TRUE;
}
void CFileListView::OnDragLeave()
{
    DropEffect = DROPEFFECT_NONE;
}
CProcessingItem* CFileListView::FindItem(const CPoint& point)
{
    LVHITTESTINFO lvhti;
    ::ZeroMemory(&lvhti, sizeof(lvhti));
    lvhti.pt = point;
    GetListCtrl().HitTest(&lvhti);
    if(lvhti.iItem < 0) return nullptr;
    return FindItem(lvhti.iItem);
}
void CFileListView::Sort()
{
    CListCtrl& list_ctrl = GetListCtrl();
    if(FLV_COLUMN_VIDEO == Settings.SortedColumn)
        list_ctrl.SortItems(FileListViewSortBySource, Settings.SortOrder);
    else if(FLV_COLUMN_STATE == Settings.SortedColumn)
        list_ctrl.SortItems(FileListViewSortByState, Settings.SortOrder);
}
void CFileListView::OnColumnClick(int column_index)
{
    if(column_index != FLV_COLUMN_VIDEO && column_index != FLV_COLUMN_STATE)
        return;
    if(column_index == Settings.SortedColumn)
        Settings.SortOrder = !Settings.SortOrder;
    else
        Settings.SortedColumn = column_index;
    Sort();
}
void CFileListView::UpdateItems()
{
    CListCtrl& list_ctrl = GetListCtrl();
    list_ctrl.DeleteAllItems();
    for(auto& i : FileList.Items)
        AddItem(i.second.get());

    Sort();
}
void CFileListView::UpdateItemStates()
{
    CListCtrl& lc = GetListCtrl();
    const int count = lc.GetItemCount();
    for(int index = 0; index < count; ++index)
    {
        LVITEM li;
        ::ZeroMemory(&li, sizeof(li));
        li.iItem = index;
        li.mask = LVIF_PARAM;
        lc.GetItem(&li); 

        CProcessingItem* pi = reinterpret_cast<CProcessingItem*>(li.lParam);
        ASSERT(pi);
        if(nullptr == pi)
            continue;

        if(pi->State != PIS_WAIT)
            continue;
        lc.SetItemText(index, FLV_COLUMN_STATE, GetItemStateText(pi)); 
    }

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
    list_ctrl.SetItemText(item_index, FLV_COLUMN_STATE, GetItemStateText(item)); 
    list_ctrl.SetItemText(item_index, FLV_COLUMN_RESULT, item->ResultString);
}
void CFileListView::AddItem(const CProcessingItem* item)
{
    ASSERT(item);
    if(nullptr == item) return;
    
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
void CFileListView::SaveSelection()
{
    CListCtrl& lc = GetListCtrl();
    const int count = lc.GetItemCount();
    for(int index = 0; index < count; ++index)
    {
        LVITEM li;
        ::ZeroMemory(&li, sizeof(li));
        li.iItem = index;
        li.mask = LVIF_PARAM | LVIF_STATE;
        li.stateMask = LVIS_SELECTED;

        lc.GetItem(&li); 

        CProcessingItem* pi = reinterpret_cast<CProcessingItem*>(li.lParam);
        ASSERT(pi);
        if(nullptr == pi)
            continue;

        pi->Selected = (li.state & LVIS_SELECTED) ? TRUE : FALSE;
    }
}
PProcessingItem CFileListView::GetUnprocessedItem(bool selected_only)
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

        CProcessingItemList::iterator i = FileList.Items.find(reinterpret_cast<CProcessingItem*>(lvi.lParam));
        PProcessingItem pi = i->second;
        if(PIS_WAIT != pi->State)
            continue;
        if(selected_only && false == pi->Selected)
            continue;

        return pi;
    }
    return nullptr;
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
    if(0 == data)
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
CProcessingItem* CFileListView::GetFocusedItem(int* index /*= nullptr*/)
{
    const int found = GetListCtrl().GetNextItem(-1, LVNI_FOCUSED); 
    if(found < 0) return nullptr;

    if(index) *index = found;
    return FindItem(found);
}
