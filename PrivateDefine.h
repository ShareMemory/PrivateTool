#ifndef PRIVATE_DEFINE_H
#define PRIVATE_DEFINE_H

#define MAX_PATH 260
#define MAX_SIZE 256
#define NULL 0

#ifdef UNICODE
#define tchar wchar_t
#define tstring wstring
#define tstrcpy wcscpy
#define tstrcpy_s wcscpy_s
#define tstrcat wcscat
#define tstrcat_s wcscat_s
#define tstrlen wcslen
#define tprintf wprintf
#define stprintf swprintf
#define to_tstring to_wstring
#define tstrcmp wcscmp
#define tstrncpy wcsncpy
#define tstrncpy_s wcsncpy_s
#define _tstrlwr _wcslwr
#define _tstrlwr_s _wcslwr_s
#define totlower towlower
#define tifstream wifstream
#define tfopen _wfopen
#define tfopen_s _wfopen_s
#define tsprintf swprintf
#define tsprintf_s swprintf_s

#define __TEXT(quote) L##quote      // r_winnt
#define MAX_COUNT 128
#else
#define tchar char
#define tstring string
#define tstrcpy strcpy
#define tstrcpy_s strcpy_s
#define tstrcat strcat
#define tstrcat_s strcat_s
#define tstrlen strlen
#define tprintf printf
#define stprintf sprintf
#define to_tstring to_string
#define tstrcmp strcmp
#define tstrncpy strncpy
#define tstrncpy_s strncpy_s
#define _tstrlwr _strlwr
#define _tstrlwr_s _strlwr_s
#define totlower tolower
#define tifstream ifstream
#define tfopen fopen
#define tfopen_s fopen_s
#define tsprintf sprintf
#define tsprintf_s sprintf_s

#define __TEXT(quote) quote         // r_winnt
#define MAX_COUNT 256
#endif
#define TEXT(quote) __TEXT(quote)   // r_winnt

#endif

