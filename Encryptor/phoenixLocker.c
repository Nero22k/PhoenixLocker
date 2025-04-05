#include <windows.h>
#include "utils.h" // Contains the utils functions
#include "list_files.h" // Contains the SearchFiles function

// If you get undefined reference to `___chkstk_ms' error when compiling then please double check if you have added a big array on the stack because you should not do that.

int start()
{
    PWSTR directory = L"";

    // Parse the command line arguments
    int argc;
    int status = 0;
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
        if(_wcscmp(argv[1], L"--directory") == 0) // Check if the first argument is either --directory (local) or --remote (remote)
        {
            directory = argv[2];
            status = ExecuteSearchFile(directory); // Enumerate the directory passed as argument
        }
        else if(_wcscmp(argv[1], L"--remote") == 0)
        {
            // Check if the directory is UNC path
            directory = argv[2];
            if(PathIsUNCW(directory))
            {
                // UNC path example \\server\share\folder
                status = ExecuteSearchFile(directory);
            }
            else
            {
                writeToConsoleA("The path is not a UNC path\n");
                return 1;
            }
        }
    }
    else
    {
        // Enumerate all drives in the system (C:, D:, E:, etc.)
        status = ExecuteSearchFile(directory);
    }

    return status;
}