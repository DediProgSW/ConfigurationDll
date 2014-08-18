// IcSetEmmcDLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#define DLL_API_EXPORT
#include "../common/ic_set_lib.h"
#include "ConfigEmmc.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CConfigEmmc *pEmmcDlg = NULL;
DLL_API UINT Config_Create(DWORD dwStyle, CRect &rc, CWnd *pParentWnd, CString &sIniPath, CString &sComment)
{
	int err = CONFIG_ERR_NO;
#ifdef _DEBUG
	assert(!pEmmcDlg);
#endif
	if (!pEmmcDlg)
	{
		pEmmcDlg = new CConfigEmmc;
		pEmmcDlg->Create(IDD_DLG_EMMC, pParentWnd);
		pEmmcDlg->SetParent(pParentWnd);
		pEmmcDlg->UpdateUI(sIniPath, sComment);
	}
	return err;
}

DLL_API UINT Config_Destory()
{
	int err = CONFIG_ERR_NO;

#ifdef _DEBUG
	assert(pEmmcDlg);
#endif
	pEmmcDlg->DestroyWindow();
	return err;
}

DLL_API UINT Config_Show(CRect &rc, int nCmdShow)
{
	int err = CONFIG_ERR_NO;

#ifdef _DEBUG
	assert(pEmmcDlg);
	assert(pEmmcDlg->m_hWnd);
#endif

	if (pEmmcDlg && pEmmcDlg->m_hWnd)
	{
		pEmmcDlg->MoveWindow(&rc);
		pEmmcDlg->ShowWindow(nCmdShow);
	}
	return err;
}

DLL_API UINT Config_GetEmmcData(vector<struct emmc_config> &ConfigVector)
{
	int err = CONFIG_ERR_NO;
	int nItem = pEmmcDlg->m_ctrList.GetItemCount();
	ConfigVector.clear();
	struct emmc_config sEmmcConfig;
	CString sAddr, sValue;
	int i;
	for (i = 0; i < nItem; i++)
	{
		sAddr = pEmmcDlg->m_ctrList.GetItemText(i, LIST_CTRL_ADDR_INDEX);
		sValue = pEmmcDlg->m_ctrList.GetItemText(i, LIST_CTRL_VALUE_INDEX);

#ifdef _UNICODE
		if (sAddr.Find(_T("X")) != -1 || sAddr.Find(_T("x")) != -1)
		{
			sEmmcConfig.addr = wcstoul((const wchar_t*)sAddr.GetBuffer(), NULL, 16);
		}
		else
		{
			sEmmcConfig.addr = wcstoul((const wchar_t*)sAddr.GetBuffer(), NULL, 10);
		}

		if (sValue.Find(_T("X")) != -1 || sValue.Find(_T("x")) != -1)
		{
			sEmmcConfig.value = wcstoul((const wchar_t*)sValue.GetBuffer(), NULL, 16);
		}
		else
		{
			sEmmcConfig.value = wcstoul((const wchar_t*)sValue.GetBuffer(), NULL, 10);
		}
#else
		if (sAddr.Find(_T("X")) || sAddr.Find(_T("x")))
		{
			param->extcsd_value[i].addr = strtoul((const char*)sAddr.GetBuffer(), NULL, 16);
			param->extcsd_value[i].vaule = strtoul((const char*)sValue.GetBuffer(), NULL, 16);
		}
		else
		{
			param->extcsd_value[i].addr = strtoul((const char*)sAddr.GetBuffer(), NULL, 10);
			param->extcsd_value[i].vaule = strtoul((const char*)sValue.GetBuffer(), NULL, 10);
		}
#endif
		ConfigVector.push_back(sEmmcConfig);
	}

	return err;
}