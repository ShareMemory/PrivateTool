#ifndef PRIVATE_DEFINE_H
#define PRIVATE_DEFINE_H

#define MAX_PATH 260
#define MAX_SIZE 256

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
#define _tstrlwr _wcslwr
#define tfopen _wfopen
#define __TEXT(quote) L##quote      // r_winnt
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
#define _tstrlwr _strlwr
#define tfopen fopen
#define __TEXT(quote) quote         // r_winnt
#endif
#define TEXT(quote) __TEXT(quote)   // r_winnt
#endif