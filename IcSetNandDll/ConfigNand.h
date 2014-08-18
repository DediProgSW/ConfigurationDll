#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "afxcmn.h"
#include "ListCtrlEx.h"
#define DLL_API_EXPORT
#include "../common/ic_set_lib.h"

// CConfigNand dialog
#include <vector>

#define LIST_INDEX              
#define LIST_IMGINDEX_INDEX     0
#define LIST_BLOCKINDEX_INDEX   1
#define LIST_BLOCKCOUNT_INDEX   2
#define LIST_BBM_INDEX          3  
#define LIST_ECCSIZE_INDEX      4
#define LIST_MAXERRBIT_INDEX    5




class CConfigNand : public CPropertyPage
{
	DECLARE_DYNAMIC(CConfigNand)

public:
	CConfigNand();
	virtual ~CConfigNand();

// Dialog Data
	enum { IDD = IDD_DLG_NAND };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void AddGuardedItem(int nCount);
	void InitGuardedList();
public:
	virtual BOOL OnInitDialog();
	void UpdateUI(CString &sIniPath, CString &sComment);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CListCtrlEx m_ctrList;
	CBrush* back_brush;
	COLORREF color;
	std::vector<struct nand_config> m_vNand;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	CListCtrlEx m_lcGuarded;
	CComboBox m_cbGdArea;
	afx_msg void OnCbnSelchangeComboGuarded();
};
