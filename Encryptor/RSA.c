#include "RSA.h" // Contains the RSA functions
#include "utils.h" // Contains the utils functions

BOOL BCryptImportPublicKey(BCRYPT_ALG_HANDLE hProvider, PBYTE lpData, ULONG dwDataLen, BCRYPT_KEY_HANDLE* hKey)
{
    BOOL bResult = FALSE;
	NTSTATUS Status = NO_ERROR;

    union
	{
		PVOID pvStructInfo;
		PCERT_INFO pCertInfo;
		PCERT_PUBLIC_KEY_INFO PublicKeyInfo;
	} info;

    ULONG cb = 0;
	BCRYPT_RSAKEY_BLOB* prkb = NULL;

    bResult = CryptDecodeObjectEx( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		X509_PUBLIC_KEY_INFO,
		lpData,
		dwDataLen,
		CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG, 0, &info.pvStructInfo, &cb ); // Get the public key info

	if( !bResult )
	{
		goto Exit;
	}

    bResult = CryptDecodeObjectEx( X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		CNG_RSA_PUBLIC_KEY_BLOB,
		info.PublicKeyInfo->PublicKey.pbData,
		info.PublicKeyInfo->PublicKey.cbData,
		CRYPT_DECODE_ALLOC_FLAG, 0, ( void** )&prkb, &cb ); // Get the public key blob

	if( !bResult )
	{
		goto Exit;
	}

    Status = BCryptImportKeyPair(
		hProvider,
		NULL,
		BCRYPT_RSAPUBLIC_BLOB,
		hKey,
		( PUCHAR )prkb,
		cb,
		0 );

	bResult = (Status == NO_ERROR);
	if( !bResult )
	{
		goto Exit;
	}

Exit:
    if( prkb )
		LocalFree(prkb);

	if( info.pvStructInfo )
		LocalFree(info.pvStructInfo);

	return bResult;
}

BOOL RSAEncrypt(PBYTE pbInputData, DWORD dwInputDataSize, PBYTE* lpEncryptedBuffer, PDWORD dwEncryptedBufferLen)
{
    NTSTATUS Status = NO_ERROR;
	BOOL bResult = FALSE;

	if( !pbInputData || dwInputDataSize <= 0 || !lpEncryptedBuffer || !dwEncryptedBufferLen )
	{
        //MessageBoxW(NULL, L"Invalid params", L"Error", MB_OK);
		return FALSE;
	}

	LPCWSTR PUBLIC_KEY = L"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw1M+GVje+gOh+W+TlkCpb/8QFo/7jKz+x46vsyoaE1YyrvdUv2DbO3ruBroawmoQSc1i92xUDbwCf4BTMo837DoMuBEL0JBY58mv2uV2mWbGoTkStEyJBaIkSUQaVGpNHqULXSz9WXWfVucysMndML5VbbD8qFdhH7NC97FkX1wCGeHs2gf8P39R6MeqI2zrOv32qS9wVlwtYPgAAzqLrhQb2Le+Xf8Es+R3XUWsJ+3jDPQ3810cUb+zrfkle5nEI6ZQtLgJQ/uh0TCZ/8pyItJT6hP5wO00Z1WrCNFT9BxRrmTFgS30K+Oficwvy0iGKL7zOy6HEd20O2aVUm1LXQIDAQAB";

    BCRYPT_ALG_HANDLE hProvider = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	DWORD dwSkip;
	DWORD dwFlags;

	//LPBYTE pbKeyBuffer = NULL;
	//DWORD dwKeyBufferLen = 0;

	BYTE DERPubKey[2048] = {0};
	DWORD DERPubKeyLen = sizeof(DERPubKey);

    //
	// Open the RSA crypto provider
	//
	Status = BCryptOpenAlgorithmProvider(&hProvider, BCRYPT_RSA_ALGORITHM, NULL, 0 );

	bResult = (Status == NO_ERROR);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Read public key from disk
	//
	/*bResult = stringToByteArray(PUBLIC_KEY, &pbKeyBuffer, &dwKeyBufferLen);
    if(!bResult)
	{
		MessageBoxW(NULL, L"stringToByteArray failed", L"Error", MB_OK);
		goto Exit;
	}*/

    //
	// Convert PEM to DER
	//
	bResult = CryptStringToBinaryW(PUBLIC_KEY, _wcslen(PUBLIC_KEY), CRYPT_STRING_BASE64, DERPubKey, &DERPubKeyLen, &dwSkip, &dwFlags );

	if( bResult == FALSE ) // For some reason it fails with error code 13 
	{
		goto Exit;
	}

    //
	// Import RSA Public Key
	//
	bResult = BCryptImportPublicKey(hProvider, DERPubKey, DERPubKeyLen, &hKey);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Get Required encrypted buffer length
	//
	Status = BCryptEncrypt(
		hKey,
		pbInputData,
		dwInputDataSize,
		NULL,
		NULL,
		0,
		NULL,
		0,
		dwEncryptedBufferLen,
		BCRYPT_PAD_PKCS1);

	bResult = (Status == NO_ERROR);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Allocate buffer for output ciphertext, HeapAlloc is used because RSA block sizes are not huge
	//
	*lpEncryptedBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *dwEncryptedBufferLen);

	bResult = (*lpEncryptedBuffer != NULL);
	if( !bResult )
	{
		goto Exit;
	}

    //
	// Perform encryption
	//
	Status = BCryptEncrypt(
		hKey,
		pbInputData,
		dwInputDataSize,
		NULL,
		NULL,
		0,
		*lpEncryptedBuffer,
		*dwEncryptedBufferLen,
		dwEncryptedBufferLen,
		BCRYPT_PAD_PKCS1);

	bResult = (Status == NO_ERROR);
	if( !bResult )
	{
		//
		// Since we're returning FALSE we wanna release the heap buffer here.
		//
		HeapFree(GetProcessHeap(), 0, *lpEncryptedBuffer);
		*lpEncryptedBuffer = NULL;
		goto Exit;
	}

Exit:
	/*if( pbKeyBuffer )
	{
		HeapFree(GetProcessHeap(), 0, pbKeyBuffer);
		pbKeyBuffer = NULL;
	}*/

	if( hKey )
		BCryptDestroyKey(hKey);

	if( hProvider )
		BCryptCloseAlgorithmProvider(hProvider, 0);

	return bResult;
}