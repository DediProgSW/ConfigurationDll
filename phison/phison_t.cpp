// phison_t.cpp : Defines the exported functions for the DLL application.
//

// operation.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <tchar.h>
#include <time.h>
#include <assert.h>
#include <Windows.h>
#include "iphlpapi.h"
#include "../common/prog_info.h"
#include "zstring.h"
#include "../Log/log.h"
#include "../common/assist.h"
#include "../path/path.h"

#pragma pack(push, 1)
struct uid {
        unsigned char   index;
        unsigned char   mac;
        unsigned short  dhm;
        unsigned char   my;
};
        
#pragma pack(pop)


#define __OLD_PATH__    0

#define TOUT 6000

static int uid_index = 0;

// PhisonInit.cpp : Defines the exported functions for the DLL application.
//

typedef int(register_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          reg_addr,
        unsigned short          *data,
        const void              *param
        );

typedef int(sram_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned short          address,
        unsigned int           *data,
        unsigned int            data_size,
        const void              *param
        );

typedef int(fifo_func_t) (
        int                     read_or_write,      /* 0 -> read,  1 -> write*/
        unsigned char           *cvpData,
        unsigned int            uiLength,
        const void              *param
        );




#define Read 0
#define Write 1

#define Err_OutOfSize -1
#define Err_Format -2
#define Err_CtrlRST1 -3
#define Err_CtrlRST2 -4
#define Err_CheckFlash -5
#define Err_LoadCode -6
#define Err_PreFormat -7
#define Err_RunPRAM -8
#define Err_ProgISP -9
#define Err_VerifyISP -10
#define Err_ProgHW -11
#define Err_ProgWP -12
#define Err_VerifyHW -13
#define Err_VerifyWP -14
#define Err_USB -100
#define Err_FIFO -101

#define Err_NoFile -200
#define Err_FileFMT -201

#include "stdafx.h"
#include <vector>

const TCHAR *FileA = _T("PhisonFileA.bin");
const TCHAR *FileB = _T("PhisonFileB.bin");
const TCHAR *FileC = _T("PhisonFileC.bin");
const TCHAR *HW = _T("PhisonHW.bin");
const TCHAR *WP = _T("PhisonWP.bin");
const TCHAR *FMT =_T("PhisonPreFMT.bin");


//#define PS7000

#ifdef PS7000
#define    Manufacture_A_Bin_Size   512*80   //40KB
#define    Firmware_B_Bin_Size      512*241  //120.5KB

#else

#define    Manufacture_A_Bin_Size   512*161   
#define    Firmware_B_Bin_Size      512*417 

#endif

#define	   CLKSET	0x8C





static int fifo_wr(
	unsigned int len,
	unsigned char *buf,
	register_func_t         *reg_func,
	fifo_func_t             *fifo_func,
	const void              *func_param
	)
{
	unsigned short fifolen;
	unsigned int tout;
	unsigned int remainLen;
	unsigned int i;

	unsigned int translen;
	remainLen = len;
 


	while (remainLen > 0)
	{
                CACL_TAKES_TIME_BEGIN(fifowr);
		do
		{
			reg_func(Read, 0x35, &fifolen, func_param);
			fifolen &= 0xFFF;			
                        tout = CACL_TAKES_TIME_END(fifowr);
		} while ((fifolen !=0) && (tout < 200));

		if (tout == 200)
			return Err_FIFO;
		
		translen = (remainLen < 4096) ? remainLen : 4096;
		if (fifo_func(Write, buf+(len-remainLen), translen, func_param))
			return Err_USB;
		
		remainLen -= translen;
	}

	return 0;

}

static int fifo_rd(
	unsigned int len,
	unsigned char *buf,
	register_func_t         *reg_func,
	fifo_func_t             *fifo_func,
	const void              *func_param
	)
{
	unsigned short fifolen;
	unsigned int tout;
	unsigned int remainLen;

	remainLen = len;
	while (remainLen > 0)
	{
                CACL_TAKES_TIME_BEGIN(fiford);
		do
		{
			reg_func(Read, 0x35, &fifolen, func_param);
			fifolen &= 0xFFF;
			//Sleep(1);
                        tout = CACL_TAKES_TIME_END(fiford);
		} while ((fifolen < 512) && (tout < 200));

		if (tout == 200)
			return Err_FIFO;

		if (fifo_func(Read, buf + (len - remainLen), 512, func_param))
			return Err_USB;

		remainLen -= 512;
	}

	return 0;

}

static int CardRST(
			sram_func_t	*sram_func,
			fifo_func_t *fifo_func,
			const void *func_param)
{
	
	unsigned int sramtemp[512];
	memset(sramtemp,0,512);
	sramtemp[8]=0x3;
	sram_func(Write,0,sramtemp,64,func_param);

	unsigned int timeout=0;

        CACL_TAKES_TIME_BEGIN(cardrst);

	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(cardrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
		
	if(sramtemp[8]&0x4000)
		return Err_CtrlRST1;	

	return 0;
}

static int CtrlRST(int RSTMode,
			sram_func_t	*sram_func,
			fifo_func_t *fifo_func,
			const void *func_param)
{
	unsigned int sramtemp[512];
	memset(sramtemp,0,512);
	sramtemp[0]=RSTMode==1?0x2:0x4;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

        CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT*2)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_CtrlRST1;	

	return 0;
}

static int CheckFlash(unsigned char *rbuf,
			register_func_t *reg_func,
			sram_func_t	*sram_func,
			fifo_func_t *fifo_func,
			const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];
	memset(sramtemp,0,512);
	sramtemp[0]=0x0;
	sramtemp[3]=1;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0]=0x85;
	//if(fifo_func(Write,data_buf,512,func_param))
		//return Err_USB;
	if (fifo_wr(512, data_buf, reg_func, fifo_func, func_param))
		return Err_USB;

	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_CheckFlash;	

	unsigned short fifolen;
	timeout = 0;
	do
	{
		reg_func(Read, 0x36, &fifolen, func_param);
		fifolen &= 0xFFF;
		Sleep(1);
		timeout++;
	} while ((fifolen < 256) && (timeout < 200));

	if (timeout == 200)
		return Err_FIFO;

	if(fifo_func(Read,rbuf,512,func_param))
		return Err_USB;

	return 0;
}

static int LoadCode(unsigned char *buff,
			 unsigned int len,
			 register_func_t *reg_func,
			 sram_func_t	*sram_func,
			 fifo_func_t *fifo_func,
			 const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];
	memset(sramtemp,0,512);
	sramtemp[0]=0x8;
	sramtemp[3]=len/512;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0] = 0x09;
	data_buf[1] = (unsigned char)(len/512 - 1);
	//if(fifo_func(Write,data_buf,512,func_param))
	//	return Err_USB;
	//if(fifo_func(Write,buff+512,len-512,func_param))
	//	return Err_USB;
	if (fifo_wr(512, data_buf, reg_func, fifo_func, func_param))
		return Err_USB;
	if (fifo_wr(len - 512, buff + 512, reg_func, fifo_func, func_param))
		return Err_USB;

	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_CheckFlash;	

	return 0;
}

static int RunPRAM(
	register_func_t *reg_func,
	sram_func_t	*sram_func,
	fifo_func_t *fifo_func,
	const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];
	memset(sramtemp,0,512);
	sramtemp[0]=0x8;
	sramtemp[3]=1;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0] = 0x2e;
	//if(fifo_func(Write,data_buf,512,func_param))
		//return Err_USB;
	if (fifo_wr(512,data_buf,reg_func,fifo_func,func_param))
		return Err_USB;

	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_RunPRAM;	

	return 0;
}

static int PreFormat(
			unsigned char *PreFMTData,
			register_func_t *reg_func,
			sram_func_t	*sram_func,
			fifo_func_t *fifo_func,
			const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];
	unsigned char pref_code = 0x02;
RETRY:
	sramtemp[0]=0x8;
	sramtemp[3]=1;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf, 0x00, 512);
	PreFMTData[0] = 0x07;
	PreFMTData[1]= pref_code;
	//if(fifo_func(Write,PreFMTData,512,func_param))
		//return Err_USB;
	if (fifo_wr(512, PreFMTData, reg_func, fifo_func, func_param))
		return Err_USB;

	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_PreFormat;

	if ( (sramtemp[0x10]&0x02000000) && (pref_code != 0x07) ) { // After Preformat, card is still locked.
		pref_code = 0x07;
		goto RETRY;
	}

	if (sramtemp[0x10]&0x02000000)
		return Err_PreFormat;

	return 0;
}

static int WriteVerifyCode(unsigned char *buff,
			 unsigned int len,
			 register_func_t *reg_func,
			 sram_func_t	*sram_func,
			 fifo_func_t *fifo_func,
			 const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];

	//program
	memset(sramtemp,0,512);
	sramtemp[0]=0x8;
	sramtemp[3]=len/512;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0] = 0x11;
	data_buf[1] = (unsigned char)((len/512 - 1)>>8);
	data_buf[2]= (unsigned char )(len/512-1);
	//if(fifo_func(Write,data_buf,512,func_param))
		//return Err_USB;
	//if(fifo_func(Write,buff+512,len-512,func_param))
		//return Err_USB;
	log_add("site %d,phison isp fifo start", ((int *)func_param)[1]);
	if (fifo_wr(512, data_buf, reg_func, fifo_func, func_param))
		return Err_USB;
	if (fifo_wr(len - 512, buff + 512, reg_func, fifo_func, func_param))
		return Err_USB;

	log_add("site %d,phison isp fifo OK", ((int *)func_param)[1]);

	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT*2)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_ProgISP;	
	
	log_add("site %d,phison isp write OK", ((int *)func_param)[1]);

	memset(sramtemp,0,512);
	sramtemp[0]=0x8;
	sramtemp[3]=len/512;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0] = 0x12;
	data_buf[1] = (unsigned char)((len/512 - 1)>>8);
	data_buf[2]= (unsigned char )(len/512-1);
	//if(fifo_func(Write,data_buf,512,func_param))
		//return Err_USB;
	//if(fifo_func(Write,buff+512,len-512,func_param))
		//return Err_USB;
	log_add("site %d,phison isp verify fifo start", ((int *)func_param)[1]);
	if (fifo_wr(512, data_buf, reg_func, fifo_func, func_param))
		return Err_USB;
	if (fifo_wr(len - 512, buff + 512, reg_func, fifo_func, func_param))
		return Err_USB;

	log_add("site %d,phison isp verify fifo OK", ((int *)func_param)[1]);

	CACL_TAKES_TIME_BEGIN(ctrlrst1);
	timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst1);
		if (timeout > TOUT*2)
			return Err_CtrlRST1;
	}


	log_add("site %d,phison isp verify result: %x", ((int *)func_param)[1],sramtemp[8]);
	if(sramtemp[8]&0x4000)
		return Err_ProgISP;	
	log_add("site %d,phison isp verify ok", ((int *)func_param)[1]);
	return 0;
}

static int WriteParaPage(unsigned char HWorWP,
			 unsigned char *buff,
			 register_func_t *reg_func,
			 sram_func_t	*sram_func,
			 fifo_func_t *fifo_func,
			 const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];
	memset(sramtemp,0,512);
	sramtemp[0]=0x8;
	sramtemp[3]=2;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0] = HWorWP==1?0x08:0x01;
	//if(fifo_func(Write,data_buf,512,func_param))
		//return Err_USB;
	//if(fifo_func(Write,buff,512,func_param))
		//return Err_USB;
	if (fifo_wr(512, data_buf, reg_func, fifo_func, func_param))
		return Err_USB;
	if (fifo_wr(512, buff , reg_func, fifo_func, func_param))
		return Err_USB;

	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_ProgHW;	

	return 0;
}

static int VerifyParaPage(unsigned char HWorWP,
					unsigned char *buff,
					register_func_t *reg_func,
					sram_func_t	*sram_func,
					fifo_func_t *fifo_func,
					const void *func_param)
{
	unsigned int sramtemp[512];
	unsigned char data_buf[512];
	memset(sramtemp,0,512);
	sramtemp[0]=0;
	sramtemp[3]=1;
	sramtemp[8]=0xb;
	sram_func(Write,0,sramtemp,64,func_param);

	memset(data_buf,0,512);
	data_buf[0] = HWorWP==1?0x84:0x82;
	//if(fifo_func(Write,data_buf,512,func_param))
		//return Err_USB;
	if (fifo_wr(512, data_buf, reg_func, fifo_func, func_param))
		return Err_USB;


	CACL_TAKES_TIME_BEGIN(ctrlrst);
	unsigned int timeout = 0;
	while ((sramtemp[8] & 0x8000) == 0)
	{
		if (sram_func(Read, 0, sramtemp, 64, func_param)) return Err_USB;
		timeout = CACL_TAKES_TIME_END(ctrlrst);
		if (timeout > TOUT)
			return Err_CtrlRST1;
	}
	if(sramtemp[8]&0x4000)
		return Err_ProgHW;	
	
	unsigned short fifolen;
	timeout = 0;
	do
	{
		reg_func(Read, 0x36, &fifolen, func_param);
		fifolen &= 0xFFF;
		Sleep(1);
		timeout++;
	} while ((fifolen < 256) && (timeout < 200));

	if (timeout == 200)
		return Err_FIFO;

	if(fifo_func(Read,data_buf,512,func_param))
		return Err_USB;

	if(memcmp(data_buf,buff,512)==0)
		return 0;
	else
		return Err_VerifyHW;
}

static int read_FW(unsigned char *buf,unsigned int *len, const TCHAR *PhisonFileA)
{          
	HANDLE hFile;
	unsigned int uiFileSize;

        unsigned long cnt;
#if __OLD_PATH__
        char path[MAX_PATH];

        cnt = GetModuleFileNameA(NULL, path, MAX_PATH);
	while (cnt--) {
		if (path[cnt] == L'\\')
			break;
	}
	path[cnt] = L'\0';
	strncat_s(path, PhisonFileA, sizeof(path) / sizeof(path[0]));
#else
        TCHAR path[MAX_PATH];

        path_datafile(PhisonFileA, path);
#endif
	hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return Err_NoFile;

	uiFileSize = GetFileSize(hFile,NULL);

	if(uiFileSize>1024*1024)
	{
		CloseHandle(hFile);
		return Err_FileFMT;
	}

	memset(buf, 0, 1024 * 1024);
	ReadFile(hFile,buf,uiFileSize,(LPDWORD)&uiFileSize,NULL);
	CloseHandle(hFile);
	*len=(uiFileSize/512+1)*512;

	if(buf[0]!='K' || buf[1]!='S' || buf[510]!= 0x55 || buf[511]!=0xAA)
		return Err_FileFMT;

	return 0;
}

static int read_ISP(unsigned char *buf,unsigned int *len, const TCHAR *PhisonFileB)
{
	HANDLE hFile;
	unsigned int uiFileSize;

        unsigned long cnt;
#if __OLD_PATH__
        char path[MAX_PATH];
        cnt = GetModuleFileNameA(NULL, path, MAX_PATH);
	while (cnt--) {
		if (path[cnt] == L'\\')
			break;
	}
	path[cnt] = L'\0';
	strncat(path, PhisonFileB, sizeof(path) / sizeof(path[0]));
#else
        TCHAR path[MAX_PATH];
        path_datafile(PhisonFileB, path);
#endif

	hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return Err_NoFile;

	uiFileSize = GetFileSize(hFile,NULL);

	if(uiFileSize>1024*1024)
	{
		CloseHandle(hFile);
		return Err_FileFMT;
	}
	memset(buf, 0, 1024 * 1024);
	ReadFile(hFile,buf,uiFileSize,(LPDWORD)&uiFileSize,NULL);
	CloseHandle(hFile);
	*len=(uiFileSize/512+1)*512;

	if(buf[0]!='K' || buf[1]!='S' || buf[510]!= 0x55 || buf[511]!=0xAA)
		return Err_FileFMT;

	return 0;
}

static int read_PreFMT(unsigned char *buf,unsigned int *len, const TCHAR *PhisonPreFMT)
{
	HANDLE hFile;
	unsigned int uiFileSize;

        unsigned long cnt;

#if __OLD_PATH__
        char path[MAX_PATH];

        cnt = GetModuleFileNameA(NULL, path, MAX_PATH);
	while (cnt--) {
		if (path[cnt] == L'\\')
			break;
	}
	path[cnt] = L'\0';
	strncat(path, PhisonPreFMT, sizeof(path) / sizeof(path[0]));
#else
        TCHAR path[MAX_PATH];
        path_datafile(PhisonPreFMT, path);
#endif

	hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return Err_NoFile;

	uiFileSize = GetFileSize(hFile,NULL);

	if(uiFileSize!=512)
	{
		CloseHandle(hFile);
		return Err_FileFMT;
	}
	memset(buf, 0, 1024 * 1024);
	ReadFile(hFile,buf,uiFileSize,(LPDWORD)&uiFileSize,NULL);
	CloseHandle(hFile);
	*len=512;

	return 0;
}

static int read_HW(unsigned char *buf,unsigned int *len, const TCHAR *PhisonHW)
{
       	HANDLE hFile;
		SYSTEMTIME sys;
	unsigned int uiFileSize;
        
        unsigned long cnt;
#if __OLD_PATH__
        char path[MAX_PATH];

        cnt = GetModuleFileNameA(NULL, path, MAX_PATH);
	while (cnt--) {
		if (path[cnt] == L'\\')
			break;
	}
	path[cnt] = L'\0';
	strncat(path, PhisonHW, sizeof(path) / sizeof(path[0]));
#else
        TCHAR path[MAX_PATH];
        
        path_datafile(PhisonHW, path);

#endif
	hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return Err_NoFile;

	uiFileSize = GetFileSize(hFile,NULL);

	if(uiFileSize!=512)
	{
		CloseHandle(hFile);
		return Err_FileFMT;
	}
	memset(buf, 0, 1024 * 1024);
	ReadFile(hFile,buf,uiFileSize,(LPDWORD)&uiFileSize,NULL);
	CloseHandle(hFile);
	*len=512;

	if(buf[0]!='H' || buf[1]!='V' || buf[2]!= '5' || buf[3]!='0')
		return Err_FileFMT;

	buf[495] = 0x02;	//good card mark

	GetLocalTime(&sys);
	buf[502] = ((sys.wYear & 0x0f) << 4) | (sys.wMonth & 0x0f);
	buf[503] = sys.wDay;


	return 0;
}

void CIDReplace(unsigned char *buf,unsigned char *ReplaceData)
{
        int i;
         
        for(i=0;i<5;i++)
                buf[300+i]=ReplaceData[i];
         
        unsigned char CID[16];
        for(i=0;i<16;i++)
                CID[i]=buf[i+290];
         
        unsigned char CRC7Tbl[256]={
        0x00,0x12,0x24,0x36,0x48,0x5a,0x6c,0x7e,0x90,0x82,0xb4,0xa6,0xd8,0xca,0xfc,0xee,
        0x32,0x20,0x16,0x04,0x7a,0x68,0x5e,0x4c,0xa2,0xb0,0x86,0x94,0xea,0xf8,0xce,0xdc,
        0x64,0x76,0x40,0x52,0x2c,0x3e,0x08,0x1a,0xf4,0xe6,0xd0,0xc2,0xbc,0xae,0x98,0x8a,
        0x56,0x44,0x72,0x60,0x1e,0x0c,0x3a,0x28,0xc6,0xd4,0xe2,0xf0,0x8e,0x9c,0xaa,0xb8,
        0xc8,0xda,0xec,0xfe,0x80,0x92,0xa4,0xb6,0x58,0x4a,0x7c,0x6e,0x10,0x02,0x34,0x26,
        0xfa,0xe8,0xde,0xcc,0xb2,0xa0,0x96,0x84,0x6a,0x78,0x4e,0x5c,0x22,0x30,0x06,0x14,
        0xac,0xbe,0x88,0x9a,0xe4,0xf6,0xc0,0xd2,0x3c,0x2e,0x18,0x0a,0x74,0x66,0x50,0x42,
        0x9e,0x8c,0xba,0xa8,0xd6,0xc4,0xf2,0xe0,0x0e,0x1c,0x2a,0x38,0x46,0x54,0x62,0x70,
        0x82,0x90,0xa6,0xb4,0xca,0xd8,0xee,0xfc,0x12,0x00,0x36,0x24,0x5a,0x48,0x7e,0x6c,
        0xb0,0xa2,0x94,0x86,0xf8,0xea,0xdc,0xce,0x20,0x32,0x04,0x16,0x68,0x7a,0x4c,0x5e,
        0xe6,0xf4,0xc2,0xd0,0xae,0xbc,0x8a,0x98,0x76,0x64,0x52,0x40,0x3e,0x2c,0x1a,0x08,
        0xd4,0xc6,0xf0,0xe2,0x9c,0x8e,0xb8,0xaa,0x44,0x56,0x60,0x72,0x0c,0x1e,0x28,0x3a,
        0x4a,0x58,0x6e,0x7c,0x02,0x10,0x26,0x34,0xda,0xc8,0xfe,0xec,0x92,0x80,0xb6,0xa4,
        0x78,0x6a,0x5c,0x4e,0x30,0x22,0x14,0x06,0xe8,0xfa,0xcc,0xde,0xa0,0xb2,0x84,0x96,
        0x2e,0x3c,0x0a,0x18,0x66,0x74,0x42,0x50,0xbe,0xac,0x9a,0x88,0xf6,0xe4,0xd2,0xc0,
        0x1c,0x0e,0x38,0x2a,0x54,0x46,0x70,0x62,0x8c,0x9e,0xa8,0xba,0xc4,0xd6,0xe0,0xf2};
         
        unsigned char regs;
        regs = 0;
        for(i = 0 ;i < 15; i++)
        {
                regs = CRC7Tbl[CID[i] ^ regs ]; 
        }
         
        CID[15]=regs+1;
        buf[305]=CID[15];
}
#pragma comment(lib,"Iphlpapi.lib") 
int get_mac(unsigned char *mac)
{
    PIP_ADAPTER_INFO pAdapterInfo;  
    DWORD AdapterInfoSize;  
    TCHAR szMac[32] = {0};  
    DWORD Err;  
 
    assert(mac);

    AdapterInfoSize = 0;  
    Err = GetAdaptersInfo(NULL, &AdapterInfoSize);  
     
    if((Err != 0) && (Err != ERROR_BUFFER_OVERFLOW))
    {  
        return   -1;  
    }  
     
    //   分配网卡信息内存  
    pAdapterInfo = (PIP_ADAPTER_INFO)malloc(AdapterInfoSize);  
    if(pAdapterInfo == NULL)
    {  
        return   -1;  
    }  
     
    if(GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize) != 0)
    {  
        free(pAdapterInfo);  
        return   -1;  
    }  
 
    *mac = pAdapterInfo->Address[0] ^
    pAdapterInfo->Address[1] ^
    pAdapterInfo->Address[2] ^ 
    pAdapterInfo->Address[3] ^ 
    pAdapterInfo->Address[4] ^ 
    pAdapterInfo->Address[5];  
 
    free(pAdapterInfo);

    return 0;
}

static int get_uid_index()
{
        return InterlockedIncrement((unsigned long *)&uid_index);
}

static int handler_WP(unsigned char *buf)
{
        int err;
        struct uid id;
        SYSTEMTIME sys;

        GetLocalTime(&sys);


        id.my = (sys.wMonth << 4) | ((sys.wYear - 1997) & 0xf);

        err = get_mac(&id.mac);
        if (err < 0)
                return err;
        id.dhm = sys.wDay * 24 * 60 + sys.wHour * 60 + sys.wMinute;
        id.index = get_uid_index();
        
        CIDReplace(buf, (unsigned char *)&id);

        return 0;
}

static int read_WP(unsigned char *buf,unsigned int *len, const TCHAR *PhisonWP)
{
        int err;
	HANDLE hFile;
	unsigned int uiFileSize;

        unsigned long cnt;

#if __OLD_PATH__
        char path[MAX_PATH];

        cnt = GetModuleFileNameA(NULL, path, MAX_PATH);
	while (cnt--) {
		if (path[cnt] == L'\\')
			break;
	}
	path[cnt] = L'\0';
	strncat(path, PhisonWP, sizeof(path) / sizeof(path[0]));
#else
        TCHAR path[MAX_PATH];
        path_datafile(PhisonWP, path);
#endif
	hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return Err_NoFile;

	uiFileSize = GetFileSize(hFile,NULL);

	if(uiFileSize!=512)
	{
		CloseHandle(hFile);
		return Err_FileFMT;
	}

	ReadFile(hFile,buf,uiFileSize,(LPDWORD)&uiFileSize,NULL);
        
        if (handler_WP(buf) < 0)
                err = Err_NoFile;
        else
                err = 0;

	CloseHandle(hFile);
	*len=512;

	return err;
}

static int pn_to_path(
                        const wchar_t *pn,
                        wchar_t PhisonFileA[MAX_PATH],
                        wchar_t PhisonFileB[MAX_PATH],
                        wchar_t PhisonFileC[MAX_PATH],
                        wchar_t PhisonHW[MAX_PATH],
                        wchar_t PhisonWP[MAX_PATH],
                        wchar_t PhisonPreFMT[MAX_PATH]
                        )
{
	TCHAR tmp[128];
	zstring_copy(tmp, pn, 128, _T('['));
	_stprintf(PhisonFileA, _T("%s\\%s"), tmp, FileA);
	_stprintf(PhisonFileB, _T("%s\\%s"), tmp, FileB);
	_stprintf(PhisonFileC, _T("%s\\%s"), tmp, FileC);
	_stprintf(PhisonHW, _T("%s\\%s"), tmp, HW);
	_stprintf(PhisonWP, _T("%s\\%s"), tmp, WP);
	_stprintf(PhisonPreFMT, _T("%s\\%s"), tmp, FMT);
	
	return 0;
}

extern "C" __declspec(dllexport)
int prepare_init(
                const struct chip_info_c       *chip,
                register_func_t         *reg_func,
		sram_func_t             *sram_func,
		fifo_func_t             *fifo_func,
		const void              *func_param
				 )
{
        //Path
        TCHAR PhisonFileA[128];
        TCHAR PhisonFileB[128];
        TCHAR PhisonFileC[128];
        TCHAR PhisonHW[128];
        TCHAR PhisonWP[128];
        TCHAR PhisonPreFMT[128];

	pn_to_path(
                        chip->part_name,
                        PhisonFileA,
                        PhisonFileB,
                        PhisonFileC,
                        PhisonHW,
                        PhisonWP,
                        PhisonPreFMT
                        );

        //Begin

	unsigned int filelen;
	int result;
	unsigned short regvalue;
	unsigned char *data_buf;

        log_add("site %d,new version");


        data_buf = (unsigned char *)malloc(1 * 1024 * 1024);
        if (!data_buf)
                return -1;

        log_add("site %d,phison 1", ((int *)func_param)[1]);

	//ctrl reset1
	regvalue=1;
	reg_func(Write,0x30, &regvalue,func_param);
	//set clock;
	regvalue=0xFC;
	reg_func(Write,0x12, &regvalue,func_param);
	if(CtrlRST(1,sram_func,fifo_func,func_param)) {
		//return Err_CtrlRST1;
                result = Err_CtrlRST1;
                goto end;
        }

        log_add("site %d,phison 2", ((int *)func_param)[1]);
	//set clock;
	regvalue=CLKSET;
	reg_func(Write,0x12,&regvalue,func_param);
	if(CheckFlash(data_buf,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_CheckFlash;
		//return Err_CheckFlash;
                goto end;
        }

        log_add("site %d,phison 3", ((int *)func_param)[1]);

	memset(data_buf,0,1024*1024);
	result=read_FW(data_buf,&filelen,PhisonFileA);
	if(result)
		return result;
	if(LoadCode(data_buf,filelen+512,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_LoadCode;
                goto end;
		//return Err_LoadCode;
        }

        log_add("site %d,phison 4", ((int *)func_param)[1]);

	if(RunPRAM(reg_func,sram_func,fifo_func,func_param)) {
                result = Err_RunPRAM;
                goto end;
		//return Err_RunPRAM;
        }

	if(CheckFlash(data_buf,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_CheckFlash;
		//return Err_CheckFlash;
                goto end;
        }

        log_add("site %d,phison 5", ((int *)func_param)[1]);

	result=read_PreFMT(data_buf,&filelen, PhisonPreFMT);
	if(result)
                goto end;
		//return result;
	if(PreFormat(data_buf,reg_func,sram_func,fifo_func,func_param)) {
		//return Err_PreFormat;
                result = Err_PreFormat;
                goto end;
        }

        log_add("site %d,phison 6", ((int *)func_param)[1]);

	if(CheckFlash(data_buf,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_CheckFlash;
                goto end;
		//return Err_CheckFlash;
        }

	memset(data_buf,0,1024*1024);
	result=read_ISP(data_buf,&filelen,PhisonFileB);
	if(result)
                goto end;
		//return result;
	
        log_add("site %d,phison 7", ((int *)func_param)[1]);

	if(WriteVerifyCode(data_buf,Firmware_B_Bin_Size,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_VerifyISP;
                goto end;
		//return Err_VerifyISP;
        }

	log_add("site %d,Phison ISP W&V OK", ((int *)func_param)[1]);
	//set clock;
	regvalue=0xFC;
	reg_func(Write,0x12,&regvalue,func_param);
	if(CtrlRST(2,sram_func,fifo_func,func_param)) {
                result = Err_CtrlRST2;
                goto end;
		//return Err_CtrlRST2;
        }

        log_add("site %d,phison 8", ((int *)func_param)[1]);

	//set clock;
	regvalue=CLKSET;
	reg_func(Write,0x12,&regvalue,func_param);
	if(CheckFlash(data_buf,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_CheckFlash;
                goto end;
		//return Err_CheckFlash;
        }
	result=read_HW(data_buf,&filelen,PhisonHW);
	if(result)
                goto end;
		//return result;
	if(WriteParaPage(1,data_buf,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_ProgHW;
		//return Err_ProgHW;
                goto end;
        }
	
        log_add("site %d,phison 9", ((int *)func_param)[1]);

	//set clock;
	regvalue=0xFC;
	reg_func(Write,0x12,&regvalue,func_param);
	if(CtrlRST(2,sram_func,fifo_func,func_param)) {
		//return Err_CtrlRST2;
                result = Err_CtrlRST2;
                goto end;
        }

        log_add("site %d,phison 10", ((int *)func_param)[1]);

	//set clock;
	regvalue=CLKSET;
	reg_func(Write,0x12,&regvalue,func_param);
	result=read_WP(data_buf+512,&filelen,PhisonWP);
	if(result)
                goto end;
		//return result;
	if(WriteParaPage(2,data_buf+512,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_ProgWP;
		//return Err_ProgWP;
                goto end;
        }

	//set clock;
	regvalue=0xFC;
	reg_func(Write,0x12,&regvalue,func_param);
	if(CtrlRST(2,sram_func,fifo_func,func_param)) {
                result = Err_CtrlRST2;
		//return Err_CtrlRST2;
                goto end;
        }

        log_add("site %d,phison 11", ((int *)func_param)[1]);

	//set clock;
	regvalue=CLKSET;
	reg_func(Write,0x12,&regvalue,func_param);
	if(VerifyParaPage(1,data_buf,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_VerifyHW;
                goto end;
		//return Err_VerifyHW;
        }
	if(VerifyParaPage(2,data_buf+512,reg_func,sram_func,fifo_func,func_param)) {
                result = Err_VerifyWP;
                goto end;
		//return Err_VerifyWP;
        }

        log_add("site %d,phison 12", ((int *)func_param)[1]);

	//set clock;
	regvalue=0xFC;
	reg_func(Write,0x12,&regvalue,func_param);
	if(CardRST(sram_func,fifo_func,func_param)) {
                result = Err_CtrlRST2;
		//return Err_CtrlRST2;
                goto end;
        }

end:

        log_add("site %d,phison 13", ((int *)func_param)[1]);
	//if (result)	//clean ID
	//{
	//	memset(data_buf, 0, 8);
	//	sram_func(Write, 0x10, (unsigned int *)data_buf, 2, func_param);
	//}
        if (data_buf)
                free(data_buf);
		
	return result;

}
