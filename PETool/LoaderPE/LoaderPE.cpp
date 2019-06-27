#include "LoaderPE.h"
#include "Encode.h"

CLoaderPE::CLoaderPE()
{
	lpBuffer	= NULL;
	hFile		= NULL;
	dLen		= 0;
	lpImageBuffer = NULL;
	lpNewFileBuff = NULL;

	pImageDosHeader			= NULL;
	pImageNTHeader			= NULL;
	pImageSectionHeader		= NULL;
	pImageFileHeader		= NULL;
	pImageOperFileHeader	= NULL;
	mSectionName.clear();
}

CLoaderPE::CLoaderPE(LPCSTR lpFileName)
{
	CLoaderPE();
	if ((hFile = OpenFile(lpFileName, &OpenBuff, OF_READWRITE)) == HFILE_ERROR)
	{
		TCHAR szError[MAX_PATH] = { 0 };
		StringCchPrintf(szError, MAX_PATH, TEXT("OpenFile is Error,Error Id: %d"), GetLastError());
		MessageBox(NULL, szError, TEXT("Error"), MB_OK);
		return;
	}
	SetFilePointer((HANDLE)hFile, NULL, NULL, FILE_BEGIN);

	dFileLen = GetFileSize((HANDLE)hFile, &dLen);

	lpBuffer = malloc(dFileLen);
	if (!ReadFile((HANDLE)hFile, lpBuffer, dFileLen, &dLen, NULL))
	{
		TCHAR szError[MAX_PATH] = { 0 };
		StringCchPrintf(szError, MAX_PATH, TEXT("ReadFile is Error,Error Id: %d"), GetLastError());
		MessageBox(NULL, szError, TEXT("Error"), MB_OK);
		return;
	}
	if (dFileLen != dLen)
	{
		TCHAR szError[MAX_PATH] = { 0 };
		StringCchPrintf(szError, MAX_PATH, TEXT("ReadFile is Error,Error Id: %d"), GetLastError());
		MessageBox(NULL, szError, TEXT("Error"), MB_OK);
		return;
	}
	nNewFileSize = dFileLen;
	CloseHandle((HANDLE)hFile);
}

void CLoaderPE::SaveSectionName()
{
	for (int i = 0; i < GetPeHeader()->NumberOfSections; ++i)
	{
		mSectionName.insert(pair<BYTE*, BOOL>(GetSectionHeader(i)->Name,TRUE));
	}
}

CLoaderPE::~CLoaderPE()
{
	hFile = NULL;
	if (lpBuffer != NULL)
	{
		free(lpBuffer);
		lpBuffer = NULL;
	}

	pImageDosHeader = NULL;
	pImageNTHeader = NULL;
	pImageSectionHeader = NULL;
	pImageFileHeader = NULL;
	pImageOperFileHeader = NULL;
}

PIMAGE_DOS_HEADER CLoaderPE::GetDosHeader()
{
	return (PIMAGE_DOS_HEADER)lpBuffer;
}

BOOL CLoaderPE::IsPeFile()
{
	if (GetNtHeader()->Signature != 0x4550)
	{
		return FALSE;
	}
	return TRUE;
}

PIMAGE_NT_HEADERS CLoaderPE::GetNtHeader()
{
	return (PIMAGE_NT_HEADERS)((CHAR*)lpBuffer + GetDosHeader()->e_lfanew);
}

PIMAGE_FILE_HEADER CLoaderPE::GetPeHeader()
{
	
	return &GetNtHeader()->FileHeader;
}

PIMAGE_OPTIONAL_HEADER CLoaderPE::GetOperHeader()
{
	return &GetNtHeader()->OptionalHeader;
}

PIMAGE_SECTION_HEADER CLoaderPE::GetSectionHeader(int nIndex)
{
	if (nIndex < 0)
	{
		nIndex = 0;
	}else if (nIndex >= GetPeHeader()->NumberOfSections)
	{
		nIndex = GetPeHeader()->NumberOfSections;
	}
	return (PIMAGE_SECTION_HEADER)((CHAR*)&GetOperHeader()->Magic + GetPeHeader()->SizeOfOptionalHeader) + nIndex;
}

PIMAGE_EXPORT_DIRECTORY CLoaderPE::GetExportDir()
{
	if (GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0)
	{
		MessageBox(NULL, TEXT("û�õ�������"), TEXT("warning"), MB_OK);
		exit(0);
	}
	return (PIMAGE_EXPORT_DIRECTORY)((DWORD)lpBuffer + RVAToOffset(GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, lpBuffer));
}

PIMAGE_BASE_RELOCATION CLoaderPE::GetBaseReloc(INT nIndex)
{
	DWORD bak = RVAToOffset(GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, lpBuffer);
	PDWORD pIndex = (PDWORD)((DWORD)lpBuffer +  RVAToOffset(GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress, lpBuffer));
	if (*pIndex == 0 && *(pIndex + 1) == 0)
	{
		return NULL;
	}
	nIndex = (nIndex < 0 ? 0 : nIndex);
	for (int i = 0; i < nIndex; ++i)
	{
		pIndex = (PDWORD)(((DWORD)pIndex) + *(pIndex + 1));
		if (*pIndex == 0 && *(pIndex + 1) == 0)
		{
			break;
		}
	}

	return (PIMAGE_BASE_RELOCATION)pIndex;
}

BOOL CLoaderPE::FileBuffCopyInImageBuff()
{
	lpImageBuffer = malloc(GetOperHeader()->SizeOfImage);
	memset(lpImageBuffer, 0, GetOperHeader()->SizeOfImage);
	//lpImageBuffer = malloc(GetOperHeader()->SizeOfImage*2);
	if (lpImageBuffer == NULL)
	{
		printf("VirtualAlloc failed.\n");
		return FALSE;
	}
	//���ļ�ͷ����ȥ
	memcpy(lpImageBuffer, lpBuffer, GetOperHeader()->SizeOfHeaders);

	for (int i = 0; i < GetPeHeader()->NumberOfSections; ++i)
	{
		if (GetSectionHeader(i)->SizeOfRawData == 0 || GetSectionHeader(i)->PointerToRawData == 0)
		{
			continue;
		}
		//�ѽڵ����ݿ���ȥ
		memcpy((LPVOID)((CHAR*)lpImageBuffer + GetSectionHeader(i)->VirtualAddress), (LPVOID)((CHAR*)lpBuffer+GetSectionHeader(i)->PointerToRawData),
			GetSectionHeader(i)->SizeOfRawData);
	}

	RedirectHeader();
	
	return TRUE;
}

BOOL CLoaderPE::ImageBuffToFileBuff()
{
	//����ڴ���û���ݣ�ֱ���˳�
	if (lpImageBuffer == NULL)
	{
		printf("lpImageBuffer is null");
		return FALSE;
	}

	lpNewFileBuff = malloc(nNewFileSize);
	memset(lpNewFileBuff, 0, nNewFileSize);
	if (lpNewFileBuff == NULL)
	{
		printf("malloc failed.\n");
		return FALSE;
	}

	memcpy(lpNewFileBuff, lpImageBuffer, pImageOperFileHeader->SizeOfHeaders);

	for (int i = 0; i < pImageFileHeader->NumberOfSections; ++i)
	{
		memcpy((LPVOID)((CHAR*)lpNewFileBuff + (pImageSectionHeader + i)->PointerToRawData), (LPVOID)((CHAR*)lpImageBuffer + (pImageSectionHeader + i)->VirtualAddress),
			(pImageSectionHeader + i)->Misc.VirtualSize);
	}

	return TRUE; 
}

INT CLoaderPE::GetSectionNullSize(int nIndex)
{
	int nNum = 0;
	//��Ϊָ���Ǵ�0��ʼ����ģ���������Ҫ��һ��
	for (int i = GetSectionHeader(nIndex)->PointerToRawData + GetSectionHeader(nIndex)->SizeOfRawData - 1; i > GetSectionHeader(nIndex)->PointerToRawData; --i)
	{
		if (*((CHAR*)lpBuffer + i) == 0)
		{
			nNum++;
		}
		else
		{
			return nNum;
		}
	}
	return nNum;
}

BOOL CLoaderPE::AddSection(LPCSTR szName, SIZE_T nSize)
{
	//�Ȱѽ����ֱ���������
	SaveSectionName();
	//Ȼ���ж���û���ظ��Ľ���
	if (IsSectionName((BYTE*)szName))
	{
		MessageBox(NULL, TEXT("Error"), TEXT("SectionName is Repetition"), MB_OK);
		return FALSE;
	}
	lpBuffer = rMalloc(lpBuffer, dFileLen,nSize);
	//�ڵ�������
	int NumberSection = GetPeHeader()->NumberOfSections;
	//�����һ���ڱ��Ƶ�����һ��λ��
	memcpy(GetSectionHeader(NumberSection), GetSectionHeader(NumberSection - 1), sizeof(IMAGE_SECTION_HEADER));
	//�޸Ľڱ������
	GetPeHeader()->NumberOfSections += 1;
	//�޸������Ĵ�С
	GetOperHeader()->SizeOfImage += nSize;
	//�޸Ľ�����
	strcpy_s((CHAR*)GetSectionHeader(NumberSection)->Name, IMAGE_SIZEOF_SHORT_NAME, szName);
	GetSectionHeader(NumberSection)->Misc.VirtualSize = nSize;
	GetSectionHeader(NumberSection)->SizeOfRawData = nSize;
	//ȡ��
	int nVirSize = GetSectionHeader(NumberSection - 1)->Misc.VirtualSize / 0x1000;
	GetSectionHeader(NumberSection)->VirtualAddress += ((nVirSize + 1) * 0x1000);
	GetSectionHeader(NumberSection)->PointerToRawData += GetSectionHeader(NumberSection - 1)->SizeOfRawData;
	return TRUE;
}

BOOL CLoaderPE::AddSectionForStretch(LPCSTR szName, SIZE_T nSize /*= 0x1000*/)
{
	//�Ȱѽ����ֱ���������
	SaveSectionName();
	//Ȼ���ж���û���ظ��Ľ���
	if (IsSectionName((BYTE*)szName))
	{
		MessageBox(NULL, TEXT("Error"), TEXT("SectionName is Repetition"), MB_OK);
		return FALSE;
	}
	lpImageBuffer = rMalloc(lpImageBuffer, dFileLen, nSize);
	RedirectHeader();
	//�ڵ�������
	int NumberSection = pImageFileHeader->NumberOfSections;
	//�����һ���ڱ��Ƶ�����һ��λ��
	memcpy(pImageSectionHeader + NumberSection, pImageSectionHeader + NumberSection - 1, sizeof(IMAGE_SECTION_HEADER));
	//�޸Ľڱ������
	pImageFileHeader->NumberOfSections += 1;
	//�޸������Ĵ�С
	pImageOperFileHeader->SizeOfImage += nSize;
	//�޸Ľ�����
	strcpy_s((CHAR*)(pImageSectionHeader + NumberSection)->Name, IMAGE_SIZEOF_SHORT_NAME, szName);
	(pImageSectionHeader + NumberSection)->Misc.VirtualSize = nSize;
	(pImageSectionHeader + NumberSection)->SizeOfRawData = nSize;
	//ȡ��
	int nVirSize = (pImageSectionHeader + NumberSection - 1)->Misc.VirtualSize / 0x1000;
	(pImageSectionHeader + NumberSection)->VirtualAddress += ((nVirSize + 1) * 0x1000);
	(pImageSectionHeader + NumberSection)->PointerToRawData += (pImageSectionHeader + NumberSection - 1)->SizeOfRawData;
	return TRUE;
}

void CLoaderPE::RedirectHeader()
{
	pImageDosHeader = (PIMAGE_DOS_HEADER)lpImageBuffer;
	pImageNTHeader = (PIMAGE_NT_HEADERS)((CHAR*)lpImageBuffer + pImageDosHeader->e_lfanew);
	pImageFileHeader = &pImageNTHeader->FileHeader;
	pImageOperFileHeader = &pImageNTHeader->OptionalHeader;
	pImageSectionHeader = (PIMAGE_SECTION_HEADER)((CHAR*)&pImageOperFileHeader->Magic + pImageFileHeader->SizeOfOptionalHeader);
}

BOOL CLoaderPE::SaveFile(LPCSTR szBuff,INT nBufSize, LPCSTR szName)
{
#ifdef UNICODE
	hFile = (HFILE)CreateFile(Encode::ctowc(szName), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
	hFile = (HFILE)CreateFile(szName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif // !UNICODE

	if ((HANDLE)hFile == INVALID_HANDLE_VALUE)
	{
		LPTSTR buf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&buf, 0, NULL);
		MessageBox(NULL, buf, TEXT("FileError"), MB_OK);
		return FALSE;
	}
	if (FALSE == WriteFile((HANDLE)hFile, szBuff, nBufSize, NULL, NULL))
	{
		TCHAR szError[MAX_PATH] = { 0 };
		StringCchPrintf(szError, MAX_PATH, TEXT("WriteFile is Error,Error Id: %d"), GetLastError());
		MessageBox(NULL, szError, TEXT("Error"), MB_OK);
	}
	if (FALSE == FlushFileBuffers((HANDLE)hFile))
	{
		TCHAR szError[MAX_PATH] = { 0 };
		StringCchPrintf(szError, MAX_PATH, TEXT("FlushFileBuffers is Error,Error Id: %d"), GetLastError());
		MessageBox(NULL, szError, TEXT("Error"), MB_OK);
	}
	CloseHandle((HANDLE)hFile);
	return TRUE;
}

LPVOID CLoaderPE::rMalloc(LPVOID ptr, INT nOldSize,INT nAddSize)
{
	nNewFileSize = (nAddSize += nOldSize);
	//���ptr���ڿգ�˵���Ƿ����ڴ�
	if (ptr == NULL)
	{
		ptr = malloc(nOldSize);
		return ptr;
	}
	//������Ҫ�޸�ԭ���ڴ�Ĵ�С
	LPVOID lpNewBuff = malloc(nAddSize);
	memset(lpNewBuff, 0, nAddSize);
	memcpy(lpNewBuff, ptr, nOldSize);
	free(ptr);
	ptr = NULL;
	return lpNewBuff;
}
//�鿴map���Ƿ������key
BOOL CLoaderPE::IsSectionName(BYTE* bName)
{
	return mSectionName.count(bName);
}

INT CLoaderPE::GetFileHeaderBlankSize()
{
	return (CHAR*)lpBuffer + GetOperHeader()->SizeOfHeaders - (CHAR*)GetSectionHeader(GetPeHeader()->NumberOfSections);
}

VOID CLoaderPE::MoveHeaderForDOS()
{
	memmove((CHAR*)lpBuffer + sizeof(IMAGE_DOS_HEADER), (CHAR*)lpBuffer + GetDosHeader()->e_lfanew, sizeof(IMAGE_NT_HEADERS) + sizeof(IMAGE_SECTION_HEADER)*GetPeHeader()->NumberOfSections);
	GetDosHeader()->e_lfanew = sizeof(IMAGE_DOS_HEADER);
}

VOID CLoaderPE::ExpandFinalSection(INT nSize)
{
	lpBuffer = rMalloc(lpBuffer, nNewFileSize, nSize);
	int NumberSection = GetPeHeader()->NumberOfSections - 1;
	GetSectionHeader(NumberSection)->Misc.VirtualSize += nSize;
	GetSectionHeader(NumberSection)->SizeOfRawData += nSize;
	GetOperHeader()->SizeOfImage += nSize;
}

VOID CLoaderPE::PrintExportDir()
{
	CHAR* FunctionName;

	printf("%s\n", (DWORD)lpBuffer + RVAToOffset(GetExportDir()->Name,lpBuffer));

	DWORD dNameOrdinal = RVAToOffset(GetExportDir()->AddressOfNameOrdinals, lpBuffer);
	WORD* pNameOrdinal = (WORD*)((DWORD)lpBuffer + dNameOrdinal);
	DWORD dFuncs = RVAToOffset(GetExportDir()->AddressOfFunctions, lpBuffer);
	DWORD* pFuncs = (DWORD*)((DWORD)lpBuffer + dFuncs);

	INT Num = GetExportDir()->NumberOfFunctions;
	for (int i = 0; i < Num; ++i)
	{
		int j = 0;
		//ѭ����ַid����ű��е�λ��
		for (j; j < Num; ++j)
		{
			if (*(pNameOrdinal+j) == i)
			{
				break;
			}
		}

		DWORD dName = RVAToOffset(GetExportDir()->AddressOfNames + (j * 4), lpBuffer);
		DWORD* RVANames = (DWORD*)((DWORD)lpBuffer + dName);
		DWORD  FOANames = RVAToOffset(*(DWORD*)RVANames, lpBuffer);
		FunctionName = (CHAR*)lpBuffer + FOANames;

		INT nNameOrdinal = *pNameOrdinal + i;
		printf("%d\t0x%08x\t%s\n", nNameOrdinal, *(pFuncs + (nNameOrdinal - GetExportDir()->Base)), FunctionName);
	}
}

DWORD CLoaderPE::GetFuncAddresForName(LPCSTR szFuncName)
{
	for (int i = 0; i < GetExportDir()->NumberOfNames; ++i)
	{
		DWORD dName = RVAToOffset(GetExportDir()->AddressOfNames + (i * 4), lpBuffer);
		DWORD* RVANames = (DWORD*)((DWORD)lpBuffer + dName);
		DWORD  FOANames = RVAToOffset(*(DWORD*)RVANames, lpBuffer);
		CHAR* FunctionName = (CHAR*)lpBuffer + FOANames;
		if (strcmp(FunctionName,szFuncName) == 0)
		{
			DWORD dNameOrdinal = RVAToOffset(GetExportDir()->AddressOfNameOrdinals, lpBuffer);
			WORD* pNameOrdinal = (WORD*)((DWORD)lpBuffer + dNameOrdinal);
			DWORD dFuncs = RVAToOffset(GetExportDir()->AddressOfFunctions, lpBuffer);
			DWORD* pFuncs = (DWORD*)((DWORD)lpBuffer + dFuncs);
			return *(pFuncs + (*pNameOrdinal + i));
		}
	}
	return -1;
}

DWORD CLoaderPE::GetFuncAddresForNumber(INT nNum)
{
	DWORD dNameOrdinal = RVAToOffset(GetExportDir()->AddressOfNameOrdinals, lpBuffer);
	WORD* pNameOrdinal = (WORD*)((DWORD)lpBuffer + dNameOrdinal);
	DWORD dFuncs = RVAToOffset(GetExportDir()->AddressOfFunctions, lpBuffer);
	DWORD* pFuncs = (DWORD*)((DWORD)lpBuffer + dFuncs);
	INT nNameOrdinal = *pNameOrdinal + nNum - 1;	//�����Ǵ�0�±꿪ʼ�ģ����������һ
	return *(pFuncs + (nNameOrdinal - GetExportDir()->Base));
}

VOID CLoaderPE::PrintBaseRrloc()
{
	INT nIndexTab = 0;	//��¼��ĸ���
	PBaseAddr pBase = (PBaseAddr)((DWORD)GetBaseReloc() + 8);
	while (GetBaseReloc(nIndexTab)->VirtualAddress)
	{
		printf("RVA��ַ��%X\n", GetBaseReloc(nIndexTab)->VirtualAddress);
		printf("��Ҫ�޸ĵĵ�ַ��\n");
		PBaseAddr pBase = (PBaseAddr)((DWORD)GetBaseReloc(nIndexTab) + 8);
		DWORD Bak = (GetBaseReloc(nIndexTab)->SizeOfBlock - 8) / 2;
		INT nIndex = 0;
		for (nIndex; nIndex < Bak; ++nIndex)
		{
			if (pBase[nIndex].Flag == 3)
			{
				if (nIndex % 4 == 0 && nIndex != 0)
				{
					printf("\n");
				}
				printf("%X\t", pBase[nIndex].Addr + GetBaseReloc(nIndexTab)->VirtualAddress);
			}
		}
		printf("\n");
		printf("\n");
		nIndexTab += 1;
	}
}

WORD CLoaderPE::GetBaseRelocNum()
{
	INT nNum = 0;
	while (GetBaseReloc(nNum)->VirtualAddress && GetBaseReloc(nNum)->SizeOfBlock)
	{
		nNum += 1;
	}
	return nNum ;
}

VOID CLoaderPE::RepairBaseRrloc(DWORD addr)
{
	INT nIndexTab = 0;	//��¼��ĸ���
	while (GetBaseReloc(nIndexTab)->VirtualAddress)
	{
		PBaseAddr pBase = (PBaseAddr)((DWORD)GetBaseReloc(nIndexTab) + 8);
		DWORD Bak = (GetBaseReloc(nIndexTab)->SizeOfBlock - 8) / 2;
		INT nIndex = 0;
		for (nIndex; nIndex < Bak; ++nIndex)
		{
			if (pBase[nIndex].Flag == 3)
			{
				LPVOID OffsetAddr = (PCHAR)lpBuffer + RVAToOffset(pBase[nIndex].Addr + GetBaseReloc(nIndexTab)->VirtualAddress, lpBuffer);
				DWORD bak = *(DWORD*)OffsetAddr;
				*(DWORD*)OffsetAddr += addr;
			}
		}
		nIndexTab += 1;
	}
}

PIMAGE_IMPORT_DESCRIPTOR CLoaderPE::GetImportTable(INT nIndex /*= 0*/)
{
	return (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)lpBuffer + RVAToOffset(GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, lpBuffer));
}

VOID CLoaderPE::PrintImportTable()
{
	PIMAGE_IMPORT_DESCRIPTOR pImport = GetImportTable();
	INT nIndex = 0;//�����ĸ���
	while ((pImport + nIndex)->Name)
	{
		printf("%s\n", (DWORD)lpBuffer + RVAToOffset((pImport + nIndex)->Name, lpBuffer));
		//����
		PDWORD pIAT = (PDWORD)(DWORD(lpBuffer) + RVAToOffset((pImport + nIndex)->FirstThunk, lpBuffer));
		//��ַ
		PDWORD pINT = (PDWORD)(DWORD(lpBuffer) + RVAToOffset((pImport + nIndex)->OriginalFirstThunk, lpBuffer));
		while (*pIAT)
		{
			//�ж����λ
			if (IMAGE_SNAP_BY_ORDINAL(*pINT))
			{
				printf("��ţ�0x%x \t��ַ��0x%x \n", *pINT & 0xFFFF, *pINT);
			}
			else
			{
				PCHAR pName = (PCHAR)lpBuffer + RVAToOffset(*pINT, lpBuffer) + sizeof(WORD);
				printf("���ƣ�%s \t ��ַ��0x%x\n", pName,*pINT);
			}
			pIAT += 1;
			pINT += 1;
		}
		nIndex += 1;
	}
}

VOID CLoaderPE::PrintBoundImport()
{
	DWORD BoundBase = (DWORD)lpBuffer + RVAToOffset(GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress,lpBuffer);
	PIMAGE_BOUND_IMPORT_DESCRIPTOR pBoundImport = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)BoundBase;
	printf("%s\n", BoundBase + pBoundImport->OffsetModuleName);
}

INT CLoaderPE::GetImportTableNum()
{
	PIMAGE_IMPORT_DESCRIPTOR pImport = GetImportTable();
	INT nIndex = 0;//�����ĸ���
	while ((pImport + nIndex)->Name)
	{
		nIndex += 1;
	}
	return nIndex;
}

BOOL CLoaderPE::MoveImpotrTableForSection()
{
	//���һ����
	INT nIndex = rand() % GetPeHeader()->NumberOfSections;
	INT nSecSize = GetSectionNullSize(nIndex);
	INT nImportSize = GetImportTableNum() * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	//����ýڵĿհ״�С�����е����Ĵ�С
	if (nSecSize < nImportSize + sizeof(IMAGE_IMPORT_DESCRIPTOR))
	{
		return FALSE;
	}
	//��¼�µ�����ƫ��
	DWORD dImportFOA = GetSectionHeader(nIndex)->PointerToRawData + GetSectionHeader(nIndex)->SizeOfRawData - nSecSize;
	//�����µ�����µ��׵�ַ
	DWORD dImportAddr = (DWORD)lpBuffer + dImportFOA;
	//�ɵ����ĵ�ַ
	DWORD dOldImportAddr = DWORD(lpBuffer) + RVAToOffset(GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, lpBuffer);
	memmove((PDWORD)dImportAddr, (PDWORD)dOldImportAddr, nImportSize);
	//�޸��µ������RVA
	GetOperHeader()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = OffsetToRVA(dImportFOA, lpBuffer);
	return TRUE;
}

BOOL CLoaderPE::AddImportTable()
{
	MoveImpotrTableForSection();
	PIMAGE_IMPORT_DESCRIPTOR pImport = GetImportTable();
	INT nImportNum = GetImportTableNum();
	memmove(pImport + nImportNum, pImport + nImportNum - 1, sizeof(IMAGE_IMPORT_DESCRIPTOR));
	return TRUE;
}

BOOL CLoaderPE::InImportTable(LPCSTR szDllName, LPCSTR szFuncName)
{
	if (NULL == szDllName || szFuncName == NULL)
	{
		return FALSE;
	}
	AddImportTable();
	PIMAGE_IMPORT_DESCRIPTOR updateImport = GetImportTable(GetImportTableNum() - 1);
	//���һ���ڣ�����Ҫ�޸ĵ���Ϣ�ӽ�ȥ
	WORD nIndex = rand() % GetPeHeader()->NumberOfSections;
	//��������ڿհ״��Ĵ�С
	INT nSecSize = GetSectionNullSize(nIndex);
	//��¼�¿հ״���ʼ��ƫ��
	DWORD dImportFOA = GetSectionHeader(nIndex)->PointerToRawData + GetSectionHeader(nIndex)->SizeOfRawData - nSecSize;

	//������Ҫ�޸ĵ��׵�ַ
	PDWORD dImportAddr = (PDWORD)((DWORD)lpBuffer + dImportFOA);
	//�޸ĵ���������
	memmove(dImportAddr, szDllName, strlen(szDllName));
	updateImport->Name = OffsetToRVA(dImportFOA, lpBuffer);

	//�޸���ƫ�ƣ���¼������ƫ�ƣ���������IMAGE_IMPORT_BY_NAME�ĵڶ���ֵ����һ��ֵ��WORD���ͣ����Լ����ĳ���
	dImportFOA += (strlen(szDllName) + sizeof(WORD) + 1);
	dImportAddr = (PDWORD)((DWORD)lpBuffer + dImportFOA);
	memmove(dImportAddr, szFuncName, strlen(szFuncName));

	//��¼�����ֱ�ĵ�ַ
	DWORD NameAddr = (DWORD)dImportAddr;
	//��¼�´洢ָ�����Ʊ�ĵ�ַ
	dImportFOA += (strlen(szFuncName) + 1);
	dImportAddr = (PDWORD)((DWORD)lpBuffer + dImportFOA);
	*dImportAddr = NameAddr;
	//��IAT���INT��ָ��������RVA�׵�ַ
	updateImport->OriginalFirstThunk = OffsetToRVA(dImportFOA, lpBuffer);
	updateImport->FirstThunk = OffsetToRVA(dImportFOA, lpBuffer);
	return TRUE;
}

DWORD CLoaderPE::RVAToOffset(DWORD dwRva, PVOID pMapping)
{
	WORD nSections = GetNtHeader()->FileHeader.NumberOfSections;
	if (dwRva < GetSectionHeader()->VirtualAddress)
	{
		return dwRva;
	}
	for (int i = 0; i <= nSections; ++i)
	{
		if ((dwRva >= GetSectionHeader(i)->VirtualAddress) && (dwRva <= GetSectionHeader(i)->VirtualAddress + GetSectionHeader(i)->SizeOfRawData))
		{
			return GetSectionHeader(i)->PointerToRawData + (dwRva - GetSectionHeader(i)->VirtualAddress);
		}
	}
	return -1;
}

DWORD CLoaderPE::OffsetToRVA(DWORD dwRva, PVOID pMapping)
{
	WORD nSections = GetNtHeader()->FileHeader.NumberOfSections;
	if (dwRva < GetSectionHeader()->PointerToRawData)
	{
		return dwRva;
	}
	for (int i = 0; i <= nSections; ++i)
	{
		if ((dwRva >= GetSectionHeader(i)->PointerToRawData) && (dwRva <= GetSectionHeader(i)->PointerToRawData + GetSectionHeader(i)->SizeOfRawData))
		{
			return GetSectionHeader(i)->VirtualAddress + (dwRva - GetSectionHeader(i)->PointerToRawData);
		}
	}
	return -1;
}

