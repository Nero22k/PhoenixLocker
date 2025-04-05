#include <windows.h>
#include "utils.h" // Contains the utils functions
#include "list_files.h" // Contains the SearchFiles function

int start()
{
    PWSTR directory = L"";

    // Parse the command line arguments
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        // No console window is attached, attempt to create one
        if (!AllocConsole()) {
            return -1;
        }
    }

    Sleep(500); // Sleep for 0.5 seconds to give the parent process time to attach to the console

    if (argc > 1) // Check if there is at least one argument
    {
        if(_wcscmp(argv[1], L"--directory") == 0) // Check if the first argument is --directory
        {
            directory = argv[2];
            ExecuteSearchFile(directory); // Enumerate the directory passed as argument
        }
    }
    else
    {
        // Enumerate all drives in the system (C:, D:, E:, etc.)
        ExecuteSearchFile(directory);
    }

    return 0;
}