#include "stdafx.h"
#pragma hdrstop
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


// CFileListView

IMPLEMENT_DYNCREATE(CFileListView, CListView)

BEGIN_MESSAGE_MAP(CFileListView, CListView)
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
