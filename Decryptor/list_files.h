#include "utils.h"

#define MAX_QUEUE_SIZE 32

typedef struct FileSearch {
    WCHAR drive[3];
    WCHAR* lpFolder;
} FileSearch;

typedef struct Queue {
    WCHAR directoryPath[MAX_PATH];
    struct Queue* next;
} Queue;

void SearchDirectories(WCHAR* lpFolder);
void enqueue(WCHAR* directoryPath);
BOOL dequeue(WCHAR* directoryPath);
DWORD WINAPI SearchFilesThread(LPVOID lpParam);
DWORD WINAPI workerThread(LPVOID lpParam);
int ExecuteSearchFile(WCHAR* lpDirectory);