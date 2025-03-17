#pragma once

#define APP_NAME L"Video Preview"
#ifdef _DEBUG
#define APP_BUILD_TYPE L"debug"
#else
#define APP_BUILD_TYPE L"release"
#endif
#define APP_BUILD_DATE _T(__DATE__)
#define APP_FULL_NAME APP_NAME L" - " APP_BUILD_TYPE L" build: " APP_BUILD_DATE
#define APP_COMPANY L"kaluga456"
#define APP_URL L"https://github.com/kaluga456/VideoPreview"