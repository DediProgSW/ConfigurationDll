#pragma once
#include <assert.h>
#include <vector>
using std::vector;
//error codes
#define CONFIG_ERR
#define CONFIG_ERR_NO       0
#define CONFIG_ERR_NONE     1
#define CONFIG_ERR_PARAM    2
#define CONFIG_ERR_NOINIT   3



//emmc
struct emmc_config{
	unsigned short addr;
	unsigned short value;
};
//nand
struct nand_config
{
	wchar_t         name[30];           /* memory name */
	wchar_t         type[30];           /* NAND, NOR, MCU */
	long long       memory_addr;        /* memory start program address */
	long long       program_length;     /* program length */
	unsigned long   block_size;         /* block_size*/
	unsigned int    page_size;          /* page size in byte*/
	unsigned char   BBMType;		    //0= NoManagement
										//1= Skip Bad Block
										//2= RBA+
										//3= RBA-
										//other: RFU
	unsigned char BMPOS;                //bad block mark position
	unsigned short MaxErrBits;          //[3:0]: max error bit allowed
	                                    //[15:4] error bit reset size:
	                                    //size=256x[7:4]+[15:8]
	unsigned int RBAStartIndex;         //reserved area start block index
	unsigned int GuardAreaSet[12];

};
static struct _bbm_tbl{
	unsigned int index;
	CString BBmName; 
}BBM_TBL[]=
{
	0, _T("NoManagement"),
	1, _T("Skip Bad Block"),
//	2, _T("RBA+"),
//	3, _T("RBA-"),
};
typedef struct _bbm_set{
	unsigned char BBMType; //0= NoManagement
	                       //1= Skip Bad Block
	                       //2= RBA+
	                       //3= RBA-
	                       //other: RFU
	unsigned char BMPOS;   //bad block mark position
	unsigned short MaxErrBits;   //[3:0]: max error bit allowed
	                             //[15:4] error bit reset size:
	                             //size=256x[7:4]+[15:8]
	unsigned int RBAStartIndex;  //reserved area start block index
	unsigned int GuardAreaSet[12];//4 groups max, each group takes 3 parameters: start, end and max count of bad block allowed
	unsigned int BadBlockCount;
	
	unsigned int BadBlockList[];  //page index of bad block
}BBMSET;

