#include <windows.h>
#include <bcrypt.h>

BOOL AESDecrypt(PBYTE pbInputData, DWORD dwInputDataSize, PBYTE pbKey, DWORD dwKeyLen, PBYTE pbIV, DWORD dwIVLen, PBYTE* lpDecryptedBuffer, PDWORD dwDecryptedBufferLen);