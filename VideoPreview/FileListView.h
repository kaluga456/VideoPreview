#pragma once

//file list view column ids
enum
{
    FLV_COLUMN_VIDEO,
    FLV_COLUMN_STATE,       //image or progress bar
    FLV_COLUMN_RESULT, //error description if any

    FLV_COLUMNS_COUNT
};

class CFileListView : public CListView
{
private:
    CMenu PopupMenu;
    CProcessingItem* FindItem(const CPoint& point); 
    void AddItem(const CProcessingItem* item);

protected: // create from serialization only
	CFileListView();
	DECLARE_DYNCREATE(CFileListView)

// Generated message map functions
protected:
    afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

    virtual void OnInitialUpdate(); // called first time after construct

public:
    virtual ~CFileListView();

	CVideoPreviewDoc* GetDocument() const;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    //events
    void OnColumnClick(int column_index);

    //items
    int FindItem(const CProcessingItem* item);
    CProcessingItem* FindItem(int item_index);
    CProcessingItem* GetFocusedItem(int* index = NULL);
    PProcessingItem GetUnprocessedItem();
    void RemoveItem(const CProcessingItem* item);
    void UpdateItem(const CProcessingItem* item);
    void UpdateItem(const CProcessingItem* item, int item_index);
    void UpdateItems();
    void Sort();
};

#ifndef _DEBUG  // debug version in FileListView.cpp
inline CVideoPreviewDoc* CFileListView::GetDocument() const
   { return reinterpret_cast<CVideoPreviewDoc*>(m_pDocument); }
#endif

