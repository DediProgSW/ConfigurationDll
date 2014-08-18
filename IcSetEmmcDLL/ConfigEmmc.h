#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "ValueEdit.h"
#include "CGridListCtrlEx\CGridListCtrlGroups.h"
// CConfigEmmc dialog
#define	ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#define LIST_CTRL_INDEX
enum {
	LIST_CTRL_ADDR_INDEX,
	LIST_CTRL_VALUE_INDEX
};
class CConfigEmmc : public CPropertyPage
{
	DECLARE_DYNAMIC(CConfigEmmc)

public:
	CConfigEmmc();   // standard constructor
	virtual ~CConfigEmmc();

// Dialog Data
	enum { IDD = IDD_DLG_EMMC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
#ifdef USE_EXLIST
	CGridListCtrlEx m_ctrList;
#else
	CListCtrl m_ctrList;
#endif
	CValueEdit m_ctrEditAddr;
	CValueEdit m_ctrEditValue;
	CBrush* back_brush;
	COLORREF color;
	void InitListCtrl();
	afx_msg void OnBnClickedBtAdd();
	afx_msg void OnBnClickedBtDelete();
	afx_msg void OnBnClickedBtReset();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	void InitListHeader();
	virtual BOOL OnInitDialog();
	void UpdateUI(CString &sIniPath, CString &sComment);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton m_btAdd;
	CButton m_btDelete;
	CButton m_btReset;
	CStatic m_stAddr;
	CStatic m_stValue;
	CStatic m_stGroList;
	CStatic m_stGroPtnSet;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
};
