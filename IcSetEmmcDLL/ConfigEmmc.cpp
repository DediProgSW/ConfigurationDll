// ConfigEmmc.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigEmmc.h"
#include "afxdialogex.h"
#include <assert.h>
#include <io.h>

//GridListCtrlEx..............

#include "CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridColumnTraitHyperLink.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"
#include "CGridListCtrlEx\ViewConfigSection.h"

// CConfigEmmc dialog

IMPLEMENT_DYNAMIC(CConfigEmmc, CPropertyPage)

CConfigEmmc::CConfigEmmc(): CPropertyPage(CConfigEmmc::IDD)
{
	color = (COLORREF)RGB(255, 255, 255);
	back_brush = new CBrush(color);
}

CConfigEmmc::~CConfigEmmc()
{
	
}

void CConfigEmmc::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EMMC, m_ctrList);
	DDX_Control(pDX, IDC_EDIT_ADDR, m_ctrEditAddr);
	DDX_Control(pDX, IDC_EDIT_VALUE, m_ctrEditValue);
	DDX_Control(pDX, IDC_BT_ADD, m_btAdd);
	DDX_Control(pDX, IDC_BT_DELETE, m_btDelete);
	DDX_Control(pDX, IDC_BT_RESET, m_btReset);
	DDX_Control(pDX, IDC_STATIC_ADDR, m_stAddr);
	DDX_Control(pDX, IDC_STATIC_VALUE, m_stValue);
	DDX_Control(pDX, IDC_GROUP_LIST, m_stGroList);
	DDX_Control(pDX, IDC_GROUP_PTNSET, m_stGroPtnSet);
}


BEGIN_MESSAGE_MAP(CConfigEmmc, CPropertyPage)
	ON_BN_CLICKED(IDC_BT_ADD, &CConfigEmmc::OnBnClickedBtAdd)
	ON_BN_CLICKED(IDC_BT_DELETE, &CConfigEmmc::OnBnClickedBtDelete)
	ON_BN_CLICKED(IDC_BT_RESET, &CConfigEmmc::OnBnClickedBtReset)
	ON_BN_CLICKED(IDOK, &CConfigEmmc::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CConfigEmmc::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CConfigEmmc message handlers



void CConfigEmmc::OnBnClickedBtAdd()
{
	// TODO: Add your control notification handler code here
	CString sAddr, sValue;
	m_ctrEditAddr.GetWindowText(sAddr);
	m_ctrEditValue.GetWindowText(sValue);
	if (sAddr.IsEmpty())
	{
		AfxMessageBox(_T("Please enter Address!"));
		m_ctrEditAddr.SetFocus();
		return;
	}
	if (sValue.IsEmpty())
	{
		AfxMessageBox(_T("Please enter Address!"));
		m_ctrEditValue.SetFocus();
		return;
	}
	unsigned long nItem = m_ctrList.GetItemCount(); 
	m_ctrList.InsertItem(nItem, sAddr);
	m_ctrList.SetItemText(nItem, LIST_CTRL_VALUE_INDEX, sValue);
}


void CConfigEmmc::OnBnClickedBtDelete()
{
	// TODO: Add your control notification handler code here
	int nSel = m_ctrList.GetSelectionMark();
	m_ctrList.DeleteItem(nSel);
}


void CConfigEmmc::OnBnClickedBtReset()
{
	// TODO: Add your control notification handler code here
	m_ctrList.DeleteAllItems();
}


void CConfigEmmc::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}


void CConfigEmmc::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
}


BOOL CConfigEmmc::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	
	// TODO:  Add extra initialization here
	if (GetDlgItem(IDOK)->m_hWnd)
	{
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	}
	InitListCtrl();
	m_ctrEditAddr.SetWindowText(_T("0x"));
	m_ctrEditValue.SetWindowText(_T("0x"));
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CConfigEmmc::InitListCtrl()
{

	const enum ColType {
		CGridColumn_Trait,
		CGridColumn_Trait_Text,
		CGridColumn_Trait_DateTime,
		CGridColumn_Trait_Edit,
		CGridColumn_Trait_CheckBox,
		CGridColumn_Trait_HyperLink,
		CGridColumn_Trait_Image,
		CGridColumn_Trait_Combo,
		CGridColumn_Trait_Visitor
	};
	struct {
		CString ColTx;
		enum ColType  ColType;
		unsigned short  ColLen;
	} ColHeader[] =
	{
		_T("Address"), CGridColumn_Trait_Text, 100,
		_T("Value"), CGridColumn_Trait_Text, 100,
	};

	//set ExStyle
	DWORD mStyle;
	mStyle = m_ctrList.GetExtendedStyle();
	mStyle = mStyle | LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT;//| LVS_EX_CHECKBOXES;
	m_ctrList.EnableToolTips(TRUE);
	m_ctrList.SetExtendedStyle(mStyle);

	int col;
	for (col = 0; col < ARRAY_SIZE(ColHeader); col++)
	{
		m_ctrList.InsertColumn(col, ColHeader[col].ColTx, LVCFMT_LEFT, ColHeader[col].ColLen, col);
	}
}



void CConfigEmmc::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	/*if (m_ctrList.m_hWnd)
	{
		CRect rc,rcList;
		GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_ctrList.GetWindowRect(&rcList);
		m_ctrList.ScreenToClient(&rcList);
		rcList.right = rc.right;
		m_ctrList.MoveWindow(&rcList);
	}*/
	// TODO: Add your message handler code here
}
void CConfigEmmc::UpdateUI(CString &sIniPath, CString &sComment)
{
	if (sIniPath.IsEmpty() || sComment.IsEmpty())
		return;
#ifdef _UNICODE
	if(_waccess(sIniPath,00) != 0)
		return;
#else
	if (_access(sIniPath, 00) != 0)
		return;
#endif
	const struct {
		unsigned int ID;
		CButton *pBt;
	}bt_language_tb[]=
	{
		IDC_BT_ADD,&m_btAdd,
		IDC_BT_DELETE,&m_btDelete,
		IDC_BT_RESET,&m_btReset,
	};
	const struct {
		unsigned int ID;
		CStatic *pSt;
	}st_language_tb[] =
	{
		IDC_STATIC_ADDR,&m_stAddr,
		IDC_STATIC_VALUE,&m_stValue,
		IDC_GROUP_LIST,&m_stGroList,
		IDC_GROUP_PTNSET,&m_stGroPtnSet,
	};

	int i;
	CString sKeyName;
#ifdef _UNICODE
	WCHAR temp[64];
#else
	CHAR temp[64];
#endif
	for (i = 0; i < sizeof(bt_language_tb) / sizeof(bt_language_tb[0]); i++)
	{
		sKeyName.Format(_T("%d"), bt_language_tb[i].ID);
		memset(temp, 0, sizeof(temp[0]) * 64);
		if (GetPrivateProfileString(sComment, sKeyName, NULL, temp, 64, sIniPath))
		{
			bt_language_tb[i].pBt->SetWindowText(temp);
		}
	}
	for (i = 0; i < sizeof(st_language_tb) / sizeof(st_language_tb[0]); i++)
	{
		sKeyName.Format(_T("%d"), st_language_tb[i].ID);
		memset(temp, 0, sizeof(temp[0]) * 64);
		if (GetPrivateProfileString(sComment, sKeyName, NULL, temp, 64, sIniPath))
		{
			st_language_tb[i].pSt->SetWindowText(temp);
		}
	}

}


HBRUSH CConfigEmmc::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_BTN:
	case CTLCOLOR_STATIC:
	{
							pDC->SetBkMode(TRANSPARENT);
							if (back_brush&&back_brush->m_hObject)
								return (HBRUSH)(back_brush->m_hObject);
	}
		break;
	case CTLCOLOR_DLG:
	{
						
						 if (back_brush&&back_brush->m_hObject)
							return (HBRUSH)(back_brush->m_hObject);
	}
		break;
	default:
		break;
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CConfigEmmc::OnDestroy()
{
	CPropertyPage::OnDestroy();
	delete back_brush;
	// TODO: Add your message handler code here
}
