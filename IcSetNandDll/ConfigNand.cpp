// ConfigNand.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigNand.h"
#include "afxdialogex.h"


// CConfigNand dialog

IMPLEMENT_DYNAMIC(CConfigNand, CPropertyPage)

CConfigNand::CConfigNand()
	: CPropertyPage(CConfigNand::IDD)
{
	color = (COLORREF)RGB(255, 255, 255);
	back_brush = new CBrush(color);
	m_vNand.clear();
}

CConfigNand::~CConfigNand()
{
	
}

void CConfigNand::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_NAND, m_ctrList);
	DDX_Control(pDX, IDC_LIST_GUARDED, m_lcGuarded);
	DDX_Control(pDX, IDC_COMBO_GUARDED, m_cbGdArea);
}


BEGIN_MESSAGE_MAP(CConfigNand, CPropertyPage)
	ON_BN_CLICKED(IDOK, &CConfigNand::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CConfigNand::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_GUARDED, &CConfigNand::OnCbnSelchangeComboGuarded)
END_MESSAGE_MAP()


// CConfigNand message handlers


BOOL CConfigNand::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  Add extra initialization here
	/*GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);*/
	DWORD mStyle;
	mStyle = m_ctrList.GetExtendedStyle();
	mStyle = mStyle | LVS_EX_GRIDLINES |LVS_EX_FULLROWSELECT;
	m_ctrList.EnableToolTips(TRUE);
	m_ctrList.SetExtendedStyle(mStyle);
	m_ctrList.SetExCtrlResource(IDC_ROOT_INDEX, 50);
	m_ctrList.SetFont(this->GetFont());

	mStyle = m_lcGuarded.GetExtendedStyle();
	mStyle = mStyle | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	m_lcGuarded.EnableToolTips(TRUE);
	m_lcGuarded.SetExtendedStyle(mStyle);
	m_lcGuarded.SetExCtrlResource(IDC_ROOT_INDEX+50, 50);
	m_lcGuarded.SetFont(this->GetFont());

	InitGuardedList();
	m_cbGdArea.AddString(_T("0"));
	m_cbGdArea.AddString(_T("1"));
	m_cbGdArea.AddString(_T("2"));
	m_cbGdArea.AddString(_T("3"));
	m_cbGdArea.AddString(_T("4"));
	m_cbGdArea.SetCurSel(0);
	OnCbnSelchangeComboGuarded();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CConfigNand::UpdateUI(CString &sIniPath, CString &sComment)
{
	if (sIniPath.IsEmpty() || sComment.IsEmpty())
		return;
#ifdef _UNICODE
	if (_waccess(sIniPath, 00) != 0)
		return;
#else
	if (_access(sIniPath, 00) != 0)
		return;
#endif
	

}


void CConfigNand::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}


void CConfigNand::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
}


void CConfigNand::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);
	if (m_ctrList.m_hWnd)
	{
		CRect rc;
		int width;
		GetWindowRect(&rc);
		width = rc.Width() -30;

		m_ctrList.GetWindowRect(&rc);
		rc.right = rc.left + width;
		ScreenToClient(&rc);
		m_ctrList.MoveWindow(&rc);

		m_lcGuarded.GetWindowRect(&rc);
		rc.right = rc.left + width;
		ScreenToClient(&rc);
		m_lcGuarded.MoveWindow(&rc);
	}
	// TODO: Add your message handler code here
}


HBRUSH CConfigNand::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	switch (nCtlColor)
	{
	case CTLCOLOR_BTN:
	case CTLCOLOR_STATIC:
	{
							pDC->SetBkMode(TRANSPARENT);
							if(back_brush&&back_brush->m_hObject)
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


void CConfigNand::OnDestroy()
{
	CPropertyPage::OnDestroy();
	delete back_brush;
	m_vNand.clear();
	// TODO: Add your message handler code here
}


void CConfigNand::OnCbnSelchangeComboGuarded()
{
	// TODO: Add your control notification handler code here
	CString sTx;
	m_cbGdArea.GetLBText(m_cbGdArea.GetCurSel(), sTx);
	int nCount = _tstoi(sTx);
	AddGuardedItem(nCount);
}


#define GUARDEDAREA_INDEX 0
#define START_INDEX 1
#define END_INDEX 2
#define BADBLOCK_INDEX 3
void CConfigNand::AddGuardedItem(int nCount)
{
	CString sTx;
	m_lcGuarded.DeleteAllItems();
	for (int i = 0; i < nCount; i++)
	{
		sTx.Format(_T("%d"), i);
		m_lcGuarded.InsertItem(i, sTx);

		sTx.Format(_T("0"));
		m_lcGuarded.SetItemText(i, START_INDEX, sTx,TRUE);
		m_lcGuarded.SetItemText(i, END_INDEX, sTx, TRUE);
		m_lcGuarded.SetItemText(i, BADBLOCK_INDEX, sTx, TRUE);

	}
}
void CConfigNand::InitGuardedList()
{
	int col;
	struct {
		CString ColTx;
		ENUM_LIST_TYPE  ColType;
		unsigned short  ColLen;
	} ColHeader[] =
	{
		_T("Guarded Area Index"), LISTCTRL_EX_TEXT, 140,
		_T("Start"), LISTCTRL_EX_EDITBOX, 90,
		_T("End"), LISTCTRL_EX_EDITBOX, 90,
		_T("Bad Block Allowd"), LISTCTRL_EX_EDITBOX, 140,
	};
	for (col = 0; col <sizeof(ColHeader) / sizeof(ColHeader[0]); col++)
		m_lcGuarded.InsertColumn(col, ColHeader[col].ColTx, ColHeader[col].ColType, LVCFMT_LEFT, ColHeader[col].ColLen);
}
