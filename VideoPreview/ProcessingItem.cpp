#include "stdafx.h"
#pragma hdrstop
#include "ProcessingItem.h"

CProcessingItem::CProcessingItem(LPCTSTR source_file_name) : SourceFileName(source_file_name), State(PIS_READY)
{
}
CProcessingItem::CProcessingItem(int state, LPCTSTR source_file_name, LPCTSTR result_string /*= _T("")*/)
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
