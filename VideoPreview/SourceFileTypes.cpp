#include "stdafx.h"
#pragma hdrstop
#include "SourceFileTypes.h"

CSourceFileTypes SourceFileTypes;

CSourceFileType::CSourceFileType()
{
    Extension[0] = 0;
}
CSourceFileType::CSourceFileType(LPCTSTR extension)
{
    Init(extension);
}
LPCTSTR CSourceFileType::Init(LPCTSTR extension)
{
    if(NULL == extension)
    {
        Extension[0] = 0;
        return NULL;
    }

    int size = 0;
    for(LPCTSTR pos = extension; ; ++pos, ++size)
    {
        if(_T(';') == *pos) //delimiter
        {
            Extension[size] = 0;
            ::_tcslwr_s(Extension, size + 1);
            return pos + 1;
        }
        if(0 == *pos) //end of string
        {
            Extension[size] = 0;
            ::_tcslwr_s(Extension, size + 1);
            return NULL;
        }
        if(size == SOURCE_FILE_TYPE_MAX_SIZE - 1) //length limit
        {
            Extension[0] = 0;
            ::_tcslwr_s(Extension, size);
            return pos + 1;
        }
        if(::isalnum(*pos))
            Extension[size] = *pos;
        else //invalid character
        {
            Extension[0] = 0;
            return pos + 1;
        }
    }
    return NULL;
}
void CSourceFileTypes::SetString(LPCTSTR source_file_types_string)
{
    SourceFileTypeList.clear();
    if(NULL == source_file_types_string)
        return;

    CSourceFileType file_type;
    for(LPCTSTR pos = source_file_types_string; pos != NULL; pos = file_type.Init(pos))
    {
        if(true == file_type.IsValid())
            SourceFileTypeList.insert(file_type);
    }
}
CString CSourceFileTypes::GetString()
{
    CString result;
    for(CSourceFileTypeList::const_iterator i = SourceFileTypeList.begin(); i != SourceFileTypeList.end(); ++i)
    {
        ASSERT(i->IsValid());
        if(false == i->IsValid())
            continue;
        result += CString(i->Get()) + _T(';');
    }
    return result;
}
bool CSourceFileTypes::GetFilterString(LPTSTR filter_string, UINT size)
{
    //e.g. "Video files\0*.avi;*.mkv;*.mp4;*.mpg;*.wmv\0All Files\0*.*\0\0";
    filter_string[0] = 0;

    UINT pos = 0;
    int result = ::swprintf_s(filter_string, size, _T("%s"), _T("Video files|"));

    if(result < 0)
        return false;
    pos = result + 1;
    for(CSourceFileTypeList::const_iterator i = SourceFileTypeList.begin(); i != SourceFileTypeList.end(); ++i)
    {
        ASSERT(i->IsValid());
        if(false == i->IsValid())
            continue;
        if(pos >= size)
            return false;
        result = ::swprintf_s(filter_string + pos, size - pos, _T("*.%s;"), i->Get());
        if(result < 0)
            return false;
        pos += result;
    }

    ++pos;
    if(pos + (15 * sizeof(TCHAR)) >= size)
        return false;
    memcpy(filter_string + pos, _T("|All Files (*.*)|*.*||"), 15 * sizeof(TCHAR));
    return true;
}
CString CSourceFileTypes::GetFilterString() const
{
    CString all_files = _T("All Files (*.*)|*.*||");
    if(SourceFileTypeList.empty()) return all_files;

    CString result = _T("Video Files|");
    for(CSourceFileTypeList::const_iterator i = SourceFileTypeList.begin(); i != SourceFileTypeList.end(); ++i)
    {
        ASSERT(i->IsValid());
        if(false == i->IsValid())
            continue;

        CString ext;
        ext.Format(_T("*.%s;"), i->Get());
        result += ext;
    }
    result += _T("|");
    return result + all_files;
}
bool CSourceFileTypes::AddType(LPCTSTR extension)
{
    if(NULL == extension)
        return false;

    CSourceFileType file_type(extension);
    if(false == file_type.IsValid())
        return false;

    if(SourceFileTypeList.end() == SourceFileTypeList.find(file_type))
        SourceFileTypeList.insert(file_type);  
    return true;
}
bool CSourceFileTypes::RemoveType(LPCTSTR extension)
{
    if(NULL == extension)
        return false;

    CSourceFileType file_type(extension);
    if(false == file_type.IsValid())
        return false;

    CSourceFileTypeList::iterator i = SourceFileTypeList.find(file_type);
    if(i != SourceFileTypeList.end())
    {
        SourceFileTypeList.erase(i);
        return true;
    }
    return false;
}
bool CSourceFileTypes::HasType(LPCTSTR ext) const
{
    CSourceFileType sft(ext);
    CSourceFileTypeList::const_iterator i = SourceFileTypeList.find(sft);
    return i != SourceFileTypeList.end();
}
bool CSourceFileTypes::CheckName(LPCTSTR file_name) const
{
    if(NULL == file_name) 
        return false;
    LPCTSTR dot_pos = _tcsrchr(file_name, _T('.'));
    if(NULL == dot_pos) 
        return false;
    CString ext = dot_pos + 1;
    ext.MakeLower();
    return HasType(ext);
}