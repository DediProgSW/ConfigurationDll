#ifndef	_ASSIST_H
#define	_ASSIST_H

#define	ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#define	ALIGN_SIZE(a, align)	(((a + align - 1) / align) * align)

#define	ALIGN_SIZE_CNT(a, align)	((a + align - 1) / align)


#define OFFSET(type, member)    ((unsigned long)(&((type *)0)->member))

#ifndef max 
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#define	CACL_TAKES_TIME_BEGIN(x)	LARGE_INTEGER	x##cfre;\
					LARGE_INTEGER	x##cticks1, x##cticks2;\
					QueryPerformanceFrequency(&x##cfre);\
					QueryPerformanceCounter(&x##cticks1)

#define	CACL_TAKES_TIME_END(x)	((int)(QueryPerformanceCounter(&x##cticks2),\
				((x##cticks2.QuadPart - x##cticks1.QuadPart) * 1000 / (float)x##cfre.QuadPart)))


#define TAKES_TIME_INIT(x)               LARGE_INTEGER	x##tfre;\
					LARGE_INTEGER	x##tticks1, x##tticks2;\
	                                QueryPerformanceFrequency(&x##tfre);\
                                        QueryPerformanceCounter(&x##tticks1)

#define TAKES_TIME_REFRESH(x)           QueryPerformanceCounter(&x##tticks1)

#define	TAKES_TIME_CACL(x)	        ((int)(QueryPerformanceCounter(&x##tticks2),\
				        ((x##tticks2.QuadPart - x##tticks1.QuadPart) * 1000 / (float)x##tfre.QuadPart)))




#endif
