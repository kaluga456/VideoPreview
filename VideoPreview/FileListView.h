#pragma once

//file list view column ids
enum
{
    FLV_COLUMN_SOURCE_FILE,
    FLV_COLUMN_STATE,       //image or progress bar
    FLV_COLUMN_DESCRIPTION, //error description if any

    FLV_COLUMNS_COUNT
};

class CFileListView : public CListView
{
protected: // create from serialization only
	CFileListView();
	DECLARE_DYNCREATE(CFileListView)

public:
    virtual ~CFileListView();

	CVideoPreviewDoc* GetDocument() const;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	

	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    afx_msg void OnDestroy();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

    virtual void OnInitialUpdate(); // called first time after construct
    virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

    void OnColumnClick(int column_index);
    void Sort();
};

#ifndef _DEBUG  // debug version in FileListView.cpp
inline CVideoPreviewDoc* CFileListView::GetDocument() const
   { return reinterpret_cast<CVideoPreviewDoc*>(m_pDocument); }
#endif

