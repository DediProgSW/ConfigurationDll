#pragma once
/* This header only use for Dediprog programmer Client projects     *
*  created by peilin.huang 4/28/2014                                */

#define SIZE_OF_ARRAY(A) sizeof(A)/sizeof(A[0])
#define DEDI_TYPE typedef
#define DEDI_GL


/* Engineer operation                                               */
DEDI_TYPE enum _CTE_ENG_OPTYPE{
	ENG_OPTYPE_ERASE,
	ENG_OPTYPE_BLANK,
	ENG_OPTYPE_PROGRAM,
	ENG_OPTYPE_VERIFY,
}CTE_ENG_OPTYPE;

DEDI_TYPE struct _CTS_ENG_OPTYPE_DES {
	CTE_ENG_OPTYPE eType;
	CString        strDes;
}CTS_ENG_OPTYPE_DES;


