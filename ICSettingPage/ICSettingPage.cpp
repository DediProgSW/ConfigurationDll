// CICSettingPage.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ICSettingPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CICSettingPage::CICSettingPage(
                                UINT nIDTemplate,
                                struct chip_info_c * ChipInfo,
                                unsigned char *RegisterBuff,
                                unsigned long BuffLen,
                                unsigned long *pDataLen,
                                CWnd* pParent /*=NULL*/)
                                : CMFCPropertyPage(nIDTemplate),
                                m_pChipInfo(ChipInfo),
                                m_pRegBuff(RegisterBuff),
                                m_nBuffLen(BuffLen),
                                m_pDataLen(pDataLen)
{
        ASSERT(ChipInfo);
        ASSERT(RegisterBuff);
        ASSERT(pDataLen);
}



void CICSettingPage::OnOK()
{
        // TODO: Add your specialized code here and/or call the base class
 
        OutputData(m_pChipInfo, m_pRegBuff, m_nBuffLen, m_pDataLen);

        CMFCPropertyPage::OnOK();
}


LRESULT CICSettingPage::OnWizardBack()
{
        // TODO: Add your specialized code here and/or call the base class

        OutputData(m_pChipInfo, m_pRegBuff, m_nBuffLen, m_pDataLen);
        return CMFCPropertyPage::OnWizardBack();
}


BOOL CICSettingPage::OnSetActive()
{
        // TODO: Add your specialized code here and/or call the base class
        ShowData(m_pChipInfo, m_pRegBuff, *m_pDataLen);
        return CMFCPropertyPage::OnSetActive();
}


LRESULT CICSettingPage::OnWizardNext()
{
        // TODO: Add your specialized code here and/or call the base class

        OutputData(m_pChipInfo, m_pRegBuff, m_nBuffLen, m_pDataLen);
        return CMFCPropertyPage::OnWizardNext();
}


BOOL CICSettingPage::OnInitDialog()
{
        CMFCPropertyPage::OnInitDialog();

        // TODO:  Add extra initialization here
        ShowData(m_pChipInfo, m_pRegBuff, *m_pDataLen);
        return TRUE;  // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}
