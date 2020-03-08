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

//sorting callbacks
static int CALLBACK FileListViewSortBySource(LPARAM param1, LPARAM param2, LPARAM ascending)
{
    ASSERT(param1);
    ASSERT(param2);
    const CProcessingItem* left = reinterpret_cast<const CProcessingItem*>(param1);
    const CProcessingItem* right = reinterpret_cast<const CProcessingItem*>(param2);
    const int result = ::_tcsicmp(left->SourceFileName, right->SourceFileName);
    return ascending ? result : -result;
}
static int CALLBACK FileListViewSortByState(LPARAM param1, LPARAM param2, LPARAM ascending)
{
    ASSERT(param1);
    ASSERT(param2);
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

    int column_width1 = Options.ColumnWidth1;
    int column_width2 = Options.ColumnWidth2;
    int column_width3 = Options.ColumnWidth3;

    //validate column widths
    if(column_width1 <= 0) column_width1 = 10;
    if(column_width2 <= 0) column_width2 = 10;
    if(column_width3 <= 0) column_width3 = 10;

    listCtrl.SetColumnWidth(0, column_width1);
    listCtrl.SetColumnWidth(1, column_width2);
    listCtrl.SetColumnWidth(2, column_width3);

    //TEST:
    i = listCtrl.InsertItem(0, _T("item 1"));
    i = listCtrl.InsertItem(0, _T("item 2"));
    i = listCtrl.InsertItem(0, _T("item 3"));

    CHeaderCtrl* headerCtrl = listCtrl.GetHeaderCtrl();
    headerCtrl->ShowWindow(SW_SHOW);
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
    CListCtrl& listCtrl = GetListCtrl();
    if(FLV_COLUMN_SOURCE_FILE == Options.SortedColumn)
        listCtrl.SortItems(FileListViewSortBySource, Options.SortOrder);
    else if(FLV_COLUMN_STATE == Options.SortedColumn)
        listCtrl.SortItems(FileListViewSortByState, Options.SortOrder);
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
