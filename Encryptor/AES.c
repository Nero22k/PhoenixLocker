#include "AES.h" // Contains the RSA functions
#include "utils.h" // Contains the utils functions

// AES Encryption using 128 bit key and 128 bit IV (16 bytes each)
BOOL AESEncrypt(PBYTE pbInputData, DWORD dwInputDataSize, PBYTE pbKey, DWORD dwKeyLen, PBYTE pbIV, DWORD dwIVLen, PBYTE* lpEncryptedBuffer, PDWORD dwEncryptedBufferLen)
{
    NTSTATUS Status = NO_ERROR;
    BOOL bResult = FALSE;

    if (!pbInputData || dwInputDataSize <= 0 || !lpEncryptedBuffer || !dwEncryptedBufferLen)
    {
        return FALSE;
    }

    BCRYPT_ALG_HANDLE hProvider = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;

    BYTE TempInitVector[16] = { 0 };
    my_memcpy(TempInitVector, pbIV, dwIVLen); // Copy IV to temp buffer

    // Open Crypto Provider for AES
    Status = BCryptOpenAlgorithmProvider(&hProvider, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(Status))
    {
        goto Exit;
    }

    // Set the encryption key
    Status = BCryptGenerateSymmetricKey(hProvider, &hKey, NULL, 0, pbKey, dwKeyLen, 0);
    if (!BCRYPT_SUCCESS(Status))
    {
        goto Exit;
    }

	if(dwInputDataSize == 0x1000)
	{
		Status = BCryptEncrypt(hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, NULL, 0, dwEncryptedBufferLen, 0);
		if (!BCRYPT_SUCCESS(Status))
		{
			goto Exit;
		}
	}
	else
	{
		// Get Required encrypted buffer length (with padding)
		Status = BCryptEncrypt(hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, NULL, 0, dwEncryptedBufferLen, BCRYPT_BLOCK_PADDING);
		if (!BCRYPT_SUCCESS(Status))
		{
			goto Exit;
		}
	}

    // Allocate buffer for output ciphertext
    *lpEncryptedBuffer = (PBYTE)VirtualAlloc(NULL, *dwEncryptedBufferLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (*lpEncryptedBuffer == NULL)
    {
        goto Exit;
    }

	if(dwInputDataSize == 0x1000)
	{
		Status = BCryptEncrypt(hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, *lpEncryptedBuffer, *dwEncryptedBufferLen, dwEncryptedBufferLen, 0);
		if (!BCRYPT_SUCCESS(Status))
		{
			HeapFree(GetProcessHeap(), 0, *lpEncryptedBuffer);
			*lpEncryptedBuffer = NULL;
			goto Exit;
		}
	}
	else
	{
		// Perform encryption (with padding)
		Status = BCryptEncrypt(hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, *lpEncryptedBuffer, *dwEncryptedBufferLen, dwEncryptedBufferLen, BCRYPT_BLOCK_PADDING);
		if (!BCRYPT_SUCCESS(Status))
		{
			HeapFree(GetProcessHeap(), 0, *lpEncryptedBuffer);
			*lpEncryptedBuffer = NULL;
			goto Exit;
		}
	}

    bResult = TRUE;

Exit:
    if (hKey)
        BCryptDestroyKey(hKey);

    if (hProvider)
        BCryptCloseAlgorithmProvider(hProvider, 0);

    return bResult;
}