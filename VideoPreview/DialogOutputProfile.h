#pragma once

class CDialogOutputProfile : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogOutputProfile)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
    bool IsNewProfile; //in
    int IsCopyFrom; //out
    CString ProfileName; //out
    CString CopyFrom; //out

    CDialogOutputProfile(CWnd* parent, bool new_profile = false);
	virtual ~CDialogOutputProfile();

    BOOL OnInitDialog();
    void OnOK();
};
