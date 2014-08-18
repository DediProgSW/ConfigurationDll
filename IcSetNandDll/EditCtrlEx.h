#pragma once
//class edit
class CEditCtrlEx : public CEdit
{
	DECLARE_DYNAMIC(CEditCtrlEx)

public:
	CEditCtrlEx();
	virtual ~CEditCtrlEx();

protected:
	DECLARE_MESSAGE_MAP()
	unsigned int m_limit;
	CString m_lastStr;
public:
	void SetLimitInput(unsigned int uiLimit = 0);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};