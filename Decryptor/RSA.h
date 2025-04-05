#include <windows.h>
#include <bcrypt.h>

#define RSA_PRIVATE_KEY_FILENAME "private.pem"
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

BOOL BCryptImportPrivateKey(BCRYPT_ALG_HANDLE hProvider, PBYTE lpData, ULONG dwDataLen, BCRYPT_KEY_HANDLE* hKey);
BOOL RSADecrypt(PBYTE pbInputData, DWORD dwInputDataSize, PBYTE* lpDecryptedBuffer, PDWORD dwDecryptedBufferLen);
