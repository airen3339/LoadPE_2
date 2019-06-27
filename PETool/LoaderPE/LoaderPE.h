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
	//��������ṹ
	PIMAGE_EXPORT_DIRECTORY GetExportDir();
	//����ض�λ��
	PIMAGE_BASE_RELOCATION GetBaseReloc(INT nIndex = 0);

	DWORD RVAToOffset(DWORD dwRva, PVOID pMapping);
	DWORD OffsetToRVA(DWORD dwRva, PVOID pMapping);
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
	PIMAGE_SECTION_HEADER GetSectionHeader(int nIndex = 1);
	//Ӳ�̿������ڴ�
	BOOL FileBuffCopyInImageBuff();
	//�ڴ濽����Ӳ��
	BOOL ImageBuffToFileBuff();
	//��ý�ʣ��ռ�Ĵ�С
	INT GetSectionNullSize(int nIndex = 0);
	//��ӽ�;In:�ڵ�����
	BOOL AddSection(LPCSTR szName, SIZE_T nSize = 0x1000);
	//��չ���һ����
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
	//��ӡ�����
	VOID PrintExportDir();
	//���ݺ������ҵ�������ַ
	DWORD GetFuncAddresForName(LPCSTR szFuncName);
	//��������ҵ�������ַ
	DWORD GetFuncAddresForNumber(INT nNum);
	//����ض�λ��
	VOID PrintBaseRrloc();
	//����ض�λ��ĸ���
	WORD GetBaseRelocNum();
	//�޸��ض�λ��
	VOID RepairBaseRrloc(DWORD addr);
	//��õ����
	PIMAGE_IMPORT_DESCRIPTOR GetImportTable(INT nIndex = 0);
	//��������
	VOID PrintImportTable();
	//����󶨵����
	VOID PrintBoundImport();
	//��õ����ĸ���
	INT GetImportTableNum();
	//�ƶ��������; nIndex:�ڼ�����
	BOOL MoveImpotrTableForSection(INT nIndex = 0);
	
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
	//�ض����
	typedef struct BaseRelocAddress
	{
		WORD Addr	: 12;
		WORD Flag  : 4;
	}BaseAddr,*PBaseAddr;

public:
	PIMAGE_DOS_HEADER		pImageDosHeader;
	PIMAGE_NT_HEADERS		pImageNTHeader;
	PIMAGE_SECTION_HEADER	pImageSectionHeader;
	PIMAGE_FILE_HEADER		pImageFileHeader;
	PIMAGE_OPTIONAL_HEADER	pImageOperFileHeader;
};