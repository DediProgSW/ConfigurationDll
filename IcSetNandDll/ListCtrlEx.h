#pragma once
#include "afxcmn.h"
#include "EditCtrlEx.h"
#include <vector>

typedef struct _COMBOBOX_ITEM{
	CComboBox     *pBoxItem;
	unsigned int   nResId;
	unsigned int   nRow;
	unsigned int   nCol;
}COMBOBOX_ITEM, *PCOMBOBOX_ITEM;

typedef struct _CHECKBOX_ITEM{
	CButton *pCheckBox;
	unsigned int   nResId;
	unsigned int   nRow;
	unsigned int   nCol;
}CHECKBOX_ITEM, *PCHECKBOX_ITEM;

typedef struct _EDITBOX_ITEM{
	CEditCtrlEx *pEditBox;
	unsigned int nResId;
	unsigned int nRow;
	unsigned int nCol;
}EDITBOX_ITEM,*PEDITBOX_ITEM;

typedef enum _LIST_CTRL_TYPE{
	LISTCTRL_EX_TEXT,
	LISTCTRL_EX_COMBOBOX,
	LISTCTRL_EX_CHECKBOX,
	LISTCTRL_EX_EDITBOX,
	LISTCTRL_EX_ICON,
}ENUM_LIST_TYPE;

typedef struct _COL_TYPE
{
	unsigned int      nCol;
	ENUM_LIST_TYPE    Type;
}COL_TYPE, *PCOL_TYPE;


//class list
class CListCtrlEx :
	public CListCtrl
{
public:
	CListCtrlEx();
	~CListCtrlEx();
protected:
	std::vector<COMBOBOX_ITEM> m_vComboBox;
	std::vector<CHECKBOX_ITEM> m_vCheckItem;
	std::vector<EDITBOX_ITEM>  m_vEditBox;
	std::vector<COL_TYPE> m_vColType;
	unsigned int ROOT_RESOURCE_INDEX;
	unsigned int NEXT_RESOURCE_INDEX;
	unsigned int m_MaxResource;

	bool m_bSetResource;
	
	//signal cell funcs
	ENUM_LIST_TYPE GetColStyle(int nCol);
	unsigned int GetNextResIndex();
protected:
public:
	virtual BOOL SetItemText(int nItem,int nSubItem,LPCTSTR lpszText,BOOL bShowEx = FALSE);
	BOOL SetComboSel(int nItem, int nSubItem, LPCTSTR lpszText);
	BOOL SetCheckBox(int nItem, int nSubItem, BOOL bCheck);
	BOOL SetEditBox(int nItem, int nSubItem, LPCTSTR lpszText);
	void SetEditLimit(int nItem, int nSubItem, unsigned int uiLimit);
	virtual BOOL DeleteItem(int nItem);
	virtual CString GetItemText(int nItem, int nCol);
	virtual int InsertColumn(int nCol, LPCTSTR lpszColumnHeading,ENUM_LIST_TYPE eType = LISTCTRL_EX_TEXT, int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);
	void AddComboBoxItem(int nRow, int nCol, const CString& strItemText);
	void SetComboBoxSel(int nRow, int nCol, int nCur);
	void SetExCtrlResource(unsigned int RootIndex, unsigned int nMaxCount = 1);
public:
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);
	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMKillfocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};

