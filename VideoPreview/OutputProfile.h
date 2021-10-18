#pragma once

const int MAX_OUTPUT_PROFILE_HEADER_SIZE = 2048; //TODO: required?

enum
{
    SERIALIZE_SCHEMA_VERSION = 1,
    UNKNOWN_SCHEMA_VERSION = -1
};

//output file format
enum
{
    OUTPUT_FILE_FORMAT_BMP,
    OUTPUT_FILE_FORMAT_JPG,
    OUTPUT_FILE_FORMAT_PNG,

    OUTPUT_FILE_FORMAT_COUNT
};

//output image width calculation methods
enum
{
    OUTPUT_IMAGE_WIDTH_BY_ORIGINAL_FRAME,
    OUTPUT_IMAGE_WIDTH_AS_IS,
    OUTPUT_IMAGE_WIDTH_BY_FRAME_WIDTH,
    OUTPUT_IMAGE_WIDTH_BY_FRAME_HEIGHT,

    OUTPUT_IMAGE_WIDTH_COUNT
};

//timestamp position flags
enum
{
    TIMESTAMP_TYPE_LEFT = 0x00000001,
    TIMESTAMP_TYPE_RIGHT = 0x00000002,
    TIMESTAMP_TYPE_CENTER = TIMESTAMP_TYPE_LEFT | TIMESTAMP_TYPE_RIGHT,
    TIMESTAMP_TYPE_BOTTOM = 0x00000004
};

//timestamp position
enum
{
    TIMESTAMP_TYPE_DISABLED = 0x00000000, //don`t write timestamp

    TIMESTAMP_TYPE_TOP_LEFT = TIMESTAMP_TYPE_LEFT,
    TIMESTAMP_TYPE_TOP_CENTER = TIMESTAMP_TYPE_CENTER,
    TIMESTAMP_TYPE_TOP_RIGHT = TIMESTAMP_TYPE_RIGHT,
    TIMESTAMP_TYPE_BOTTOM_LEFT = TIMESTAMP_TYPE_LEFT | TIMESTAMP_TYPE_BOTTOM,
    TIMESTAMP_TYPE_BOTTOM_CENTER = TIMESTAMP_TYPE_CENTER | TIMESTAMP_TYPE_BOTTOM,
    TIMESTAMP_TYPE_BOTTOM_RIGHT = TIMESTAMP_TYPE_RIGHT | TIMESTAMP_TYPE_BOTTOM,

    TIMESTAMP_TYPE_COUNT
};
/////////////////////////////////////////////////////////////////////////////
//default font
const int DEFAULT_OP_FONT_SIZE = -20;
const int DEFAULT_OP_FONT_WEIGTH = FW_NORMAL;
const int DEFAULT_OP_FONT_CHARSET = DEFAULT_CHARSET;
const int DEFAULT_OP_FONT_COLOR = COLORREF(RGB(0xFF, 0xFF, 0xFF));
LPCTSTR const DEFAULT_OP_FONT_FACE = _T("Tahoma");

//background and header deafults
const int DEFAULT_OP_BACKGROUND_COLOR = COLORREF(RGB(0x00, 0x00, 0x00));
const int DEFAULT_OP_WRITE_HEADER = TRUE;
LPCTSTR const DEFAULT_OP_HEADER_TEXT = _T("<default header text>");  //TODO:

//frames count deafults
const int DEFAULT_OP_FRAME_COLUMNS = 4;
const int DEFAULT_OP_FRAME_ROWS = 4;
const int DEFAULT_OP_USE_TIME_INTERVAL = FALSE;
const int DEFAULT_OP_FRAME_TIME_INTERVAL = 60;

//output image width deafults
const int DEFAULT_OP_OUTPUT_IMAGE_WIDTH_TYPE = OUTPUT_IMAGE_WIDTH_AS_IS;
const int DEFAULT_OP_OUTPUT_IMAGE_WIDTH = 800;
const int DEFAULT_OP_BORDER_PADDING = 10;
const int DEFAULT_OP_FRAME_PADDING = 10;

//timestamp defaults
const int DEFAULT_TIMESTAMP_TYPE = TIMESTAMP_TYPE_BOTTOM_RIGHT;

//ouput file deafults
LPCTSTR const DEFAULT_OP_OUTPUT_FILE_NAME = _T("%s"); //TODO:
const int DEFAULT_OP_OUTPUT_FILE_FORMAT = OUTPUT_FILE_FORMAT_JPG; //predefined
/////////////////////////////////////////////////////////////////////////////
//CFontData -font description
class CFontData
{
public:
    int Size;
    int Weight;
    int Charset;
    COLORREF Color;
    CString Face;

    CFontData();
    void SetDefault();
    void Set(const LOGFONT& logfont, COLORREF color);
    void Get(LOGFONT& logfont) const;
};
/////////////////////////////////////////////////////////////////////////////
//COutputProfile - output settings
class COutputProfile : public CObject
{
public:
    //background and header
    int BackgroundColor; //COLORREF
    int WriteHeader; //bool
    CString HeaderText; //format string
    CFontData HeaderFont;

    //frames count
    int FrameColumns;
    int FrameRows;

    //TODO:
    int UseTimeInterval; //bool: true => frames count defined by video duration and TimeInterval
    int FrameTimeInterval; //seconds

    //output image width
    int OutputSizeMethod; //enum
    int OutputImageSize; //pixels
    int BorderPadding; //pixels
    int FramePadding; //pixels

    //timestamp
    int TimestampType; //enum
    CFontData TimestampFont;
   
    //ouput file
    CString OutputFileName; //format string
    int OutputFileFormat; //enum

    //operations
    void operator=(const COutputProfile& src); 
    void SetDefault();
    void Normalize();

    DECLARE_SERIAL(COutputProfile)
    virtual void Serialize(CArchive& archive);
};

typedef boost::shared_ptr<COutputProfile> POutputProfile;



