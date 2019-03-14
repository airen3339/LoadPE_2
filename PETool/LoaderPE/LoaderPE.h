#pragma once
#include <windows.h>
#include <strsafe.h>
#include <vector>

class CLoaderPE
{
public:
	CLoaderPE();
	CLoaderPE(LPCSTR lpFileName);
	~CLoaderPE();
	//���dosͷ
	PIMAGE_DOS_HEADER GetDosHeader();
	//�Ƿ���PEͷ����PE�ļ��Ƿ���
	BOOL IsPeFile();
	//���NTͷ
	PIMAGE_NT_HEADERS GetNtHeader();
	//���PEͷ
	PIMAGE_FILE_HEADER GetPeHeader();
	//��ÿ�ѡPEͷ
	PIMAGE_OPTIONAL_HEADER GetOperHeader();
	//�������Ŀ¼
	PIMAGE_DATA_DIRECTORY GetDataDir();
	//��ȡ�����ڱ�;nIndex : �ڼ����ڱ�
	PIMAGE_SECTION_HEADER GetSectionHeader(int nIndex = 0);
private:
	HFILE			hFile;
	OFSTRUCT		OpenBuff;
	LARGE_INTEGER	FileSize;
	LPVOID			lpBuffer;
	DWORD			dLen;
	DWORD			dFileLen;
	PCHAR			pImgBuffer;
};