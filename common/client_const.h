#pragma once
/* This header only use for Dediprog programmer Client projects     *
*  created by peilin.huang 4/28/2014                                */

#define SIZE_OF_ARRAY(A) sizeof(A)/sizeof(A[0])
#define DEDI_TYPE typedef
#define DEDI_GL

/* Chip Info                                                        */
DEDI_TYPE enum _CTE_CHIP_TYPE{
	CPTYPE_SPI_NOR,
	CPTYPE_PARALLEL_NOR,
	CPTYPE_SPI_NAND,
	CPTYPE_PARALLEL_NAND,
	CPTYPE_EEPROM,
	CPTYPE_EMMC,
	CPTYPE_SD,
	CPTYPE_MCU,
	CPTYPE_OTHERS,
}CTE_CHIP_TYPE;

DEDI_TYPE struct _CTS_CHIPTYPE_DESCRIP{
	CTE_CHIP_TYPE eType;            //chip type
	CString       strDes;           //chip type description
	CString       strKeyWords;      //key words of chip type description
}CTS_CHIPTYPE_DESCRIP;



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

