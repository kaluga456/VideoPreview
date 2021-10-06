#ifndef _APP_STRING_H_
#define _APP_STRING_H_

namespace app {

//STL strings
typedef std::string astring;
typedef std::wstring wstring;
#ifndef _UNICODE
    typedef std::string string;
#else
    typedef std::wstring string;
#endif //_UNICODE

inline size_t strcpy(TCHAR* buffer, const TCHAR* src, size_t buffer_size)
{
    //meaningless
    if(NULL == buffer)
        return 0;

    //treat as empty string coping
    if(NULL == src)
    {
        *buffer = 0;
        return 0;
    }

    //copy
    size_t counter = 0;
    const TCHAR* src_pos = src;
    for(TCHAR* buffer_pos = buffer; ; ++buffer_pos, ++src_pos, ++counter)    
    {
        //end of buffer reached
        if(counter == buffer_size - 1)
        {
            *buffer_pos = 0;
            return counter;
        }

        //copy
        *buffer_pos = *src_pos;

        //end of source string reached
        if(0 == *src_pos)
            return counter;
    }

    return 0;
}

} //namespace app

#endif //_APP_STRING_H_
