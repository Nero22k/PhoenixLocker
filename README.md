```
$$$$$$$\  $$\                                     $$\                 $$\                          $$\                           
$$  __$$\ $$ |                                    \__|                $$ |                         $$ |                          
$$ |  $$ |$$$$$$$\   $$$$$$\   $$$$$$\  $$$$$$$\  $$\ $$\   $$\       $$ |      $$$$$$\   $$$$$$$\ $$ |  $$\  $$$$$$\   $$$$$$\  
$$$$$$$  |$$  __$$\ $$  __$$\ $$  __$$\ $$  __$$\ $$ |\$$\ $$  |      $$ |     $$  __$$\ $$  _____|$$ | $$  |$$  __$$\ $$  __$$\ 
$$  ____/ $$ |  $$ |$$ /  $$ |$$$$$$$$ |$$ |  $$ |$$ | \$$$$  /       $$ |     $$ /  $$ |$$ /      $$$$$$  / $$$$$$$$ |$$ |  \__|
$$ |      $$ |  $$ |$$ |  $$ |$$   ____|$$ |  $$ |$$ | $$  $$<        $$ |     $$ |  $$ |$$ |      $$  _$$<  $$   ____|$$ |      
$$ |      $$ |  $$ |\$$$$$$  |\$$$$$$$\ $$ |  $$ |$$ |$$  /\$$\       $$$$$$$$\\$$$$$$  |\$$$$$$$\ $$ | \$$\ \$$$$$$$\ $$ |      
\__|      \__|  \__| \______/  \_______|\__|  \__|\__|\__/  \__|      \________|\______/  \_______|\__|  \__| \_______|\__|      
                                                                                                                                 
```                                                                                                                                 
How to compile:

- Make sure to have gcc installed preferably via https://www.msys2.org/
- Make sure to have Visual Studio Code installed
- Make sure to have C/C++ dependecies installed via Visual Studio 2022
- Double check your Environment Variables and make sure that you have add C:\msys64\mingw64\bin to system PATH
- Open the Encryption/Decryption folders in VS code and hit run build task

Supported commands:

- Encrypt/Decrypt specific directories --directory <PATH>
- Encrypt/Decrypt specific network directories --remote <PATH> ("\\server\share\folder")
- No arguments will encrypt/decrypt everything on the system

Binaries:

- The Releases folder has pre-compiled binaries

  
# Multi-threaded Encryption
![locker_performance](https://github.com/user-attachments/assets/4488892e-95d2-4089-83e0-6781863bf0ce)
