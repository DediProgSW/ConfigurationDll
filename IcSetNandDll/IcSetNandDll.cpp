// IcSetNandDll.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ConfigNand.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CConfigNand *pNandDlg = NULL;
DLL_API UINT Config_Create(DWORD dwStyle, CRect &rc, CWnd *pParentWnd, CString &sIniPath, CString &sComment)
{
	int err = 0;
	assert(!pNandDlg);
	if (!pNandDlg)
	{
		pNandDlg = new CConfigNand;
		pNandDlg->Create(IDD_DLG_NAND, pParentWnd);
		pNandDlg->SetParent(pParentWnd);
		pNandDlg->UpdateUI(sIniPath, sComment);
	}
	return err;
}

DLL_API UINT Config_Destory()
{
	int err = 0;
	assert(pNandDlg);
	pNandDlg->DestroyWindow();
	return err;
}

DLL_API UINT Config_Show(CRect &rc, int nCmdShow)
{
	int err = 0;

	assert(pNandDlg);
	assert(pNandDlg->m_hWnd);

	if (pNandDlg && pNandDlg->m_hWnd)
	{
		pNandDlg->MoveWindow(&rc);
		pNandDlg->ShowWindow(nCmdShow);
	}
	return err;
}


DLL_API UINT Config_InitNandList(vector<struct nand_config> &NandFileTb)
{
	int err = CONFIG_ERR_NO;
	//Init List Header
	if (!pNandDlg || !pNandDlg->m_hWnd || !pNandDlg->m_ctrList.m_hWnd)
	{
		err = CONFIG_ERR_NOINIT;
		return err;
	}

	//create Columns
	int col;
	struct {
		CString ColTx;
		ENUM_LIST_TYPE  ColType;
		unsigned short  ColLen;
	} ColHeader[] =
	{
		_T("Image Index"), LISTCTRL_EX_TEXT, 100,
		_T("Block Index"), LISTCTRL_EX_TEXT, 90,
		_T("Block Count"), LISTCTRL_EX_TEXT, 90,
		_T("BBM"), LISTCTRL_EX_COMBOBOX, 140,
		_T("EccUnitSize"), LISTCTRL_EX_EDITBOX, 90,
		_T("MaxErrorBit(0-15)"), LISTCTRL_EX_EDITBOX, 130,
	};
	for (col = 0; col <sizeof(ColHeader) / sizeof(ColHeader[0]); col++)
		pNandDlg->m_ctrList.InsertColumn(col, ColHeader[col].ColTx, ColHeader[col].ColType, LVCFMT_LEFT, ColHeader[col].ColLen);

	//Insert Item
	unsigned long nItem = NandFileTb.size();
	unsigned short nBbm = sizeof(BBM_TBL) / sizeof(BBM_TBL[0]);
	unsigned long i;
	unsigned short j;
	struct nand_config NandItem;
	if (!nItem || !nBbm)
	{
		err = CONFIG_ERR_PARAM;
		return err;
	}

	wchar_t sAddr[30], sLength[30];
	CString sIndex;
	CString sTemp;
	for (i = 0; i < nItem; i++)
	{
		memset(&NandItem, 0, sizeof(struct nand_config));
		NandItem = NandFileTb.at(i);
		pNandDlg->m_vNand.push_back(NandItem);
		sIndex.Format(_T("IMG%d"), i);
		pNandDlg->m_ctrList.InsertItem(i, sIndex);
		memset(sAddr, 0, sizeof(sAddr) / sizeof(sAddr[0]));
		memset(sLength, 0, sizeof(sLength) / sizeof(sLength[0]));
		swprintf_s(sAddr, _T("%ld"), NandItem.memory_addr);
		swprintf_s(sLength, _T("%ld"), NandItem.program_length);
		pNandDlg->m_ctrList.SetItemText(i, LIST_BLOCKINDEX_INDEX, sAddr);
		pNandDlg->m_ctrList.SetItemText(i, LIST_BLOCKCOUNT_INDEX, sLength);
		pNandDlg->m_ctrList.SetItemText(i, LIST_BBM_INDEX, BBM_TBL[1].BBmName, TRUE);
		for (j = 0; j < nBbm; j++)
			pNandDlg->m_ctrList.AddComboBoxItem(i, LIST_BBM_INDEX, BBM_TBL[j].BBmName);
		pNandDlg->m_ctrList.SetComboBoxSel(i, LIST_BBM_INDEX, BBM_TBL[1].index);
		sTemp.Format(_T("%d"), NandItem.page_size);
		pNandDlg->m_ctrList.SetItemText(i, LIST_ECCSIZE_INDEX, sTemp, TRUE);
		pNandDlg->m_ctrList.SetEditLimit(i, LIST_ECCSIZE_INDEX, NandItem.page_size);
		pNandDlg->m_ctrList.SetItemText(i, LIST_MAXERRBIT_INDEX, _T("0"), TRUE);
		pNandDlg->m_ctrList.SetEditLimit(i, LIST_MAXERRBIT_INDEX, 15);
	}
	return err;
}
DLL_API UINT Config_GetNandData(vector<struct nand_config> &NandFileTb)
{
	int err = CONFIG_ERR_NO;
	if (!pNandDlg || !pNandDlg->m_hWnd || !pNandDlg->m_ctrList.m_hWnd)
	{
		err = CONFIG_ERR_NOINIT;
		return err;
	}
	unsigned long nItem = pNandDlg->m_ctrList.GetItemCount();
	if (!nItem)
	{
		err = CONFIG_ERR_NONE;
		return err;
	}
	NandFileTb.clear();
	struct nand_config NandItem;
	unsigned long i;
	unsigned short ecc_size;
	unsigned short maxerr_bit;
	//Guarded Area
	unsigned int GdAreaCount;
	unsigned int GdStart = _tstoi(pNandDlg->m_lcGuarded.GetItemText(0, 1));
	unsigned int GdEnd = _tstoi(pNandDlg->m_lcGuarded.GetItemText(0, 2));
	unsigned int GdBBAllowed = _tstoi(pNandDlg->m_lcGuarded.GetItemText(0, 3));
	CString sValue;
	int j;
	for (i = 0; i <nItem; i++)
	{
		ecc_size = 0;
		maxerr_bit = 0;
		memset(&NandItem, 0, sizeof(struct nand_config));
		NandItem = pNandDlg->m_vNand.at(i);
		//BBM
		sValue = pNandDlg->m_ctrList.GetItemText(i, LIST_BBM_INDEX);
		for (j = 0; j < sizeof(BBM_TBL) / sizeof(BBM_TBL[0]); j++)
		{
			if (sValue == BBM_TBL[j].BBmName)
			{
				NandItem.BBMType = BBM_TBL[j].index;
				break;
			}
		}
		//Guard Area Set
		GdAreaCount = pNandDlg->m_lcGuarded.GetItemCount();
		for (j = 0; j < sizeof(NandItem.GuardAreaSet) / sizeof(NandItem.GuardAreaSet[0]); j++)
		{
			NandItem.GuardAreaSet[j] = 0xFFFFFFFF;
		}
		for (j = 0; j < GdAreaCount; j++)
		{
			NandItem.GuardAreaSet[j*3] = _tstoi(pNandDlg->m_lcGuarded.GetItemText(j, 1));
			NandItem.GuardAreaSet[j*3+1] = _tstoi(pNandDlg->m_lcGuarded.GetItemText(j, 2));
			NandItem.GuardAreaSet[j*3+2] = _tstoi(pNandDlg->m_lcGuarded.GetItemText(j, 3));
		}
		//ecc size
		sValue = pNandDlg->m_ctrList.GetItemText(i, LIST_ECCSIZE_INDEX);
		ecc_size = _tcstod(sValue, NULL);
		//max err bit
		sValue = pNandDlg->m_ctrList.GetItemText(i, LIST_MAXERRBIT_INDEX);
		maxerr_bit = _tcstod(sValue, NULL);
		//MaxErrBits
		int i;
		for (i = 0; (1 << i) != (ecc_size / 256); i++)
		{
			if (i == 16)
				return err;
		}		
		NandItem.MaxErrBits |= i << 4;
		NandItem.MaxErrBits |= (ecc_size % 256) << 8;
		NandItem.MaxErrBits |= maxerr_bit;
		NandFileTb.push_back(NandItem);
	}


	return err;
}