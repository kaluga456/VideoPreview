#pragma once

static const int VIDEO_FILE_TYPE_MAX_SIZE = 6;

//default source file types (must be in lower case)
LPCTSTR const DEFAULT_VIDEO_FILE_TYPES = 
    _T("avi;mkv;mp4;m4v;mp4v;mpv4;hdmov;mov;3gp;3gpp;3g2;3gp2;flv;f4v;")
    _T("ogm;ogv;rm;ram;rpm;rmm;rt;rp;smi;smil;wmv;wmp;wm;asf;")
    _T("smk;bik;fli;flc;flic;dsm;dsv;dsa;dss;ivf;divx;rmvb;amv"); 

class CVideoFileType
{
public:
    CVideoFileType();
    explicit CVideoFileType(LPCTSTR extension);
    LPCTSTR Init(LPCTSTR extension);
    bool IsValid() const {return Extension[0] != 0;}
    LPCTSTR Get() const {return Extension;}

private:
    TCHAR Extension[VIDEO_FILE_TYPE_MAX_SIZE]; //must be always in lower case
};

class CVideoFileTypesLess
{
public:
    bool operator()(const CVideoFileType& left, const CVideoFileType& right) const
    {
        return ::_tcscmp(left.Get(), right.Get()) < 0;
    }
};

typedef std::set<CVideoFileType, CVideoFileTypesLess> CVideoFileTypesList;

class CVideoFileTypes
{
public:
    void SetString(LPCTSTR source_file_types_string);
    CString GetString();

    bool GetFilterString(LPTSTR filter_string, UINT size); //for OPENFILENAME
    CString GetFilterString() const; //for CFileDialog

    bool AddType(LPCTSTR extension);
    bool RemoveType(LPCTSTR extension);

    bool HasType(LPCTSTR ext) const;
    bool IsVideoFileName(LPCTSTR file_name) const;

    CVideoFileTypesList VideoFileTypeList;
};

extern CVideoFileTypes SourceFileTypes;