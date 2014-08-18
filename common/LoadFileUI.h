#pragma once
#include <vector>
#include "../common/cmd_protocol.h"
#include "../common/client_const.h"

using namespace::std;


#ifdef LOADFILE_UI_DLL
#define LOADFILE_UI_API_C 	 extern "C" __declspec(dllexport)
#else
#define LOADFILE_UI_API_C 	 extern "C" __declspec(dllimport)
#endif

typedef UINT(*pNetBrowserDoMadal)(TCHAR FilePath[MAX_PATH],DWORD64 *FileLen,CString &LanguageDir,CString &language,CWnd *pParent);
LOADFILE_UI_API_C UINT LoadFileUIDoModal(
	CString &LanguagePath, 
	CString &ModuleName, 
	CTE_CHIP_TYPE ChipType,
	DWORD64 ChipSize,
	vector<CString> *pvSau, 
	vector<struct socket_partition_info> *pvPtnInfo, 
	vector<struct socket_file_info> *pvFileInfo, 
	pNetBrowserDoMadal pNetBrowserCallFunc,
	CWnd* pParent = NULL
	);

typedef UINT(*pLoadFileUIDoModal)(
	CString &LanguagePath,
	CString &ModuleName,
	CTE_CHIP_TYPE ChipType,
	DWORD64 ChipSize,
	vector<CString> *pvSau,
	vector<struct socket_partition_info> *pvPtnInfo,
	vector<struct socket_file_info> *pvFileInfo,
	pNetBrowserDoMadal pNetBrowserCallFunc,
	CWnd* pParent);

const struct _LOADFILE_UI_MODULENAME{
	CTE_CHIP_TYPE ChipType;
	CString DllName;
}LoadFileUiModule[] =
{
	CPTYPE_SPI_NOR, _T(".\\LoadFileUI_NORMAL.dll"),
	CPTYPE_PARALLEL_NOR, _T(".\\LoadFileUI_NORMAL.dll"),
	CPTYPE_SPI_NAND, _T(".\\LoadFileUI_NAND.dll"),
	CPTYPE_PARALLEL_NAND, _T(".\\LoadFileUI_NAND.dll"),
	CPTYPE_EEPROM, _T(".\\LoadFileUI_NORMAL.dll"),
	CPTYPE_EMMC, _T(".\\LoadFileUI_eMMC.dll"),
	CPTYPE_SD, _T(".\\LoadFileUI_eMMC.dll"),
	CPTYPE_MCU, _T(".\\LoadFileUI_MCU.dll"),
	CPTYPE_OTHERS, _T(".\\LoadFileUI_Others.dll"),
};
