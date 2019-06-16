#pragma once
#include <windows.h>
#include <strsafe.h>
#include <map>
using namespace std;

/************************************************************************/
/*��ӽڣ��ȿ����ļ�ͷ����û����ӽڱ�Ŀռ䣬
	���û�а�PEͷ�Լ����µ�ͷȫ��ͷ���Ƶ�dosͷ֮��
	Ҫ��֤�ڱ�����������ڱ�Ŀռ�Ҳ����80���ֽڡ�	
*/
/************************************************************************/

class CLoaderPE
{
private:
	void SaveSectionName();
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
	//��ȡ�����ڱ�;nIndex : �ڼ����ڱ�
	PIMAGE_SECTION_HEADER GetSectionHeader(int nIndex = 0);
	//Ӳ�̿������ڴ�
	BOOL FileBuffCopyInImageBuff();
	//�ڴ濽����Ӳ��
	BOOL ImageBuffToFileBuff();
	//��ý�ʣ��ռ�Ĵ�С
	INT GetRemainingSize(int nIndex = 0);
	//��ӽ�;In:�ڵ�����
	BOOL AddSection(LPCSTR szName, SIZE_T nSize = 0x1000);
	BOOL AddSectionForStretch(LPCSTR szName, SIZE_T nSize = 0x1000);
	//�ض���ͷ
	void RedirectHeader();
	//�����ļ�
	BOOL SaveFile(LPCSTR szBuff,INT nBufSize, LPCSTR szName);
	// Ҫ�����ڴ��ָ�룬�����С����Ҫ��չ�������С
	LPVOID rMalloc(LPVOID ptr, INT nOldSize,INT nNewSize);
	//�жϽڱ����Ƿ��ظ�
	BOOL IsSectionName(BYTE* bName);
	//����ļ�ͷ�հ״�С
	INT GetFileHeaderBlankSize();
	//���ļ�ͷ�Ƶ�DOSͷ���
	VOID MoveHeaderForDOS();
	//�������һ����
	VOID ExpandFinalSection(INT nSize = 0x1000);
/*
	����:�����ڴ���Ե�ַ���ļ�ƫ�Ƶ�ת��
	������stRVA��    �����ڴ����ƫ�Ƶ�ַ
		  lpFileBuf: �ļ���ʼ��ַ
	���أ�ת������ļ�ƫ�Ƶ�ַ
*/
	DWORD RVAToOffset(DWORD stRVA, PVOID lpFileBuf);
/*
	���ܣ��ļ�ƫ�Ƶ�ַ�������ַ��ת��
	������stOffset���ļ�ƫ�Ƶ�ַ
		  lpFileBuf:�����ڴ���ʼ��ַ
	���أ�ת����������ַ
*/
	DWORD OffsetToRVA(DWORD stOffset, PVOID lpFileBuf);
public:
	LPVOID				lpBuffer;		//Ӳ���е��ļ�
	LPVOID				lpImageBuffer;	//�ڴ��е��ļ�
	LPVOID				lpNewFileBuff;	//�ڴ��е��ļ�
	INT					nNewFileSize;
private:
	HFILE				hFile;
	OFSTRUCT			OpenBuff;
	LARGE_INTEGER		FileSize;
	DWORD				dLen;
	DWORD				dFileLen;
	map<BYTE*, BOOL>	mSectionName;
public:
	PIMAGE_DOS_HEADER		pImageDosHeader;
	PIMAGE_NT_HEADERS		pImageNTHeader;
	PIMAGE_SECTION_HEADER	pImageSectionHeader;
	PIMAGE_FILE_HEADER		pImageFileHeader;
	PIMAGE_OPTIONAL_HEADER	pImageOperFileHeader;
};

enum ETable
{
	ExportTable			= 0,	//������
	ImportTable			= 1,	//�����
	Resource			= 2,	//��Դ
	Abnormal			= 3,	//�쳣
	Certificate			= 4,	//��ȫ֤��
	ResetTable			= 5,	//�ض�λ��
	DebugInformation	= 6,	//������Ϣ
	Copyright			= 7,	//��Ȩ����
	GlobalPointer		= 8,	//ȫ��ָ��
	TLSTable			= 9,	//TLS��
	LoadConfig			= 10,	//��������
	BindImport			= 11,	//�󶨵���
	IATTable			= 12,	//IAT��
	DelayedImport		= 13,	//�ӳٵ���
	COM					= 14,
	Reserved			= 15	//������
};