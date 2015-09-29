#ifndef _ZSTRING_H
#define _ZSTRING_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _UNICODE

typedef const wchar_t * (zstring_skip_w_t)(const wchar_t *);
typedef const wchar_t * (zstring_skip_a_t)(const wchar_t *);

#define	zstring_len(str, terminal)			zstring_len_w(str)
#define	zstring_sztoul(p, char_cnt, base, terminal)	zstring_sztoul_w(p, char_cnt, base, terminal)
#define	zstring_copy(dst, src, n, terminal)		zstring_copy_w(dst, src, n, terminal)
#define	zstring_token(dst, src, n, sep, skip, termi)	zstring_token_w(dsr, src, n, sep, skip, termi)
#define zstring_str(src, dst, terminal)			zstring_str_w(src, dst, terminal)

int zstring_len_w(const wchar_t *str, int terminal_ch);
unsigned long zstring_sztoul_w(const wchar_t *ptr, unsigned long ch_cnt, int base, int terminal_ch);
unsigned long long zstring_autotoull_w(const wchar_t *ptr, int terminal_ch);
wchar_t *zstring_copy_w(wchar_t *dst, const wchar_t *src, int n, int terminal_ch);
wchar_t *zstring_token_w(wchar_t *dst, const wchar_t *src, int n, int sep, zstring_skip_w_t *skip, int terminal_ch);
wchar_t *zstring_str_w(const wchar_t *src, const wchar_t *dst, int terminal_ch);
#else

#define	zstring_len(str)		zstring_len_a(str)
#define	zstring_sztoul(p, char_cnt, base)	zstring_sztoul_a(p, char_cnt, base)
#define	zstring_copy(dst, src, n)	zstring_copy_a(dst, src, n)
#define	zstring_set_skip(skip)		zstring_set_skip_a(skip)


int zstring_len_a(const char *str);
unsigned long zstring_sztoul_a(const char *p, unsigned long ch_cnt, int base);
wchar_t *zstring_copy_a(char *dst, const char *src, int n);
int zstring_set_skip_a(const char *(*skip)(const char *));


#endif // _UNICODE

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // !_STRING_Z_H
