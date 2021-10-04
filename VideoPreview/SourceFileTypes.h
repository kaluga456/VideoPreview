#pragma once

static const int SOURCE_FILE_TYPE_MAX_SIZE = 6;

//default source file types (must be in lower case)
//TODO: enum all video file extensions
LPCTSTR const DEFAULT_SOURCE_FILE_TYPES = 
    _T("avi;mp4;m4v;mp4v;mpv4;hdmov;mov;3gp;3gpp;3g2;3gp2;flv;f4v;")
    _T("ogm;ogv;rm;ram;rpm;rmm;rt;rp;smi;smil;wmv;wmp;wm;asf;")
    _T("smk;bik;fli;flc;flic;dsm;dsv;dsa;dss;ivf;divx;rmvb;amv"); 

class CSourceFileType
{
public:
    CSourceFileType();
    explicit CSourceFileType(LPCTSTR extension);
    LPCTSTR Init(LPCTSTR extension);
    bool IsValid() const {return Extension[0] != 0;}
    LPCTSTR Get() const {return Extension;}

private:
    TCHAR Extension[SOURCE_FILE_TYPE_MAX_SIZE];
};

class CSourceFileTypeLess
{
public:
    bool operator()(const CSourceFileType& left, const CSourceFileType& right) const
    {
        const int result = ::_tcsicmp(left.Get(), right.Get());
        return result < 0;
    }
};

typedef std::set<CSourceFileType, CSourceFileTypeLess> CSourceFileTypeList;

class CSourceFileTypes
{
public:
    //operation
    void SetString(LPCTSTR source_file_types_string);
    CString GetString();
    bool GetFilterString(LPTSTR filter_string, UINT size); //for OPENFILENAME
    CString GetFilterString() const; //for CFileDialog
    bool AddType(LPCTSTR extension);
    bool RemoveType(LPCTSTR extension);

    bool HasType(LPCTSTR ext) const; //ext must be in lower case

    //data
    CSourceFileTypeList SourceFileTypeList;
};

extern CSourceFileTypes SourceFileTypes;