#include "stdafx.h"
#pragma hdrstop
#include "ProcessingItem.h"

IMPLEMENT_SERIAL(CProcessingItemListSerial, CObject, VERSIONABLE_SCHEMA | 1)

CProcessingItem::CProcessingItem(LPCTSTR source_file_name) : SourceFileName(source_file_name), State(PIS_READY)
{
}
CProcessingItem::CProcessingItem(int state, LPCTSTR source_file_name, LPCTSTR result_string)
{
    ASSERT(source_file_name);
    SourceFileName = source_file_name;
    State = (state != PIS_DONE && state != PIS_FAILED) ? PIS_READY : state;
    if(result_string != NULL && (PIS_DONE == State || PIS_FAILED == State))
        ResultString = result_string;
}
CProcessingItem::~CProcessingItem()
{
}
void CProcessingItem::Reset(bool delete_output_file)
{
    State = PIS_READY;
    ResultString = _T("");

    //TODO:
    //if(delete_output_file && false == ResultString.empty())
    //    DeleteFile(ResultString.c_str());
}

void CProcessingItemListSerial::Serialize(CArchive& archive)
{
    CObject::Serialize(archive);

    ASSERT(ProcessingItemList);
    if(NULL == ProcessingItemList) return;

    if(archive.IsStoring())
    {
        const int count = ProcessingItemList->size();
        archive << count;
        for(CProcessingItemList::const_iterator i = ProcessingItemList->begin(); i != ProcessingItemList->end(); ++i)
        {
            CProcessingItem* pi = i->second.get();
            archive << pi->State;
            archive << pi->SourceFileName;
            archive << pi->ResultString;
        }
    }
    else
    {
        ProcessingItemList->clear();

        int count = 0;
        archive >> count;

        int state = 0;
        CString src_file_name;
        CString result_string;
        for(int i = 0; i < count; ++i)
        {
            archive >> state;
            archive >> src_file_name;
            archive >> result_string;

            PProcessingItem pi(new CProcessingItem(state, src_file_name, result_string));
            (*ProcessingItemList)[pi.get()] = pi;
        }
    }

}