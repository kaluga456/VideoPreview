#include "stdafx.h"
#pragma hdrstop
#include "VideoFileTypes.h"

CVideoFileTypes SourceFileTypes;

CVideoFileType::CVideoFileType()
{
    Extension[0] = 0;
}
CVideoFileType::CVideoFileType(LPCTSTR extension)
{
    Init(extension);
}
LPCTSTR CVideoFileType::Init(LPCTSTR extension)
{
    if(nullptr == extension)
    {
        Extension[0] = 0;
        return nullptr;
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
            return nullptr;
        }
        if(size == VIDEO_FILE_TYPE_MAX_SIZE - 1) //length limit
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
    return nullptr;
}
void CVideoFileTypes::SetString(LPCTSTR source_file_types_string)
{
    VideoFileTypeList.clear();
    if(nullptr == source_file_types_string)
        return;

    CVideoFileType file_type;
    for(LPCTSTR pos = source_file_types_string; pos != nullptr; pos = file_type.Init(pos))
    {
        if(true == file_type.IsValid())
            VideoFileTypeList.insert(file_type);
    }
}
CString CVideoFileTypes::GetString()
{
    CString result;
    for(const auto& i : VideoFileTypeList)
    {
        ASSERT(i.IsValid());
        if(false == i.IsValid())
            continue;
        result += CString(i.Get()) + _T(';');
    }
    return result;
}
bool CVideoFileTypes::GetFilterString(LPTSTR filter_string, UINT size)
{
    //e.g. "Video files\0*.avi;*.mkv;*.mp4;*.mpg;*.wmv\0All Files\0*.*\0\0";
    filter_string[0] = 0;

    UINT pos = 0;
    int result = ::swprintf_s(filter_string, size, _T("%s"), _T("Video files|"));

    if(result < 0)
        return false;
    pos = result + 1;
    for (const auto& i : VideoFileTypeList)
    {
        ASSERT(i.IsValid());
        if(false == i.IsValid())
            continue;
        if(pos >= size)
            return false;
        result = ::swprintf_s(filter_string + pos, size - pos, _T("*.%s;"), i.Get());
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
CString CVideoFileTypes::GetFilterString() const
{
    CString all_files = _T("All Files (*.*)|*.*||");
    if(VideoFileTypeList.empty()) return all_files;

    CString result = _T("Video Files|");
    for (const auto& i : VideoFileTypeList)
    {
        ASSERT(i.IsValid());
        if(false == i.IsValid())
            continue;

        CString ext;
        ext.Format(_T("*.%s;"), i.Get());
        result += ext;
    }
    result += _T("|");
    return result + all_files;
}
bool CVideoFileTypes::AddType(LPCTSTR extension)
{
    if(nullptr == extension)
        return false;

    CVideoFileType file_type(extension);
    if(false == file_type.IsValid())
        return false;

    if(VideoFileTypeList.end() == VideoFileTypeList.find(file_type))
        VideoFileTypeList.insert(file_type);  
    return true;
}
bool CVideoFileTypes::RemoveType(LPCTSTR extension)
{
    if(nullptr == extension)
        return false;

    CVideoFileType file_type(extension);
    if(false == file_type.IsValid())
        return false;

    CVideoFileTypesList::iterator i = VideoFileTypeList.find(file_type);
    if(i != VideoFileTypeList.end())
    {
        VideoFileTypeList.erase(i);
        return true;
    }
    return false;
}
bool CVideoFileTypes::HasType(LPCTSTR ext) const
{
    CVideoFileType sft(ext);
    CVideoFileTypesList::const_iterator i = VideoFileTypeList.find(sft);
    return i != VideoFileTypeList.end();
}
bool CVideoFileTypes::IsVideoFileName(LPCTSTR file_name) const
{
    if(nullptr == file_name)
        return false;
    LPCTSTR dot_pos = _tcsrchr(file_name, _T('.'));
    if(nullptr == dot_pos)
        return false;
    CString ext = dot_pos + 1;
    ext.MakeLower();
    return HasType(ext);
}