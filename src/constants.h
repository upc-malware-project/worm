#pragma once
#define MAX_DOS_DRIVES 26

#define IOCTL_STORAGE_QUERY_PROPERTY (((0x0000002d) << 16) | ((0) << 14) | ((0x0500) << 2) | (0))
#undef NULL
#define NULL 0
#define MAX_PATH 260

#define WINAPI __msabi
#define STDMETHODCALLTYPE __msabi
#define CALLBACK __msabi

#define FARPROC wambda
#define NEARPROC wambda
#define PROC wambda

#define LONG int32_t /* [sic] */
#define WCHAR char16_t /* [sic] */
#define BOOL unsigned char

#define TRUE 1
#define FALSE 0

#define PVOID void*
#define PVOID64 void*
#define LPCVOID const void*
#define CHAR char
#define SHORT short
#define CONST const
#define VOID void
#define INT8 signed char
#define PINT8 signed char*
#define INT16 int16_t
#define PINT16 int16_t*
#define INT32 int32_t
#define PINT32 int32_t*
#define INT64 int64_t
#define PINT64 int64_t*
#define UINT8 unsigned char
#define PUINT8 unsigned char*
#define UINT16 uint16_t
#define PUINT16 uint16_t*
#define UINT32 uint32_t
#define PUINT32 uint32_t*
#define UINT64 uint64_t
#define PUINT64 uint64_t*
#define LONG32 int32_t
#define PLONG32 int32_t*
#define ULONG32 uint32_t
#define PULONG32 uint32_t*
#define DWORD32 uint32_t
#define PDWORD32 uint32_t*

#define INT_PTR intptr_t
#define PINT_PTR intptr_t*
#define UINT_PTR uintptr_t
#define PUINT_PTR uintptr_t*
#define LONG_PTR intptr_t
#define PLONG_PTR int32_t**
#define ULONG_PTR uintptr_t
#define PULONG_PTR uint32_t**
#define POINTER_64_INT int64_t*
#define __int3264 int64_t

#define SHANDLE_PTR int64_t
#define HANDLE_PTR uint64_t

#define UHALF_PTR uint32_t
#define PUHALF_PTR uint32_t*
#define HALF_PTR int32_t
#define PHALF_PTR int32_t*

#define SIZE_T size_t
#define PSIZE_T size_t*
#define SSIZE_T ssize_t
#define PSSIZE_T ssize_t*
#define DWORD_PTR ULONG_PTR
#define PDWORD_PTR ULONG_PTR*
#define LONG64 int64_t
#define PLONG64 int64_t*
#define ULONG64 uint64_t
#define PULONG64 uint64_t*
#define DWORD64 uint64_t
#define PDWORD64 uint64_t*
#define KAFFINITY ULONG_PTR
#define PKAFFINITY KAFFINITY*
#define KPRIORITY LONG

#define PWCHAR WCHAR*
#define LPWCH WCHAR*
#define PWCH WCHAR*
#define LPCWCH CONST WCHAR*
#define PCWCH CONST WCHAR*
#define NWPSTR WCHAR*
#define LPWSTR WCHAR*
#define PWSTR WCHAR*
#define PZPWSTR PWSTR*
#define PCZPWSTR CONST PWSTR*
#define LPUWSTR WCHAR forcealign(1)*
#define PUWSTR WCHAR forcealign(1)*
#define LPCWSTR CONST WCHAR*
#define PCWSTR CONST WCHAR*
#define PZPCWSTR PCWSTR*
#define LPCUWSTR CONST WCHAR forcealign(1)*
#define PCUWSTR CONST WCHAR forcealign(1)*
#define PCHAR CHAR*
#define LPCH CHAR*
#define PCH CHAR*
#define LPCCH CONST CHAR*
#define PCCH CONST CHAR*
#define NPSTR CHAR*
#define LPSTR CHAR*
#define PSTR CHAR*
#define PZPSTR PSTR*
#define PCZPSTR CONST PSTR*
#define LPCSTR CONST CHAR*
#define PCSTR CONST CHAR*
#define PZPCSTR PCSTR*
#define TCHAR WCHAR
#define PTCHAR WCHAR*
#define TBYTE WCHAR
#define PTBYTE WCHAR*
#define LPTCH LPWSTR
#define PTCH LPWSTR
#define PTSTR LPWSTR
#define LPTSTR LPWSTR
#define PCTSTR LPCWSTR
#define LPCTSTR LPCWSTR
#define PUTSTR LPUWSTR
#define LPUTSTR LPUWSTR
#define PCUTSTR LPCUWSTR
#define LPCUTSTR LPCUWSTR
#define LP LPWSTR
#define PSHORT int16_t*
#define PLONG int32_t*
#define HANDLE int64_t
#define PHANDLE HANDLE*
#define FCHAR BYTE
#define FSHORT WORD
#define FLONG DWORD
#define HRESULT LONG
#define CCHAR char
#define LCID DWORD
#define PLCID PDWORD
#define LANGID WORD
#define LONGLONG int64_t
#define ULONGLONG uint64_t
#define USN LONGLONG
#define PLONGLONG LONGLONG*
#define PULONGLONG ULONGLONG*
#define DWORDLONG ULONGLONG
#define PDWORDLONG DWORDLONG*
#define LARGE_INTEGER int64_t
#define PLARGE_INTEGER int64_t*

#define ULONG uint32_t
#define PULONG ULONG*
#define USHORT unsigned short
#define PUSHORT USHORT*
#define UCHAR unsigned char
#define PUCHAR UCHAR*
#define PSZ char*
#define DWORD uint32_t
#define WINBOOL BOOL
#define BOOLEAN BOOL
#define BYTE unsigned char
#define WORD unsigned short
#define FLOAT float
#define PFLOAT FLOAT*
#define PBOOL WINBOOL*
#define PBOOLEAN WINBOOL*
#define LPBOOL WINBOOL*
#define PBYTE BYTE*
#define LPBYTE BYTE*
#define PINT int*
#define LPINT int*
#define PWORD WORD*
#define LPWORD WORD*
#define LPLONG int32_t*
#define PDWORD DWORD*
#define LPDWORD DWORD*
#define LPVOID void*
#define LPCVOID const void*
#define INT int
#define UINT unsigned int
#define PUINT unsigned int*
#define WPARAM UINT_PTR
#define LPARAM LONG_PTR
#define LRESULT LONG_PTR
#define ATOM WORD
#define SPHANDLE HANDLE*
#define LPHANDLE HANDLE*
#define HGLOBAL HANDLE
#define HLOCAL HANDLE
#define GLOBALHANDLE HANDLE
#define LOCALHANDLE HANDLE
#define HGDIOBJ void*
#define PHKEY HKEY*
#define HMODULE HINSTANCE
#define HFILE int
#define HCURSOR HICON
#define COLORREF DWORD
#define LPCOLORREF DWORD*
#define ACCESS_MASK ULONG
#define REGSAM ACCESS_MASK
#define HKEY int64_t
#define SCODE LONG

#define NTSTATUS LONG
#define HACCEL int64_t
#define HBITMAP int64_t
#define HBRUSH int64_t
#define HCOLORSPACE int64_t
#define HDC int64_t
#define HGLRC int64_t
#define HDESK int64_t
#define HENHMETAFILE int64_t
#define HFONT int64_t
#define HICON int64_t
#define HMENU int64_t
#define HMETAFILE int64_t
#define HINSTANCE int64_t
#define HPALETTE int64_t
#define HPEN int64_t
#define HRGN int64_t
#define HRSRC int64_t
#define HSTR int64_t
#define HTASK int64_t
#define HWINSTA int64_t
#define HKL int64_t
#define HMONITOR int64_t
#define HWINEVENTHOOK int64_t
#define HUMPD int64_t
#define HWND int64_t

#define PDH_FUNCTION LONG

#define PDH_HCOUNTER HANDLE
#define PDH_HQUERY HANDLE
#define PDH_HLOG HANDLE
