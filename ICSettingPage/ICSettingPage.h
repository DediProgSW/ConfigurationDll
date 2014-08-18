#ifndef _ICSETTING_PAGE_H
#define _ICSETTING_PAGE_H

#include <afxpropertypage.h>

#ifdef ICSETTING_EXPORT
#define ICSETTING_API    __declspec(dllexport)
#else
#define ICSETTING_API    __declspec(dllimport)
#endif // _DLL_SAMPLE

class ICSETTING_API CICSettingPage: public CMFCPropertyPage
{
public:
        CICSettingPage(
                        UINT nIDTemplate,
                        struct chip_info_c * ChipInfo,
                        unsigned char *RegisterBuff,
                        unsigned long BuffLen,
                        unsigned long *pDataLen,
                        CWnd* pParent = NULL);
protected:
        struct chip_info_c * m_pChipInfo;
	unsigned char  *m_pRegBuff;
	unsigned long   m_nBuffLen;
	unsigned long  *m_pDataLen;
protected:
        virtual BOOL ShowData(const struct chip_info_c *chip, const unsigned char *pRegbuff, unsigned long nBufferLen) = 0;
        virtual BOOL OutputData(const struct chip_info_c *chip, unsigned char *pRegBuff, unsigned long nBufferLen, unsigned long *pDataLen) = 0;

public:
        virtual void OnOK();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        virtual LRESULT OnWizardNext();
        virtual BOOL OnInitDialog();
};


#endif // !_CICSETTING_PAGE_H
