#pragma once


// CValueEdit

class CValueEdit : public CEdit
{
	DECLARE_DYNAMIC(CValueEdit)

public:
	CValueEdit();
	virtual ~CValueEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};


