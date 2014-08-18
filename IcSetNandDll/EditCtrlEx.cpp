#include "stdafx.h"
#include "EditCtrlEx.h"

//class edit
IMPLEMENT_DYNAMIC(CEditCtrlEx, CEdit)

CEditCtrlEx::CEditCtrlEx()
{
	m_limit = 0;
	m_lastStr.Empty();
}

CEditCtrlEx::~CEditCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CEditCtrlEx, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()



// CValueEdit message handlers


void CEditCtrlEx::SetLimitInput(unsigned int uiLimit)
{
	m_limit = uiLimit;
}

void CEditCtrlEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == VK_BACK) {
		CEdit::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	if (nChar < '0' || nChar > '9')
		return;
	GetWindowText(m_lastStr);
	CEdit::OnChar(nChar, nRepCnt, nFlags);

	CString str;
	GetWindowText(str);
	unsigned int cur_value = 0;
	cur_value = _tcstod(str, NULL);
	

	if (m_limit > 0 && cur_value > m_limit)
	{
		str.Format(_T("Input value must be less thran %d !"), m_limit);
		MessageBox(str, _T("Warning!"), MB_ICONWARNING);
		SetWindowText(m_lastStr);
		SetFocus();
	}
	
	return;
}