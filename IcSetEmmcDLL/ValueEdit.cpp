// ValueEdit.cpp : implementation file
//

#include "stdafx.h"

#include "ValueEdit.h"

// CValueEdit

IMPLEMENT_DYNAMIC(CValueEdit, CEdit)

CValueEdit::CValueEdit()
{

}

CValueEdit::~CValueEdit()
{
}


BEGIN_MESSAGE_MAP(CValueEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CValueEdit message handlers




void CValueEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	int start, end;
	CString str;
	GetWindowText(str);
	
	if (nChar == VK_BACK) {
		CEdit::OnChar(nChar, nRepCnt, nFlags);
		return ;
	}

	switch (str.GetLength()) {
	case 0:
		if (nChar >= '0' && nChar <= '9') {
			CEdit::OnChar(nChar, nRepCnt, nFlags);
			return ;
		}
		break;
	case 1:
		if (nChar >= '0' && nChar <= '9' || _toupper(nChar) == 'X') {
			CEdit::OnChar(nChar, nRepCnt, nFlags);
			return ;
		}
		break;
	default:
		GetSel(start, end);
		if (str.GetAt(0) == '0' && _toupper(str.GetAt(1)) == 'X' && start == 1) {
			MessageBeep(MB_OK);
			return ;
		}
		if (str.GetAt(0) == '0' && _toupper(str.GetAt(1)) == 'X' &&
			_toupper(nChar) <= 'F' && _toupper(nChar) >= 'A') {
			CEdit::OnChar(nChar, nRepCnt, nFlags);
			return ;
		}
		if (nChar <= '9' && nChar >= '0') {
			CEdit::OnChar(nChar, nRepCnt, nFlags);
			return ;
		}
		break;
	}
	MessageBeep(MB_OK);
	return ;
}
