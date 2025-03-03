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
    DECLARE_MESSAGE_MAP()
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
    void OnSize(UINT nType, int cx, int cy);

    //items
    int FindItem(const CProcessingItem* item);
    CProcessingItem* FindItem(int item_index);
    CProcessingItem* GetFocusedItem(int* index = nullptr);
    void RemoveItem(const CProcessingItem* item);
    void UpdateItem(const CProcessingItem* item);
    void UpdateItem(const CProcessingItem* item, int item_index);
    void UpdateItems();
    void UpdateItemStates();
    void Sort();

    //processing
    void SaveSelection();
    PProcessingItem GetUnprocessedItem(bool selected_only);

protected: 
	CFileListView(); //create from serialization only
	DECLARE_DYNCREATE(CFileListView)

    afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	
    //custom events for file list
    BOOL PreTranslateMessage(MSG* msg);
    HACCEL Accel;

    virtual void OnInitialUpdate(); // called first time after construct

    //drag'n'drop
    DROPEFFECT DropEffect;
    virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD grfKeyState, CPoint point);
    virtual DROPEFFECT OnDragOver(COleDataObject*, DWORD grfKeyState, CPoint point);
    virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
    virtual void OnDragLeave();

private:
    COleDropTarget OleDropTarget;

    CProcessingItem* FindItem(const CPoint& point); 
    void AddItem(const CProcessingItem* item);
};

#ifndef _DEBUG  // debug version in FileListView.cpp
inline CVideoPreviewDoc* CFileListView::GetDocument() const
   { return reinterpret_cast<CVideoPreviewDoc*>(m_pDocument); }
#endif

