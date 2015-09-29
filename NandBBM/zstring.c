/* author: findstr
 * data: 26/04/2014
 * rev: 0.0.2
 * modify:
 *      can modify the string terminated charactor
 *	add unicode support
 *      @zstring_init init the zstring module
 *      @zstring_set_terminalch set the terminal charactor not only the 
 *              zero
 *      @zstring_len caculate the string length,
 *              the terminal charactor can be zero
 *              or variable terminal_ch can be seted via zstring_set_terminalch
 *      @zstring_sztoul can translate the string to unsigned long value
 *              not only depend the terminated charactor
 */
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "zstring.h"

//static unsigned int terminal_ch;
//static const wchar_t *(* w_skip)(const wchar_t *p);
//static const char *(* a_skip)(const char *p);
//
//int zstring_init()
//{
//        terminal_ch = 0;
//	w_skip = NULL;
//	a_skip = 0;
//        return 0;
//}
//
//int zstring_set_terminalch(int terminal_char)
//{
//        terminal_ch = terminal_char;
//
//        return 0;
//}
//
//int zstring_set_skip_w(const wchar_t *(*skip)(const wchar_t *))
//{
//	w_skip = skip;
//
//	return 0;
//}
//
//
//int zstring_set_skip_a(const char *(*skip)(const char *))
//{
//	a_skip = skip;
//
//	return 0;
//}
//
///*--------------------------------ASCII---------------------------------------------*/
//
//int zstring_len_a(const char *str)
//{
//        const char *p;
//
//	if (a_skip)
//		str = a_skip(str);
//
//        p = str;
//
//        while (*str != terminal_ch && *str != '\0')
//                str++;
//
//        return str - p;
//}
//
//static unsigned long sztoul_10_a(const char *ptr, int cnt)
//{
//        int value;
//
//        value = 0;
//
//        while ((*ptr != 0) && (cnt--) && (*ptr != terminal_ch)) {
//                value *= 10;
//                value += (*ptr++ & 0x0f);
//        }
//
//        return value;
//}
//
//unsigned long zstring_sztoul_a(const char *ptr, unsigned long ch_cnt, int base)
//{
//        int value;
//
//	if (a_skip)
//		ptr = a_skip(ptr);
//
//        if (ptr[0] == '0' && (ptr[1] | 0x20) == 'x') {
//                ptr += 2;
//                if (ch_cnt > 2)
//                        ch_cnt -= 2;
//                else
//                        ch_cnt = 0;
//                if (base != 16)
//                        return 0;
//	}
//
//        if (ptr[0] == '0' && (ptr[1] | 0x20) != 'x' && base != 8)
//                return 0;
//
//        value = 0;
//
//        while (ptr[0] != '\0' && ptr[0] != terminal_ch && ch_cnt--) {
//                value *= base;
//                value += (*ptr >> 6) * (10 - 1) + (*ptr & 0xf);
//                ptr++;
//        }
//
//        return value;
//}
//
//char *zstring_copy_a(char *dst, const char *src, int n)
//{
//	char *ret;
//
//	assert(dst);
//	assert(src);
//
//	if (a_skip)
//		src = a_skip(src);
//
//	ret = dst;
//
//	while (n-- > 0 && (*dst = *src) != terminal_ch) {
//		dst++;
//		src++;
//	}
//
//	*dst = 0;
//
//	return ret;
//}

/*--------------------------------unicode---------------------------------------------*/
int zstring_len_w(const wchar_t *str, int terminal_ch)
{
        const wchar_t *p;

        p = str;
        while (*str != terminal_ch && *str != L'\0')
                str++;

        return str - p;
}

static unsigned long sztoul_10_w(const wchar_t *ptr, int cnt, int terminal_ch)
{
        int value;

        value = 0;

        while ((*ptr != 0) && (cnt--) && (*ptr != terminal_ch)) {
                value *= 10;
                value += (*ptr++ & 0x0f);
        }

        return value;
}

unsigned long zstring_sztoul_w(const wchar_t *ptr, unsigned long ch_cnt, int base, int terminal_ch)
{
        unsigned long value;

        if (ptr[0] == L'0' && (ptr[1] | 0x20) == L'x') {
                ptr += 2;
                if (ch_cnt > 2)
                        ch_cnt -= 2;
                else
                        ch_cnt = 0;
                if (base != 16)
                        return 0;
        }

        if (ptr[0] == L'0' && (ptr[1] | 0x20) != L'x' && base != 8)
                return 0;

        value = 0;

        while (ptr[0] != L'\0' && ptr[0] != terminal_ch && ch_cnt--) {
                value *= base;
		if (!isalnum(*ptr))
			break;
                value += (*ptr >> 6) * (10 - 1) + (*ptr & 0xf);
                ptr++;
        }

        return value;
}

unsigned long long zstring_autotoull_w(const wchar_t *ptr, int terminal_ch)
{
	int base;
	int len;
	unsigned long long value;

	len = zstring_len_w(ptr, terminal_ch);
	if (len > 2 && ptr[0] == L'0' && (ptr[1] | 0x20) == L'x') {
		len -= 2;
		base = 16;
		ptr += 2;
	} else {
		base = 10;
	}

	value = 0;

        while (ptr[0] != L'\0' && ptr[0] != terminal_ch && len--) {
                value *= base;
		if (!isalnum(*ptr))
			break;
                value += (*ptr >> 6) * (10 - 1) + (*ptr & 0xf);
                ptr++;
        }

        return value;
}

wchar_t *zstring_copy_w(wchar_t *dst, const wchar_t *src, int n, int terminal_ch)
{
	wchar_t *ret;

	assert(dst);
	assert(src);

	ret = dst;

	while (n-- > 0 && (*dst = *src) != terminal_ch && (*src != L'\0')) {
		dst++;
		src++;
	}

	*dst = 0;

	return ret;
}

wchar_t *zstring_token_w(wchar_t *dst, const wchar_t *src, int n, int sep, zstring_skip_w_t *skip, int terminal_ch)
{
	wchar_t *ret;

	assert(dst);
	assert(src);

	if (skip)
		src = skip(src);

	while (n-- > 0 && (*src != sep) && (*dst = *src) != terminal_ch && (*src != L'\0')) {
		dst++;
		src++;
	}

	*dst = 0;
	if (*src == terminal_ch || *src == L'\0')
		return NULL;
	else
		return ++src;
}

static __inline int w_equ(const wchar_t *src, const wchar_t *dst, int terminal_ch)
{
	while ((*src == *dst) && (*src != terminal_ch) && (*src != L'\0') && (*dst != L'\0')) {
		dst++;
		src++;
	}
	
	if (*dst == 0)
		return 1;

	return 0;
}

wchar_t *zstring_str_w(const wchar_t *src, const wchar_t *dst, int terminal_ch)
{
	int equ;
	wchar_t *p;

	p = src;

	do {
		equ = w_equ(src, dst, terminal_ch);
		src++;
	} while (!equ && (*src != terminal_ch) && (*src != L'\0'));

	if (equ)
		return --src;
	return NULL; 
}
