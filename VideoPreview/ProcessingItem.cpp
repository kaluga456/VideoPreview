#include "stdafx.h"
#pragma hdrstop
#include "ClipboardFiles.h"
#include "VideoFileTypes.h"
#include "ProcessingItem.h"

CFileList FileList;

CProcessingItem::CProcessingItem(LPCTSTR source_file_name) : SourceFileName(source_file_name), State(PIS_WAIT), Selected(0)
{
}
CProcessingItem::CProcessingItem(int state, LPCTSTR source_file_name, LPCTSTR result_string /*= _T("")*/) : Selected(0)
{
    ASSERT(source_file_name);
    SourceFileName = source_file_name;
    State = (state != PIS_DONE && state != PIS_FAILED) ? PIS_WAIT : state;
    if(result_string != nullptr && (PIS_DONE == State || PIS_FAILED == State))
        ResultString = result_string;
}
void CProcessingItem::Reset()
{
    State = PIS_WAIT;
    ResultString = _T("");
    Selected = 0;
}

void CFileList::SetBit(int bit, bool value /*= true*/)
{
    if(value) Types |= bit;
    else Types &= ~bit;
}
void CFileList::UpdateTypes()
{
    Types = 0;
    for(auto& i : Items)
    {
        PProcessingItem pi = i.second;
        if(PIS_WAIT == pi->State) SetBit(PILS_HAS_READY);
        else if(PIS_DONE == pi->State) SetBit(PILS_HAS_DONE);
        else if(PIS_FAILED == pi->State) SetBit(PILS_HAS_FAILED);
    }
}
void CFileList::AddFile(LPCTSTR file_name)
{
    if(nullptr == file_name)
        return;

    //ignore duplicates
    for(auto& i : Items)
    {
        PProcessingItem pi = i.second;
        if(0 == pi->SourceFileName.CompareNoCase(file_name))
            return;
    }

    PProcessingItem new_item(new CProcessingItem(file_name));
    Items[new_item.get()] = new_item;
    SetBit(PILS_HAS_READY, true);
}
bool CFileList::AddFiles(CDropFiles* DropFiles)
{
    if(nullptr == DropFiles)
        return false;

    const size_t prev_size = Items.size();
    for(UINT index = 0;; ++index)
    {
        CString file_name(DropFiles->GetFileName(index));
        if(file_name.IsEmpty())
            break;

        if(false == SourceFileTypes.IsVideoFileName(file_name))
            continue;

        const DWORD file_attr = ::GetFileAttributes(file_name);
        if(file_attr & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        AddFile(file_name);
    }

    return Items.size() != prev_size;
}
bool CFileList::RemoveItems(int Type)
{
    const size_t prev_size = Items.size();
    for(CProcessingItemList::iterator i = Items.begin(); i != Items.end();)
    {
        PProcessingItem pi = i->second;
        if(Type == pi->State) 
            i = Items.erase(i);
        else
            ++i;     
    }
    return Items.size() != prev_size;
}