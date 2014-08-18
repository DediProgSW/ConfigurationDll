#pragma once


// CTestPage dialog
#include "resource.h"
#include "../ICSettingPage/ICSettingPage.h"

class CTestPage : public CICSettingPage
{
	DECLARE_DYNAMIC(CTestPage)

public:
	CTestPage(
                        struct chip_info_c * ChipInfo,
                        unsigned char *RegisterBuff,
                        unsigned long BuffLen,
                        unsigned long *pDataLen,
                        CWnd* pParent = NULL
                );
	virtual ~CTestPage();

protected:
        virtual BOOL ShowData(const struct chip_info_c *chip, const unsigned char *pRegbuff, unsigned long nBufferLen);
        virtual BOOL OutputData(const struct chip_info_c *chip, unsigned char *pRegBuff, unsigned long nBufferLen, unsigned long *pDataLen);


// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
        virtual BOOL OnInitDialog();
};
