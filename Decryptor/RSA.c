#include "RSA.h" // Contains the RSA functions
#include "utils.h" // Contains the utils functions

BOOL BCryptImportPrivateKey(BCRYPT_ALG_HANDLE hProvider, PBYTE lpData, ULONG dwDataLen, BCRYPT_KEY_HANDLE* hKey)
{
    BOOL bResult = FALSE;
	NTSTATUS Status = NO_ERROR;

	ULONG cb = 0;
	PCRYPT_PRIVATE_KEY_INFO PrivateKeyInfo = NULL;
	BCRYPT_RSAKEY_BLOB* prkb = NULL;

    bResult = CryptDecodeObjectEx(
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		PKCS_PRIVATE_KEY_INFO,
		lpData,
		dwDataLen,
		CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG,
		0, (void**)&PrivateKeyInfo, &cb
	);

    if( !bResult )
	{
		goto Exit;
	}

    bResult = CryptDecodeObjectEx(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        CNG_RSA_PRIVATE_KEY_BLOB,
        PrivateKeyInfo->PrivateKey.pbData,
        PrivateKeyInfo->PrivateKey.cbData,
        CRYPT_DECODE_ALLOC_FLAG,
        0, (void**)&prkb, &cb
    ); // Decode the private key from the private key info

    if( !bResult )
    {
        goto Exit;
    }

    Status = BCryptImportKeyPair(
        hProvider,
        NULL,
        BCRYPT_RSAPRIVATE_BLOB,
        hKey,
        (PUCHAR)prkb,
        cb,
        0
    );

    bResult = (Status == NO_ERROR);
    if( !bResult )
    {
        goto Exit;
    }

Exit:
	if( prkb )
		LocalFree(prkb);

	if( PrivateKeyInfo )
		LocalFree(PrivateKeyInfo);

	return bResult;
}

BOOL RSADecrypt(PBYTE pbInputData, DWORD dwInputDataSize, PBYTE* lpDecryptedBuffer, PDWORD dwDecryptedBufferLen)
{
    NTSTATUS Status = NO_ERROR;
	BOOL bResult = FALSE;

	if( !pbInputData || dwInputDataSize <= 0 || !lpDecryptedBuffer || !dwDecryptedBufferLen )
	{
		return FALSE;
	}

    LPCWSTR PRIVATE_KEY = L"MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDDUz4ZWN76A6H5b5OWQKlv/xAWj/uMrP7Hjq+zKhoTVjKu91S/YNs7eu4GuhrCahBJzWL3bFQNvAJ/gFMyjzfsOgy4EQvQkFjnya/a5XaZZsahORK0TIkFoiRJRBpUak0epQtdLP1ZdZ9W5zKwyd0wvlVtsPyoV2Efs0L3sWRfXAIZ4ezaB/w/f1Hox6ojbOs6/fapL3BWXC1g+AADOouuFBvYt75d/wSz5HddRawn7eMM9DfzXRxRv7Ot+SV7mcQjplC0uAlD+6HRMJn/ynIi0lPqE/nA7TRnVasI0VP0HFGuZMWBLfQr45+JzC/LSIYovvM7LocR3bQ7ZpVSbUtdAgMBAAECggEAJLx1Q4MpfNBOGIHZujqzI3NPbU7rOswWpyW4+fODL/vYUHwIHm5zDT7TocRC4CneomLUqEBhlquztJNIEoxJMk7AsaXTGQjtD7spLfxWUAe6d/wp/mseokaamIUMhq/vjUHxXzbk93xd2ZRmPkXZKh/CJX1SJ783byTiBYABpsVtKAkNblgcX5caUAzuNhmijCnlayOR2Qml3ZEfS2EP9s88yOfyG1zXTJ04W0OWtWb9JcoFpoLszFt+Q9wpidqt+1ac6CVxGbcLNMkkLGWJZRnXhzF/5YWy11BnaqaKD7WuzGryk1s58AbrACy7kBgjm2clpTg+AhgsJRc4xIL+KQKBgQDi9Kt0FpHv1YUukS/oBlrqtjUos1cEPFWf7JiBO1AvHeVynFvLNvSzLJVrFMNuLxjMIOVFnrSZF9ZLiKPJClP10WtSaVufmJiMHaEZ9gSiqQP02RmRUaB7VuwI0Oq8f4DyTWwO20g+XPW22W9CmXsx5/8jFxnk3G9R7T6INPcwdwKBgQDcUlGmGCAh792acyX6yrQHdbJh8IILONDdzkhh0YDeP4gZ7Qws56zCu0Ml3FgqGJ2KoaOfnfYUKv5UQzDHR0XkdfDiUsoBl5yjOcNnKpgKJ/M+hTjme2VOVNd8r/0XLinWjMIR8VW2PTyvgngD0NEIP1zjiBtg/Jpo15P4b0FLywKBgQCCpjSZFHEYboNHpl68pAPsf82kS3E5R/5GnO3B30IujXhLfQU+u7kr9+tgBEjoggY1ZG53oH/4bYd4XgVjMyXYKaEthOcwvah7aIcYKc7LUJ6tcdjNFf9bYVNpU9L9hizVz4n4F3XEKMFubEcnQLFxrkrqiV7np5oRwJAx2sO9ywKBgQCRtcOYFJaTuz6VGhiBL7Q2xslN6rEm2NGSEA5g2xzeX6gUdWi6/hGMzCpRjNOmK5dtiw9azi5kEjcX5UQKV4HK4ZGJLPW6bdnDPb2Uql/ZP6iorZT/IZbTYlsNdJruzLIQ2hCzqydjmkZbUO8sntrQGI/HT0nEL+uy8PhEste9VwKBgQDZkggls4H/OuvVNRiPDgiPbTOjJOSYU0cLLpCUHE0CwZVrRopYiZEj3p9LpzmdyUVd4rN7mEpl7P1ZzAHyxKaZ295RPMzpV+Qltow1pYPqydDD5Y2/YnDfWu8RjKUwcQKcSqr8qD3x5hmozV9q+9cP02s4H+8k/ppG/KrZNVdaDA==";

    BCRYPT_ALG_HANDLE hProvider = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

    DWORD dwSkip;
	DWORD dwFlags;

	//LPBYTE pbKeyBuffer = NULL;
	//DWORD dwKeyBufferLen = 0;

	BYTE DERPrivKey[2048] = {0};
	DWORD DERPrivKeyLen = sizeof(DERPrivKey);

    //
	// Open the RSA crypto provider
	//
	Status = BCryptOpenAlgorithmProvider(&hProvider, BCRYPT_RSA_ALGORITHM, NULL, 0);

	bResult = (Status == NO_ERROR); // Check if the provider was opened successfully
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Read public key from disk
	//
	/*bResult = ReadFileWToByteArray(RSA_PRIVATE_KEY_FILENAME, &pbKeyBuffer, &dwKeyBufferLen); // Read the private key from disk
	if( !bResult )
	{
        //MessageBoxW(NULL, L"ReadFileToByteArray failed", L"Error", MB_OK);
		goto Exit;
	}*/

    //
	// Convert PEM to DER
	//
    /*
    bResult = CryptStringToBinaryW((LPCSTR)pbKeyBuffer, 0, CRYPT_STRING_BASE64HEADER, DERPrivKey, &DERPrivKeyLen, NULL, NULL);

    if( !bResult )
	{
        MessageBoxW(NULL, L"CryptStringToBinaryA failed", L"Error", MB_OK);
		goto Exit;
	}
    */
	bResult = CryptStringToBinaryW(PRIVATE_KEY, _wcslen(PRIVATE_KEY), CRYPT_STRING_BASE64, DERPrivKey, &DERPrivKeyLen, &dwSkip, &dwFlags );

    if( !bResult )
	{
		goto Exit;
	}

    //
	// Import RSA Private Key
	//
	bResult = BCryptImportPrivateKey(hProvider, DERPrivKey, DERPrivKeyLen, &hKey);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Get Required encrypted buffer length
	//
	Status = BCryptDecrypt(
		hKey,
		pbInputData,
		dwInputDataSize,
		NULL,
		NULL,
		0,
		NULL,
		0,
		dwDecryptedBufferLen,
		BCRYPT_PAD_PKCS1);

	bResult = (Status == NO_ERROR);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Allocate buffer for output ciphertext, HeapAlloc is used because RSA block sizes are not huge
	//
	*lpDecryptedBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *dwDecryptedBufferLen);

	bResult = (*lpDecryptedBuffer != NULL);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Perform encryption
	//
	Status = BCryptDecrypt(
		hKey,
		pbInputData,
		dwInputDataSize,
		NULL,
		NULL,
		0,
		*lpDecryptedBuffer,
		*dwDecryptedBufferLen,
		dwDecryptedBufferLen,
		BCRYPT_PAD_PKCS1);

	bResult = (Status == NO_ERROR);
	if( !bResult )
	{
		//
		// Since we're returning FALSE we wanna release the heap buffer here.
		//
		HeapFree(GetProcessHeap(), 0, *lpDecryptedBuffer);
		*lpDecryptedBuffer = NULL;

		goto Exit;
	}

Exit:
	/*if( pbKeyBuffer )
		VirtualFree(pbKeyBuffer, 0, MEM_RELEASE);*/

	if( hKey )
		BCryptDestroyKey(hKey);

	if( hProvider )
		BCryptCloseAlgorithmProvider(hProvider, 0);

	return bResult;
}