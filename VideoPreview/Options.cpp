#include "stdafx.h"
#pragma hdrstop
#include "SourceFileTypes.h"
#include "OutputProfile.h"
#include "ProcessingItem.h"
#include "VideoPreview.h"
#include "Options.h"

extern CProcessingItemList ProcessingItemList;
COptions Options;

//TODO:
//options XML structure:
//----------------------------------------------------------
//<?xml version="1.0" encoding="utf-8"?>
//<VideoFrames>
//    <MainWindow Top Left Width Height State />
//    <FileListView ColumnWidth1 ColumnWidth2 ColumnWidth3 SortedColumn SortOrder />
//    <Source FileTypes UseExplorerContextMenu />
//    <Output Path UseSourceLocation OverwriteFiles ActionOnError />
//    <OutputProfiles SelectedProfile>
//        <Profile Name BackgroundColor BorderPadding  
//            WriteHeader HeaderText
//            FrameColumns FrameRows FrameTimeInterval UseTimeInterval
//            OutputWidthType OutputWidth FrameWidth FrameHeight FramePadding BorderPadding
//            TimestampType OutputFileName OutputFileFormat>
//            <HeaderFont Size Wight Charset Color Face />
//            <TimestampFont Size Wight Charset Color Face />
//        </Profile>
//        .....
//    </OutputProfiles>
//    <ProcessingItems SaveProcessingItems>
//        <Item State SourceFile ResultString />
//        .....
//    </ProcessingItems>
//</VideoFrames>
//----------------------------------------------------------

//TODO: options registry structure
//key: value1 value2 [...]
//MSDN: Key names cannot include the backslash character (\), but any other printable character can be used. 
//Value names and data can include the backslash character.
//----------------------------------------------------------
//HKCU\Software\VideoFrames
//    FileListView: ColumnWidth1 ColumnWidth2 ColumnWidth3 SortedColumn SortOrder
//    VideoFileTypes: FileTypes UseExplorerContextMenu
//    Output: Path UseSourceLocation OverwriteFiles ActionOnError
//    OutputProfiles: SelectedProfile
//          <ProfileName>:  BackgroundColor BorderPadding  
//                          WriteHeader HeaderText
//                          FrameColumns FrameRows FrameTimeInterval UseTimeInterval
//                          OutputWidthType OutputWidth FrameWidth FrameHeight FramePadding BorderPadding
//                          TimestampType OutputFileName OutputFileFormat
//                          HeaderFont: Size Wight Charset Color Face
//                          TimestampFont: Size Wight Charset Color Face
//          <ProfileName>: ...
//          ...
//    ProcessingItems: SaveProcessingItems
//          Items: strings{"State:SourceFile:ResultString\n"}
//----------------------------------------------------------

//deafults
const int DEFAULT_LAYOUT_X = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_Y = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_WIDTH = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_HEIGHT = CW_USEDEFAULT;
const int DEFAULT_LAYOUT_WINDOWS_TATE = SW_SHOWDEFAULT;
const int DEFAULT_USE_EXPLORER_CONTEXT_MENU = FALSE;
const int DEFAULT_USE_SOURCE_FILE_LOCATION = TRUE;
const int DEFAULT_OVERWRITE_OUTPUT_FILES = FALSE;
const int DEFAULT_ACTION_ON_ERROR = COptions::ACTION_ON_ERROR_SKIP;
const int DEFAULT_SAVE_FILE_LIST_ON_EXIT = true;
const int DEFAULT_FILE_LIST_COLUMN_WIDTH = 150;
const int DEFAULT_FILE_LIST_SORTED_COLUMN = 0;
const int DEFAULT_FILE_LIST_SORT_ORDER = 0; //TODO: check

COptions::COptions()
{
}

//TODO:
//int COptions::ReadIntAttribute(const tinyxml2::XMLElement* element, LPCTSTR name, int default_value /*= 0*/)
//{
//    if(NULL == element) 
//        return default_value;    
//    int value = 0;
//    return tinyxml2::XML_SUCCESS == element->QueryIntAttribute(name, &value) ? value : default_value;
//}
//LPCTSTR COptions::ReadStringAttribute(const tinyxml2::XMLElement* element, LPCTSTR name, LPCTSTR default_value /*= ""*/)
//{
//    if(NULL == element)
//        return default_value; 
//    LPCTSTR value = element->Attribute(name);
//    return value ? value : default_value;
//}
//void COptions::ReadFontData(LPCTSTR element_name, const tinyxml2::XMLElement* root_element, CFontData& font_data)
//{
//    ASSERT(element_name);
//    ASSERT(root_element);
//    const tinyxml2::XMLElement* element = tinyxml2::XMLConstHandle(root_element->FirstChildElement(element_name)).ToElement();
//    if(NULL == element)
//    {
//        font_data.SetDefault();
//        return;
//    }
//    font_data.Size = ReadIntAttribute(element, "Size", DEFAULT_OP_FONT_SIZE);
//    font_data.Weight = ReadIntAttribute(element, "Weght", DEFAULT_OP_FONT_SIZE);
//    font_data.Charset = ReadIntAttribute(element, "Charset", DEFAULT_OP_FONT_SIZE);
//    font_data.Color = ReadIntAttribute(element, "Color", DEFAULT_OP_FONT_COLOR);
//    font_data.Face = ReadStringAttribute(element, "Face", DEFAULT_OP_FONT_FACE);
//}
//void COptions::WriteFontData(LPCTSTR element_name, tinyxml2::XMLDocument& document, tinyxml2::XMLElement* root_element, const CFontData& font_data)
//{
//    ASSERT(element_name);
//    ASSERT(root_element);
//    tinyxml2::XMLElement* element = root_element->InsertEndChild(document.NewElement(element_name))->ToElement();
//    element->SetAttribute("Size", font_data.Size);
//    element->SetAttribute("Weght", font_data.Weight);
//    element->SetAttribute("Charset", font_data.Charset);
//    element->SetAttribute("Color", static_cast<int>(font_data.Color));
//    element->SetAttribute("Face", font_data.Face.c_str());
//}
bool COptions::Read()
{
    //main
    OutputDirectory = theApp.GetString(_T("OutputPath")); //TODO: defaults to "My Documents"
    UseSourceFileLocation = theApp.GetInt(_T("UseSourceLocation"), DEFAULT_USE_SOURCE_FILE_LOCATION);
    OverwriteOutputFiles = theApp.GetInt(_T("OverwriteFiles"), DEFAULT_OVERWRITE_OUTPUT_FILES);

    CString s = theApp.GetString(_T("SourceFileTypes"), DEFAULT_SOURCE_FILE_TYPES);
    SourceFileTypes.SetString(s);

    ActionOnError = theApp.GetInt(_T("ActionOnError"), DEFAULT_ACTION_ON_ERROR);
    SaveProcessingItems = theApp.GetInt(_T("SaveSourceFileList"), DEFAULT_SAVE_FILE_LIST_ON_EXIT);
    UseExplorerContextMenu = theApp.GetInt(_T("ExplorerContextMenu"), DEFAULT_USE_EXPLORER_CONTEXT_MENU);

    //layout
    ColumnWidth1 = theApp.GetSectionInt(_T("Layout"), _T("ColumnWidth1"), DEFAULT_FILE_LIST_COLUMN_WIDTH);
    ColumnWidth2 = theApp.GetSectionInt(_T("Layout"), _T("ColumnWidth2"), DEFAULT_FILE_LIST_COLUMN_WIDTH);
    ColumnWidth3 = theApp.GetSectionInt(_T("Layout"), _T("ColumnWidth3"), DEFAULT_FILE_LIST_COLUMN_WIDTH);
    SortedColumn = theApp.GetSectionInt(_T("Layout"), _T("SortedColumn"), DEFAULT_FILE_LIST_SORTED_COLUMN);
    SortOrder = theApp.GetSectionInt(_T("Layout"), _T("SortOrder"), DEFAULT_FILE_LIST_SORT_ORDER);

    //output profiles
    //TODO: read <default> as mandatory always-saved profile
    if(FALSE == theApp.GetObject(_T("DefaultProfile"), DefaultProfile))
        DefaultProfile.SetDefault();
    CString selected_output_profile_name = theApp.GetSectionString(_T("OutputProfiles"), _T("SelectedProfile"));
    //TODO: CObject::Serialize
    //OutputProfiles.clear();
    //for(element = tinyxml2::XMLConstHandle(output_profiles_node.FirstChildElement(_T("Profile"))).ToElement(); element != NULL; element = tinyxml2::XMLConstHandle(element->NextSiblingElement("Profile")).ToElement())
    //{
    //    LPCTSTR output_profile_name = element->Attribute("Name");
    //    if(NULL == output_profile_name) continue;

    //    //load output profile
    //    POutputProfile output_profile(new COutputProfile);
    //    OutputProfiles[output_profile_name] = output_profile;
    //    output_profile->BackgroundColor = ReadIntAttribute(element, "BackgroundColor", DEFAULT_OP_BACKGROUND_COLOR);

    //    output_profile->WriteHeader = ReadIntAttribute(element, "WriteHeader", DEFAULT_OP_BACKGROUND_COLOR);
    //    output_profile->HeaderText = ReadStringAttribute(element, "HeaderText", DEFAULT_OP_HEADER_TEXT);
    //    ReadFontData("HeaderFont", element, output_profile->HeaderFont);

    //    output_profile->FrameColumns = ReadIntAttribute(element, "FrameColumns", DEFAULT_OP_FRAME_COLUMNS);
    //    output_profile->FrameRows = ReadIntAttribute(element, "FrameRows", DEFAULT_OP_FRAME_ROWS);
    //    output_profile->FrameTimeInterval = ReadIntAttribute(element, "FrameTimeInterval", DEFAULT_OP_FRAME_TIME_INTERVAL);
    //    output_profile->UseTimeInterval = ReadIntAttribute(element, "UseTimeInterval", DEFAULT_OP_USE_TIME_INTERVAL);

    //    output_profile->OutputSizeMethod = ReadIntAttribute(element, "OutputWidthType", DEFAULT_OP_OUTPUT_IMAGE_WIDTH_TYPE);
    //    output_profile->OutputImageSize = ReadIntAttribute(element, "OutputWidth", DEFAULT_OP_OUTPUT_IMAGE_WIDTH);
    //    output_profile->FramePadding = ReadIntAttribute(element, "FramePadding", DEFAULT_OP_BORDER_PADDING);
    //    output_profile->BorderPadding = ReadIntAttribute(element, "BorderPadding", DEFAULT_OP_FRAME_PADDING);

    //    output_profile->TimestampType = ReadIntAttribute(element, "TimestampType", DEFAULT_TIMESTAMP_TYPE);
    //    ReadFontData("TimestampFont", element, output_profile->TimestampFont);

    //    LPCTSTR output_file_name = ReadStringAttribute(element, "OutputFileName", DEFAULT_OP_OUTPUT_FILE_NAME);
    //    ::strncpy_s(output_profile->OutputFileName, MAX_PATH, output_file_name, _TRUNCATE);
    //    output_profile->OutputFileFormat = ReadIntAttribute(element, "OutputFileFormat", DEFAULT_OP_OUTPUT_FILE_FORMAT);

    //    output_profile->Normalize();
    //    if(0 == ::strcmp(output_profile_name, selected_output_profile_name))
    //        SelectedOutputProfile = output_profile.get();
    //}

    ////processing items
    //tinyxml2::XMLConstHandle processing_items_node = root_element.FirstChildElement("ProcessingItems");
    //element = processing_items_node.ToElement();
    //if(NULL == file_name && SaveProcessingItems != FALSE)
    //{
    //    ProcessingItemList.clear();
    //    for(element = tinyxml2::XMLConstHandle(processing_items_node.FirstChildElement("Item")).ToElement(); element != NULL; element = tinyxml2::XMLConstHandle(element->NextSiblingElement("Item")).ToElement())
    //    {
    //        LPCTSTR source_file_name = element->Attribute("SourceFile");
    //        if(NULL == source_file_name) continue;

    //        const int state = element->IntAttribute("State");
    //        LPCTSTR result_string = element->Attribute("ResultString");
    //        PProcessingItem item(new CProcessingItem(state, source_file_name, result_string));
    //        ProcessingItemList.insert(item);
    //    }
    //}
    return true;
}
bool COptions::Write()
{
    //TODO:
    //main
    theApp.WriteString(_T("OutputPath"), OutputDirectory);
    theApp.WriteInt(_T("UseSourceLocation"), UseSourceFileLocation);
    theApp.WriteInt(_T("OverwriteFiles"), OverwriteOutputFiles);
    theApp.WriteString(_T("SourceFileTypes"), SourceFileTypes.GetString());
    theApp.WriteInt(_T("ActionOnError"), ActionOnError);
    theApp.WriteInt(_T("SaveSourceFileList"), SaveProcessingItems);
    theApp.WriteInt(_T("ExplorerContextMenu"), UseExplorerContextMenu);

    //layout
    theApp.WriteSectionInt(_T("Layout"), _T("ColumnWidth1"), ColumnWidth1);
    theApp.WriteSectionInt(_T("Layout"), _T("ColumnWidth2"), ColumnWidth2);
    theApp.WriteSectionInt(_T("Layout"), _T("ColumnWidth3"), ColumnWidth3);
    theApp.WriteSectionInt(_T("Layout"), _T("SortedColumn"), SortedColumn);
    theApp.WriteSectionInt(_T("Layout"), _T("SortOrder"), SortOrder);

    //NOTE: if file_name is not NULL, then it is export settings call
    //if(NULL == file_name && 0 == SettingsFileName[0]) return false; //settings file name is invalid

    //tinyxml2::XMLDocument settings;

    ////xml declaration
    //tinyxml2::XMLDeclaration* declaration = settings.NewDeclaration(); //by default: <?xml version="1.0" encoding="UTF-8"?>
    //settings.InsertFirstChild(declaration);
    //tinyxml2::XMLNode* root_node = settings.InsertEndChild(settings.NewElement("VidToImg"));
    //tinyxml2::XMLNode* node = NULL;
    //tinyxml2::XMLElement* element = NULL;

    ////main window layout
    //if(NULL == file_name)
    //{
    //    element = root_node->InsertEndChild(settings.NewElement("MainWindow"))->ToElement();
    //    element->SetAttribute("Top", LayoutX);
    //    element->SetAttribute("Left", LayoutY);
    //    element->SetAttribute("Width", LayoutWidth);
    //    element->SetAttribute("Height", LayoutHeight);
    //    element->SetAttribute("State", LayoutWindowState);
    //}

    ////file list view
    //if(NULL == file_name)
    //{
    //    element = root_node->InsertEndChild(settings.NewElement("FileListView"))->ToElement();
    //    element->SetAttribute("ColumnWidth1", ColumnWidth1);
    //    element->SetAttribute("ColumnWidth2", ColumnWidth2);
    //    element->SetAttribute("ColumnWidth3", ColumnWidth3);
    //    element->SetAttribute("SortedColumn", SortedColumn);
    //    element->SetAttribute("SortOrder", SortOrder);
    //}

    ////source files
    //element = root_node->InsertEndChild(settings.NewElement("Source"))->ToElement();
    //std::wstring source_file_types_string;
    //SourceFileTypes.GetString(source_file_types_string);
    //element->SetAttribute("FileTypes", source_file_types_string.c_str());
    //element->SetAttribute("ExplorerContextMenu", UseExplorerContextMenu);

    ////output
    //element = root_node->InsertEndChild(settings.NewElement("Output"))->ToElement();
    //element->SetAttribute("Path", OutputDirectory.c_str());
    //element->SetAttribute("UseSourceLocation", UseSourceFileLocation);
    //element->SetAttribute("OverwriteFiles", OverwriteOutputFiles);
    //element->SetAttribute("ActionOnError", ActionOnError);

    ////output profiles
    theApp.WriteObject(_T("DefaultProfile"), DefaultProfile);
    //node = root_node->InsertEndChild(settings.NewElement("OutputProfiles"));
    //element = node->ToElement();
    //element->SetAttribute("SelectedProfile", GetSelectedOutputProfileName());
    //for(COutputProfiles::const_iterator i = OutputProfiles.begin(); i != OutputProfiles.end(); ++i)
    //{
    //    LPCTSTR output_profile_name = i->first.c_str();
    //    ASSERT(output_profile_name[0]);
    //    const COutputProfile* output_profile = i->second.get();
    //    ASSERT(output_profile);

    //    //save output profile
    //    element = node->InsertEndChild(settings.NewElement("Profile"))->ToElement();
    //    element->SetAttribute("Name", output_profile_name);
    //    element->SetAttribute("BackgroundColor", output_profile->BackgroundColor);
    //    
    //    element->SetAttribute("WriteHeader", output_profile->WriteHeader);
    //    element->SetAttribute("HeaderText", output_profile->HeaderText.c_str());
    //    WriteFontData("HeaderFont", settings, element, output_profile->HeaderFont);

    //    element->SetAttribute("FrameColumns", output_profile->FrameColumns);
    //    element->SetAttribute("FrameRows", output_profile->FrameRows);
    //    element->SetAttribute("UseTimeInterval", output_profile->UseTimeInterval);
    //    element->SetAttribute("FrameTimeInterval", output_profile->FrameTimeInterval);
    //    
    //    element->SetAttribute("OutputWidthType", output_profile->OutputSizeMethod);
    //    element->SetAttribute("OutputWidth", output_profile->OutputImageSize);
    //    element->SetAttribute("FramePadding", output_profile->FramePadding);
    //    element->SetAttribute("BorderPadding", output_profile->BorderPadding);

    //    element->SetAttribute("TimestampType", output_profile->TimestampType);
    //    WriteFontData("TimestampFont", settings, element, output_profile->TimestampFont);

    //    element->SetAttribute("OutputFileName", output_profile->OutputFileName);        
    //    element->SetAttribute("OutputFileFormat", output_profile->OutputFileFormat);
    //}

    ////processing items
    //node = root_node->InsertEndChild(settings.NewElement("ProcessingItems"));
    //element = node->ToElement();
    //element->SetAttribute("SaveProcessingItems", SaveProcessingItems);
    //if(NULL == file_name && SaveProcessingItems != FALSE)
    //{
    //    for(CProcessingItemList::const_iterator i = ProcessingItemList.begin(); i != ProcessingItemList.end(); ++i)
    //    {
    //        PProcessingItem item = *i;
    //        ASSERT(item.get());

    //        element = node->InsertEndChild(settings.NewElement("Item"))->ToElement();
    //        element->SetAttribute("State", item->State);
    //        element->SetAttribute("SourceFile", item->SourceFileName.c_str());
    //        element->SetAttribute("ResultString", item->ResultString.c_str());
    //    }
    //}

    //return tinyxml2::XML_SUCCESS == settings.SaveFile(file_name ? file_name : SettingsFileName);

    return true;
} 
//void ExportProfile(LPCTSTR file_name)
//{
//    HRESULT hr;
//    CComPtr<IStream> pOutFileStream;
//    CComPtr<IXmlWriter> pWriter;
//
//    IStream* stream = NULL;
//    hr = SHCreateStreamOnFile(file_name, STGM_WRITE | STGM_CREATE, &stream);
//    if(FAILED(hr))
//    {
//        AfxMessageBox(_T("FAIL"));
//        return false;
//    }
//    pOutFileStream.Attach(stream);
//
//
//    //Open writeable output stream
//    if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter),(void**) &pWriter, NULL)))
//    {
//        AfxMessageBox(L"Error creating xml writer");
//        return false;
//    }
//
//    if (FAILED(hr = pWriter->SetOutput(pOutFileStream)))
//    {
//        AfxMessageBox(L"Error setting output for writer");
//        return false;
//    }
//
//    pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
//    pWriter->WriteStartDocument(XmlStandalone_Omit); 
//
//    //<VideoPreview>
//    //pWriter->WriteElementString(NULL, L"VideoPreview", NULL, NULL);
//    //pWriter->WriteStartElement(NULL, L"VideoPreview", NULL);
//    //pWriter->WriteAttributeString(NULL, L"Attr1", NULL, L"val");
//    //pWriter->WriteEndElement(); 
//
//    pWriter->WriteStartElement(NULL, L"VideoPreview", NULL);
//        pWriter->WriteElementString(NULL, L"Child", NULL, L"value");
//        pWriter->WriteEndElement(); 
//    pWriter->WriteEndElement(); 
//
//
//    pWriter->WriteEndDocument();
//
//}