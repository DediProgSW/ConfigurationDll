#include "stdafx.h"
#include "ListCtrlEx.h"
//class list

CListCtrlEx::CListCtrlEx()
{
	ROOT_RESOURCE_INDEX = 0;
	NEXT_RESOURCE_INDEX = 0;
	m_bSetResource = false;
	m_vComboBox.clear();
	m_vCheckItem.clear();
	m_vEditBox.clear();
	m_vColType.clear();
}


CListCtrlEx::~CListCtrlEx()
{
}


HRESULT CListCtrlEx::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	// TODO: Add your specialized code here and/or call the base class

	return CListCtrl::accHitTest(xLeft, yTop, pvarChild);
}


HRESULT CListCtrlEx::accSelect(long flagsSelect, VARIANT varChild)
{
	// TODO: Add your specialized code here and/or call the base class

	return CListCtrl::accSelect(flagsSelect, varChild);
}





BOOL CListCtrlEx::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	int i;
	for (i = 0; i < m_vComboBox.size(); i++)
	{
		if (!m_vComboBox.at(i).pBoxItem)
			continue;
		m_vComboBox.at(i).pBoxItem->ShowWindow(SW_HIDE);
		m_vComboBox.at(i).pBoxItem->DestroyWindow();
		m_vComboBox.at(i).pBoxItem = NULL;
	}
	for (i = 0; i < m_vCheckItem.size(); i++)
	{
		if (!m_vCheckItem.at(i).pCheckBox)
			continue;
		m_vCheckItem.at(i).pCheckBox->ShowWindow(SW_HIDE);
		m_vCheckItem.at(i).pCheckBox->DestroyWindow();
		m_vCheckItem.at(i).pCheckBox = NULL;
	}
	for (i = 0; i < m_vEditBox.size(); i++)
	{
		if (!m_vEditBox.at(i).pEditBox)
			continue;
		m_vEditBox.at(i).pEditBox->ShowWindow(SW_HIDE);
		m_vEditBox.at(i).pEditBox->DestroyWindow();
		m_vEditBox.at(i).pEditBox = NULL;
	}
	m_vEditBox.clear();
	m_vComboBox.clear();
	m_vCheckItem.clear();
	m_vColType.clear();
	ROOT_RESOURCE_INDEX = 0;
	NEXT_RESOURCE_INDEX = 0;
	return CListCtrl::DestroyWindow();
}




BOOL CListCtrlEx::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// TODO: Add your specialized code here and/or call the base class

	return CListCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
BOOL CListCtrlEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (((NMHDR*)lParam)->code)
	{
	case HDN_BEGINTRACKW://ÍÏ¶¯
	case HDN_BEGINTRACKA:
	case HDN_DIVIDERDBLCLICKA://Ë«»÷
	case HDN_DIVIDERDBLCLICKW:
		*pResult = TRUE;
		return TRUE;
	}
	return CListCtrl::OnNotify(wParam, lParam, pResult);

}
BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	ON_NOTIFY_REFLECT(LVN_DELETEALLITEMS, &CListCtrlEx::OnLvnDeleteallitems)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &CListCtrlEx::OnLvnDeleteitem)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CListCtrlEx::OnLvnGetdispinfo)
	ON_NOTIFY_REFLECT(NM_KILLFOCUS, &CListCtrlEx::OnNMKillfocus)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


void CListCtrlEx::OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	int i;
	for (i = 0; i < m_vComboBox.size(); i++)
	{
		if (!m_vComboBox.at(i).pBoxItem)
			continue;
		m_vComboBox.at(i).pBoxItem->ShowWindow(SW_HIDE);
		m_vComboBox.at(i).pBoxItem->DestroyWindow();
		m_vComboBox.at(i).pBoxItem = NULL;
	}
	for (i = 0; i < m_vCheckItem.size(); i++)
	{
		if (!m_vCheckItem.at(i).pCheckBox)
			continue;
		m_vCheckItem.at(i).pCheckBox->ShowWindow(SW_HIDE);
		m_vCheckItem.at(i).pCheckBox->DestroyWindow();
		m_vCheckItem.at(i).pCheckBox = NULL;
	}
	for (i = 0; i < m_vEditBox.size(); i++)
	{
		if (!m_vEditBox.at(i).pEditBox)
			continue;
		m_vEditBox.at(i).pEditBox->ShowWindow(SW_HIDE);
		m_vEditBox.at(i).pEditBox->DestroyWindow();
		m_vEditBox.at(i).pEditBox = NULL;
	}
	m_vComboBox.clear();
	m_vCheckItem.clear();
	m_vEditBox.clear();
	NEXT_RESOURCE_INDEX = ROOT_RESOURCE_INDEX;
//	CListCtrl::DeleteAllItems();
	*pResult = 0;
}

BOOL CListCtrlEx::DeleteItem(int nItem)
{
	int i, nCount;
	CRect rcCell;
	nCount = m_vComboBox.size();
	for (i = 0; i < nCount; i++)
	{
		if (!m_vComboBox.at(i).pBoxItem)
			continue;
		if (m_vComboBox.at(i).nRow == nItem)
		{
			
			m_vComboBox.at(i).pBoxItem->ShowWindow(SW_HIDE);
			m_vComboBox.at(i).pBoxItem->DestroyWindow();
			m_vComboBox.at(i).pBoxItem = NULL;
			m_vComboBox.erase(m_vComboBox.begin() + i);
			i--;
			nCount--;
		}
	}
	for (i = 0; i < nCount; i++)
	{
		if (m_vComboBox.at(i).nRow > nItem)
		{
			m_vComboBox.at(i).nRow--;
			if (GetSubItemRect(m_vComboBox.at(i).nRow, m_vComboBox.at(i).nCol, LVIR_LABEL, rcCell))
			{
				m_vComboBox.at(i).pBoxItem->MoveWindow(rcCell);
				m_vComboBox.at(i).pBoxItem->SetItemHeight(-1, m_vComboBox.at(i).pBoxItem->GetItemHeight(-1) - 3);
			}

		}
	}
	nCount = m_vCheckItem.size();
	for (i = 0; i < nCount; i++)
	{
		if (!m_vCheckItem.at(i).pCheckBox)
			continue;
		if (m_vCheckItem.at(i).nRow == nItem)
		{
			m_vCheckItem.at(i).pCheckBox->ShowWindow(SW_HIDE);
			m_vCheckItem.at(i).pCheckBox->DestroyWindow();
			m_vCheckItem.at(i).pCheckBox = NULL;
			m_vCheckItem.erase(m_vCheckItem.begin() + i);
			i--;
			nCount--;
		}
	}
	for (i = 0; i < nCount ; i++)
	{
		if (m_vCheckItem.at(i).nRow > nItem)
		{
			m_vCheckItem.at(i).nRow--;
			if (GetSubItemRect(m_vCheckItem.at(i).nRow, m_vCheckItem.at(i).nCol, LVIR_LABEL, rcCell))
			{
				rcCell.right -= 4;
				m_vCheckItem.at(i).pCheckBox->MoveWindow(rcCell);
			}
		}
	}
	nCount = m_vEditBox.size();
	for (i = 0; i < nCount; i++)
	{
		if (!m_vEditBox.at(i).pEditBox)
			continue;
		if (m_vEditBox.at(i).nRow == nItem)
		{
			m_vEditBox.at(i).pEditBox->ShowWindow(SW_HIDE);
			m_vEditBox.at(i).pEditBox->DestroyWindow();
			m_vEditBox.at(i).pEditBox = NULL;
			m_vEditBox.erase(m_vEditBox.begin() + i);
			i--;
			nCount--;
		}
	}
	for (i = 0; i < nCount ; i++)
	{
		if (m_vEditBox.at(i).nRow > nItem)
		{
			m_vEditBox.at(i).nRow--;
			if (GetSubItemRect(m_vEditBox.at(i).nRow, m_vEditBox.at(i).nCol, LVIR_LABEL, rcCell))
			{
				rcCell.left += 1;
				rcCell.bottom -= 1;
				m_vEditBox.at(i).pEditBox->MoveWindow(rcCell);
			}
		}
	}
	return CListCtrl::DeleteItem(nItem);
}
void CListCtrlEx::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}


void CListCtrlEx::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


//void CListCtrlEx::OnLvnInsertitem(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//	// TODO: Add your control notification handler code here
//	*pResult = 0;
//}


void CListCtrlEx::OnNMKillfocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}


HBRUSH CListCtrlEx::CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/)
{
	// TODO:  Change any attributes of the DC here

	// TODO:  Return a non-NULL brush if the parent's handler should not be called
	return NULL;
}


ENUM_LIST_TYPE CListCtrlEx::GetColStyle(int nCol)
{
	unsigned int nColCnt = m_vColType.size();
	if (!nColCnt)
		return LISTCTRL_EX_TEXT;
	for (int i = 0; i < nColCnt; i++)
	{
		if (m_vColType.at(i).nCol == nCol)
			return m_vColType.at(i).Type;
	}
	return LISTCTRL_EX_TEXT;
}
unsigned int CListCtrlEx::GetNextResIndex()
{
	if (NEXT_RESOURCE_INDEX < m_MaxResource + ROOT_RESOURCE_INDEX )
		return NEXT_RESOURCE_INDEX>ROOT_RESOURCE_INDEX ?( NEXT_RESOURCE_INDEX + 1) : ROOT_RESOURCE_INDEX;
	return 0;
}


BOOL CListCtrlEx::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText, BOOL bShowEx)
{
	if (!bShowEx)
		return CListCtrl::SetItemText(nItem, nSubItem, lpszText);

	unsigned int nResourceId = GetNextResIndex();
	DWORD dwStyle;
	CRect rcCell,rcCtrl;
	COMBOBOX_ITEM ExCtrItemComBox;
	CHECKBOX_ITEM ExCtrItemCheckBox;
	EDITBOX_ITEM  ExCtrItemEditBox;
	//no more resource
	if (!nResourceId)
		return FALSE;

	switch (GetColStyle(nSubItem))
	{
	case LISTCTRL_EX_TEXT:
		break;
	case LISTCTRL_EX_COMBOBOX:
		ExCtrItemComBox.pBoxItem = new CComboBox;
		if (!ExCtrItemComBox.pBoxItem)
			return FALSE;
		//get cell rect
		ScreenToClient(&rcCell);
		if (!GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rcCell))
			return FALSE;
		dwStyle = WS_CHILD|WS_VSCROLL | WS_HSCROLL | CBS_DROPDOWNLIST  ;
		if (!ExCtrItemComBox.pBoxItem->Create(dwStyle, rcCell, this, nResourceId))
			return FALSE;
		ExCtrItemComBox.pBoxItem->SetFont(this->GetFont());
		//fit the cell
		ExCtrItemComBox.pBoxItem->GetWindowRect(&rcCtrl);
		ScreenToClient(&rcCtrl);
		if (rcCtrl.Height() < rcCell.Height())
		{
			// Expand to fit cell
			int padding = rcCell.Height() - rcCtrl.Height();
			if (padding > 0)
				ExCtrItemComBox.pBoxItem->SetItemHeight(-1, ExCtrItemComBox.pBoxItem->GetItemHeight(-1) + (UINT)padding);
		}
		else
		if (rcCtrl.Height() > rcCell.Height() + ::GetSystemMetrics(SM_CXBORDER))
		{
			// Compress to fit cell
			int margin = rcCtrl.Height() - ExCtrItemComBox.pBoxItem->GetItemHeight(-1);
			int padding = margin - 2 * ::GetSystemMetrics(SM_CYEDGE);
			/*if ((ExCtrItem.pBoxItem->GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
				padding -= ::GetSystemMetrics(SM_CYEDGE);*/
			if (padding > 0)
				ExCtrItemComBox.pBoxItem->SetItemHeight(-1, ExCtrItemComBox.pBoxItem->GetItemHeight(-1) - (UINT)padding - 1);
		}

		ExCtrItemComBox.pBoxItem->ShowWindow(SW_SHOW);
		ExCtrItemComBox.nCol = nSubItem;
		ExCtrItemComBox.nResId = nResourceId;
		ExCtrItemComBox.nRow = nItem;
		m_vComboBox.push_back(ExCtrItemComBox);
		break;
	case LISTCTRL_EX_CHECKBOX:
		ExCtrItemCheckBox.pCheckBox = new CButton;
		if (!ExCtrItemCheckBox.pCheckBox)
			return FALSE;
		//get cell rect
		ScreenToClient(&rcCell);
		if (!GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rcCell))
			return FALSE;
		dwStyle = BS_AUTOCHECKBOX | BS_LEFTTEXT | BS_CENTER;

		if (!ExCtrItemCheckBox.pCheckBox->Create(lpszText, dwStyle, rcCell, this, nResourceId))
			return FALSE;
		ExCtrItemCheckBox.pCheckBox->SetFont(this->GetFont());
		//fit the cell
		ExCtrItemCheckBox.pCheckBox->GetWindowRect(&rcCtrl);
		ScreenToClient(&rcCtrl);
		rcCtrl.right -= 4;
		ExCtrItemCheckBox.pCheckBox->MoveWindow(&rcCtrl);
		ExCtrItemCheckBox.pCheckBox->ShowWindow(SW_SHOW);
		ExCtrItemCheckBox.nCol = nSubItem;
		ExCtrItemCheckBox.nResId = nResourceId;
		ExCtrItemCheckBox.nRow = nItem;
		m_vCheckItem.push_back(ExCtrItemCheckBox);
		break;
	case LISTCTRL_EX_ICON:
		break;
	case LISTCTRL_EX_EDITBOX:
		ExCtrItemEditBox.pEditBox = new CEditCtrlEx;
		if (!ExCtrItemEditBox.pEditBox)
			return FALSE;
		//get cell rect
		ScreenToClient(&rcCell);
		if (!GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rcCell))
			return FALSE;
		dwStyle = ES_LEFT ;
		rcCell.left += 1;
		rcCell.bottom -= 1;
		if (!ExCtrItemEditBox.pEditBox->Create(dwStyle, rcCell, this, nResourceId))
			return FALSE;
		ExCtrItemEditBox.pEditBox->SetFont(this->GetFont());
		//fit the cell
		ExCtrItemEditBox.pEditBox->GetWindowRect(&rcCtrl);
		ScreenToClient(&rcCtrl);
		ExCtrItemEditBox.pEditBox->MoveWindow(&rcCtrl);
		ExCtrItemEditBox.pEditBox->ShowWindow(SW_SHOW);
		ExCtrItemEditBox.nCol = nSubItem;
		ExCtrItemEditBox.nResId = nResourceId;
		ExCtrItemEditBox.nRow = nItem;
		ExCtrItemEditBox.pEditBox->SetWindowText(lpszText);
		m_vEditBox.push_back(ExCtrItemEditBox);
		break;
	default:
		break;
	}
	return TRUE;
}
BOOL CListCtrlEx::SetComboSel(int nItem, int nSubItem, LPCTSTR lpszText)
{
	switch (GetColStyle(nSubItem))
	{
	case LISTCTRL_EX_COMBOBOX:
		for (int i = 0; i < m_vComboBox.size(); i++)
		{
			if (m_vComboBox.at(i).nRow == nItem &&m_vComboBox.at(i).nCol == nSubItem)
			{
				CString stTx;
				for (int j = 0; j < m_vComboBox.at(i).pBoxItem->GetCount(); j++)
				{
					m_vComboBox.at(i).pBoxItem->GetLBText(j, stTx);
					if (stTx == lpszText)
					{
						m_vComboBox.at(i).pBoxItem->SetCurSel(j);
						break;
					}
				}
//				m_vComboBox.at(i).pBoxItem->SetWindowText(lpszText);
				break;
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}
BOOL CListCtrlEx::SetCheckBox(int nItem, int nSubItem, BOOL bCheck)
{ 
	CHECKBOX_ITEM ExCtrItemCheckBox;
	switch (GetColStyle(nSubItem))
	{
	case LISTCTRL_EX_CHECKBOX:
		for (int i = 0; i < m_vCheckItem.size(); i++)
		{
			if (m_vCheckItem.at(i).nRow == nItem && m_vCheckItem.at(i).nCol == nSubItem)
			{
				m_vCheckItem.at(i).pCheckBox->SetCheck(bCheck);
				break;
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}
BOOL CListCtrlEx::SetEditBox(int nItem, int nSubItem, LPCTSTR lpszText)
{
	switch (GetColStyle(nSubItem))
	{
	case LISTCTRL_EX_EDITBOX:
		for (int i = 0; i < m_vEditBox.size(); i++)
		{
			if (m_vEditBox.at(i).nRow == nItem && m_vEditBox.at(i).nCol == nSubItem)
			{
				m_vEditBox.at(i).pEditBox->SetWindowText(lpszText);
				break;
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}
void CListCtrlEx::SetEditLimit(int nItem, int nSubItem,unsigned int uiLimit)
{
	switch (GetColStyle(nSubItem))
	{
	case LISTCTRL_EX_EDITBOX:
		for (int i = 0; i < m_vEditBox.size(); i++)
		{
			if (m_vEditBox.at(i).nRow == nItem && m_vEditBox.at(i).nCol == nSubItem)
			{
				m_vEditBox.at(i).pEditBox->SetLimitInput(uiLimit);
				break;
			}
		}
		break;
	default:
		break;
	}
}
CString CListCtrlEx::GetItemText(int nItem, int nCol)
{
	int i;
	CString sTx = _T("");
	switch (GetColStyle(nCol))
	{
	case LISTCTRL_EX_TEXT:
		sTx = CListCtrl::GetItemText(nItem, nCol);
		break;
	case LISTCTRL_EX_COMBOBOX:
		for (i = 0; i < m_vComboBox.size(); i++)
		{
			if (m_vComboBox.at(i).nCol == nCol && m_vComboBox.at(i).nRow == nItem && m_vComboBox.at(i).pBoxItem)
			{
				if (m_vComboBox.at(i).pBoxItem->GetCurSel() >= 0)
					m_vComboBox.at(i).pBoxItem->GetLBText(m_vComboBox.at(i).pBoxItem->GetCurSel(), sTx);
				break;
			}
		}
		break;
	case LISTCTRL_EX_CHECKBOX:
		for (i = 0; i < m_vCheckItem.size(); i++)
		{
			if (m_vCheckItem.at(i).nCol == nCol && m_vCheckItem.at(i).nRow == nItem && m_vCheckItem.at(i).pCheckBox)
			{
				sTx.Format(_T("%d"),m_vCheckItem.at(i).pCheckBox->GetCheck());
				break;
			}
		}
		break;
	case LISTCTRL_EX_ICON:
		break;
	case LISTCTRL_EX_EDITBOX:
		for (i = 0; i < m_vEditBox.size(); i++)
		{
			if (m_vEditBox.at(i).nCol == nCol && m_vEditBox.at(i).nRow == nItem && m_vEditBox.at(i).pEditBox)
			{
				m_vEditBox.at(i).pEditBox->GetWindowTextW(sTx);
				break;
			}
		}
		break;
	default:
		break;
	}
	return sTx;
}
int CListCtrlEx::InsertColumn(int nCol, LPCTSTR lpszColumnHeading,ENUM_LIST_TYPE eType, int nFormat , int nWidth , int nSubItem)
{
	int ret = CListCtrl::InsertColumn(nCol,lpszColumnHeading,nFormat,nWidth,nSubItem);
	if (ret != -1)
	{
		COL_TYPE col_type;
		col_type.nCol = nCol;
		col_type.Type = eType;
		m_vColType.push_back(col_type);
	}
	return ret;
}
void CListCtrlEx::AddComboBoxItem(int nRow, int nCol, const CString& strItemText)
{
	for (int i = 0; i < m_vComboBox.size(); i++)
	{
		if (m_vComboBox.at(i).nCol == nCol &&m_vComboBox.at(i).nRow == nRow)
		{
			if (m_vComboBox.at(i).pBoxItem)
			{
				m_vComboBox.at(i).pBoxItem->AddString(strItemText);
				m_vComboBox.at(i).pBoxItem->SetCurSel(0);
				break;
			}
		}
	}
}
void CListCtrlEx::SetComboBoxSel(int nRow, int nCol, int nCur)
{
	for (int i = 0; i < m_vComboBox.size(); i++)
	{
		if (m_vComboBox.at(i).nCol == nCol &&m_vComboBox.at(i).nRow == nRow)
		{
			if (m_vComboBox.at(i).pBoxItem)
			{
				m_vComboBox.at(i).pBoxItem->SetCurSel(nCur);
				break;
			}
		}
	}
}
void CListCtrlEx::SetExCtrlResource(unsigned int RootIndex, unsigned int nMaxCount)
{
	if (m_bSetResource)
		return;
	ASSERT(nMaxCount);
	ROOT_RESOURCE_INDEX = RootIndex;
	NEXT_RESOURCE_INDEX = ROOT_RESOURCE_INDEX;
	m_MaxResource = nMaxCount;
	double margin = 1.4;
	LOGFONT lf = { 0 };
	CFont m_CellFont, m_GridFont;
	VERIFY(GetFont()->GetLogFont(&lf) != 0);
	if (static_cast<HFONT>(m_CellFont))
		VERIFY(m_CellFont.DeleteObject());
	VERIFY(m_CellFont.CreateFontIndirect(&lf));

	lf.lfHeight = (int)(lf.lfHeight * margin);
	lf.lfWidth = (int)(lf.lfWidth * margin);
//	wcscpy(lf.lfFaceName, _T("ËÎÌå"));
	if (static_cast<HFONT>(m_GridFont))
		VERIFY(m_GridFont.DeleteObject());
	VERIFY(m_GridFont.CreateFontIndirect(&lf));
	CListCtrl::SetFont(&m_GridFont);
	m_bSetResource = true;
	GetHeaderCtrl()->SetFont(&m_CellFont);
}



