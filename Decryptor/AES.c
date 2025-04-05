#include "AES.h" // Contains the RSA functions
#include "utils.h" // Contains the utils functions

// AES Decryption using 128 bit key and 128 bit IV (16 bytes each)
BOOL AESDecrypt(PBYTE pbInputData, DWORD dwInputDataSize, PBYTE pbKey, DWORD dwKeyLen, PBYTE pbIV, DWORD dwIVLen, PBYTE* lpDecryptedBuffer, PDWORD dwDecryptedBufferLen)
{
    NTSTATUS Status = NO_ERROR;
	BOOL bResult = FALSE;

	if( !pbInputData || dwInputDataSize <= 0 || !lpDecryptedBuffer || !dwDecryptedBufferLen )
	{
        //MessageBoxW(NULL, L"Invalid params", L"Error", MB_OK);
		return FALSE;
	}

	BCRYPT_ALG_HANDLE hProvider = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	BYTE TempInitVector[16] = {0};
	my_memcpy(TempInitVector, pbIV, dwIVLen); // Copy IV to temp buffer

    //
	// Open Crypto Provider for AES
	//
	Status = BCryptOpenAlgorithmProvider(
		&hProvider,
		BCRYPT_AES_ALGORITHM,
		NULL,
		0 );

	if (!BCRYPT_SUCCESS(Status))
    {
        goto Exit;
    }

    //
	// Set the decryption key
	//
	Status = BCryptGenerateSymmetricKey(
		hProvider,
		&hKey,
		NULL,
		0,
		pbKey,
		dwKeyLen,
		0 );

	if (!BCRYPT_SUCCESS(Status))
    {
        goto Exit;
    }

	//
	// Get Required encrypted buffer length
	//

	if(dwInputDataSize == 0x1000)
	{
		Status = BCryptDecrypt( hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, NULL, 0, dwDecryptedBufferLen, 0);

		if (!BCRYPT_SUCCESS(Status))
		{
			goto Exit;
		}
	}
	else
	{
		Status = BCryptDecrypt( hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, NULL, 0, dwDecryptedBufferLen, BCRYPT_BLOCK_PADDING);

		if (!BCRYPT_SUCCESS(Status))
		{
			goto Exit;
		}
	}

    //
	// Allocate buffer for output ciphertext, VirtualAlloc will be used because we may store huge data
	//
	*lpDecryptedBuffer = (PBYTE)VirtualAlloc(NULL, *dwDecryptedBufferLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (*lpDecryptedBuffer == NULL)
    {
        goto Exit;
    }

    //
	// Perform decryption
	//

	if(dwInputDataSize == 0x1000)
	{
		Status = BCryptDecrypt(hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, *lpDecryptedBuffer, *dwDecryptedBufferLen, dwDecryptedBufferLen, 0);

		if (!BCRYPT_SUCCESS(Status))
		{
			//
			// Since we're returning FALSE we wanna release the heap buffer here.
			//
			HeapFree(GetProcessHeap(), 0, *lpDecryptedBuffer);
			*lpDecryptedBuffer = NULL;

			goto Exit;
		}
	}
	else
	{
		Status = BCryptDecrypt(hKey, pbInputData, dwInputDataSize, NULL, TempInitVector, dwIVLen, *lpDecryptedBuffer, *dwDecryptedBufferLen, dwDecryptedBufferLen, BCRYPT_BLOCK_PADDING);

		if (!BCRYPT_SUCCESS(Status))
		{
			//
			// Since we're returning FALSE we wanna release the heap buffer here.
			//
			HeapFree(GetProcessHeap(), 0, *lpDecryptedBuffer);
			*lpDecryptedBuffer = NULL;

			goto Exit;
		}
	}

	bResult = TRUE;

Exit:
	if( hKey )
		BCryptDestroyKey(hKey);

	if( hProvider )
		BCryptCloseAlgorithmProvider(hProvider, 0);

	return bResult;   
}