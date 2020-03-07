#include "stdafx.h"
#pragma hdrstop
#include "OutputProfile.h"

COutputProfiles OutputProfiles;
COutputProfile* SelectedOutputProfile = NULL;
COutputProfile DefaultProfile;

LPCTSTR GetSelectedOutputProfileName()
{
    for(COutputProfiles::const_iterator i = OutputProfiles.begin(); i != OutputProfiles.end(); ++i)
    {
        if(SelectedOutputProfile == i->second.get())
            return i->first;
    }
    return _T("");
}
void SetSelectedOutputProfile(LPCTSTR output_profile_name)
{
    ASSERT(output_profile_name);
    for(COutputProfiles::const_iterator i = OutputProfiles.begin(); i != OutputProfiles.end(); ++i)
    {
        if(0 == ::_tcscmp(output_profile_name, i->first))
        {
            SelectedOutputProfile = i->second.get();
            return;
        }
    }
    SelectedOutputProfile = NULL;
}

CFontData::CFontData()
{
    SetDefault();
}
void CFontData::SetDefault()
{
    Size = DEFAULT_OP_FONT_SIZE;
    Weight = DEFAULT_OP_FONT_WEIGTH;
    Charset = DEFAULT_OP_FONT_CHARSET;
    Color = DEFAULT_OP_FONT_COLOR;
    Face = DEFAULT_OP_FONT_FACE;
}
void CFontData::Set(const LOGFONT& logfont, COLORREF color)
{
    Size = logfont.lfHeight;
    Weight = logfont.lfWeight;
    Charset = logfont.lfCharSet;
    Face = logfont.lfFaceName;
    Color = color;
}
void CFontData::Get(LOGFONT& logfont) const
{
    logfont.lfHeight = Size;
    logfont.lfWidth = 0;
    logfont.lfEscapement = 0;
    logfont.lfOrientation = 0;
    logfont.lfWeight = Weight;
    logfont.lfItalic = FALSE;
    logfont.lfUnderline = FALSE;
    logfont.lfStrikeOut = FALSE;
    logfont.lfCharSet = Charset;
    logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    ::_tcsncpy_s(logfont.lfFaceName, LF_FACESIZE, Face, _TRUNCATE);
}
CArchive& operator<<(CArchive& archive, const CFontData& font_data)
{
    archive << font_data.Size;
    archive << font_data.Weight;
    archive << font_data.Charset;
    archive << font_data.Color;
    archive << font_data.Face;
    return archive;
}
CArchive& operator>>(CArchive& archive, CFontData& font_data)
{
    archive >> font_data.Size;
    archive >> font_data.Weight;
    archive >> font_data.Charset;
    archive >> font_data.Color;
    archive >> font_data.Face;
    return archive;
}

IMPLEMENT_SERIAL(COutputProfile, CObject, VERSIONABLE_SCHEMA | 1)

void COutputProfile::SetDefault()
{
    //background and header
    BackgroundColor = DEFAULT_OP_BACKGROUND_COLOR;
    WriteHeader = DEFAULT_OP_WRITE_HEADER;
    HeaderFont.SetDefault();

    //frames count
    FrameColumns = DEFAULT_OP_FRAME_COLUMNS;
    FrameRows = DEFAULT_OP_FRAME_ROWS;
    UseTimeInterval = FALSE;
    FrameTimeInterval = DEFAULT_OP_FRAME_TIME_INTERVAL;

    //output image width
    OutputSizeMethod = DEFAULT_OP_OUTPUT_IMAGE_WIDTH_TYPE;
    OutputImageSize = DEFAULT_OP_OUTPUT_IMAGE_WIDTH;
    BorderPadding = DEFAULT_OP_BORDER_PADDING;
    FramePadding = DEFAULT_OP_FRAME_PADDING;

    //timestamp
    TimestampType = TIMESTAMP_TYPE_BOTTOM_RIGHT; //predefined
    TimestampFont.SetDefault();
   
    //ouput
    OutputFileName = DEFAULT_OP_OUTPUT_FILE_NAME; 
    OutputFileFormat = OUTPUT_FILE_FORMAT_JPG; //predefined
}
void COutputProfile::Normalize()
{
    //TODO:
}
void COutputProfile::operator=(const COutputProfile& objectSrc)
{
    //TODO: copy for processing thread or block writing?
}

void COutputProfile::Serialize(CArchive& archive)
{
    CObject::Serialize(archive);

    ////frames count
    //int FrameColumns;
    //int FrameRows;
    //int UseTimeInterval; //bool: true => frames count defined by video duration and TimeInterval
    //int FrameTimeInterval; //seconds

    if(archive.IsLoading())
    {
        int schema = 0;
        archive >> schema;
        if(schema != SERIALIZE_SCHEMA_VERSION)
        {
            SetDefault();
            return;
        }

        archive >> BackgroundColor;

        //header
        archive >> WriteHeader;
        archive >> HeaderText;
        archive >> HeaderFont;

        //frame grid
        archive >> FrameColumns;
        archive >> FrameRows;

        //TODO:
        //archive >> UseTimeInterval;
        //archive >> FrameTimeInterval;

        //output size
        archive >> OutputSizeMethod;
        archive >> OutputImageSize;
        archive >> BorderPadding;
        archive >> FramePadding;

        //timestamp
        archive >> TimestampType;
        archive >> TimestampFont;

        //ouput file
        archive >> OutputFileName;
        archive >> OutputFileFormat;
    }
    else
    {
        archive << int(SERIALIZE_SCHEMA_VERSION);
       
        archive << BackgroundColor;

        //header
        archive << WriteHeader;
        archive << HeaderText;
        archive << HeaderFont;

        //frame grid
        archive << FrameColumns;
        archive << FrameRows;

        //TODO:
        //archive << UseTimeInterval;
        //archive << FrameTimeInterval;

        //output size
        archive << OutputSizeMethod;
        archive << OutputImageSize;
        archive << BorderPadding;
        archive << FramePadding;

        //timestamp
        archive << TimestampType;
        archive << TimestampFont;

        //ouput file
        archive << OutputFileName;
        archive << OutputFileFormat;
    }
}

