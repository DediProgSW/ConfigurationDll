// TestPage.cpp : implementation file
//

#include "stdafx.h"
#include "TestPage.h"
#include "afxdialogex.h"
#include "../ICSettingPage/ICSettingPage.h"

// CTestPage dialog

IMPLEMENT_DYNAMIC(CTestPage, CICSettingPage)
CTestPage::CTestPage(
                        struct chip_info_c * ChipInfo,
                        unsigned char *RegisterBuff,
                        unsigned long BuffLen,
                        unsigned long *pDataLen,
                        CWnd* pParent
                ) : CICSettingPage(CTestPage::IDD,
                        ChipInfo,
                        RegisterBuff,
                        BuffLen,
                        pDataLen,
                        pParent
                        )

{

}


CTestPage::~CTestPage()
{
}

void CTestPage::DoDataExchange(CDataExchange* pDX)
{
	CICSettingPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTestPage, CICSettingPage)
END_MESSAGE_MAP()


// CTestPage message handlers
BOOL CTestPage::ShowData(const struct chip_info_c *chip, const unsigned char *pRegbuff, unsigned long nBufferLen)
{
        return TRUE;
}
BOOL CTestPage::OutputData(const struct chip_info_c *chip, unsigned char *pRegBuff, unsigned long nBufferLen, unsigned long *pDataLen)
{
        return TRUE;
}

BOOL CTestPage::OnInitDialog()
{
        CICSettingPage::OnInitDialog();

        // TODO:  Add extra initialization here

        return TRUE;  // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}
