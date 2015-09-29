/*
 *  misc.c
 *
 *    Miscellaneous functions
 *
 *  Copyright (c) 2012-2013 STMicroelectronics Limited
 *  Author: Angus Clark <Angus.Clark@st.com>
 *
 *  ---------------------------------------------------------------------------
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 51
 *  Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *  ---------------------------------------------------------------------------
 *
 */
#include "stdafx.h"


#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <nand.h>

#include <xprintf.h>


int confirm_operation(void)
{
	char tmp[4];

	while (1) {
		if (scanf("%3s", tmp) == EOF) {
			eprintf("received EOF");
			return 0;
		}

		if (strcmp(tmp, "yes") == 0 ||
		    strcmp(tmp, "y") == 0)
			return 1;

		if (strcmp(tmp, "no") == 0 ||
		    strcmp(tmp, "n") == 0)
			return 0;
	}
}

int str2xxx(char *str, char *strs[], int n_strs)
{
	int i;

	for (i = 0; i < n_strs; i++)
		if (strs[i] && strcmp(strs[i], str) == 0)
			return i;

	return -1;
}

int read_data(char *fn_dat, uint8_t *data, uint32_t size)
{
        FILE *fp;
        struct stat stat_buf;

        if (stat(fn_dat, &stat_buf) != 0) {
                eprintf("failed to stat file [%s]\n", fn_dat);
                return 1;
        }

        if (stat_buf.st_size != size) {
                eprintf("file [%s] not %u bytes [%u]\n", fn_dat,
			(unsigned int)size,
			(unsigned int)stat_buf.st_size);
                return 1;
        }

        if ((fp = fopen(fn_dat, "r")) == NULL) {
                eprintf("Failed to open file [%s]\n", fn_dat);
                return 1;
        }

        if (fread(data, size, 1, fp) != 1) {
                eprintf("failed to read data from file [%s]\n", fn_dat);
                fclose(fp);
                return 1;
        }

        fclose(fp);

        return 0;
}

int write_data(char *fn_dat, uint8_t *data, int size)
{
        FILE *fp;

        if ((fp = fopen(fn_dat, "w")) == NULL) {
                eprintf("failed to create file [%s]\n", fn_dat);
                return 1;
        }

        if (fwrite(data, size, 1, fp) != 1) {
                eprintf("failed to write data to file [%s]\n", fn_dat);
                return 1;
        }

        fclose(fp);

        return 0;

}

int ffs64(uint64_t x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffffffff)) {
		x >>= 32;
		r += 32;
	}
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

int cmp_ints(const void *a, const void *b)
{
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    return *ia  - *ib;
}

void inject_errors(uint8_t *data, int size, int n_errors, int **_err_pos)
{
	int i, j;
	int *err_pos;
	int pos, found_new;
	int byte, bit;

	err_pos = (int *)xxmalloc(n_errors * sizeof(int));

	for (i = 0; i < n_errors; i++) {
		found_new = 0;
		while (!found_new) {
			pos = rand()%(size*8);
			found_new = 1;
			for (j = 0; j < i; j++) {
				if (err_pos[j] == pos) {
					found_new = 0;
					break;
				}
			}
		}
		err_pos[i] = pos;

	}

	qsort(err_pos, n_errors, sizeof(int), cmp_ints);

	for (i = 0; i < n_errors; i++) {
		byte = err_pos[i]/8;
		bit = err_pos[i]%8;
		data[byte] ^= 1 << bit;
	}

	if (_err_pos)
		*_err_pos = err_pos;
	else
		free(err_pos);
}

extern "C" {
	int detect_errors(uint8_t *data1, uint8_t *data2, int len, int **err_pos)
	{
		int i, j;
		uint8_t x;
		int e, n_errors = 0;

		for (i = 0; i < len; i++) {
			x = data1[i] ^ data2[i];
			n_errors += bits_set[x];
		}

		if (err_pos) {
			if (n_errors) {
				*err_pos = (int *)xxmalloc(n_errors * sizeof(int));
				e = 0;
				for (i = 0; i < len; i++) {
					x = data1[i] ^ data2[i];

					for (j = 0; j < 8 && x; j++) {
						if (x & 0x1)
							(*err_pos)[e++] = (i * 8) + j;
						x >>= 1;
					}
				}
			}
			else {
				*err_pos = NULL;
			}
		}

		return n_errors;
	}
}

void *xxmalloc(size_t size)
{
	void *v;

	v = malloc(size);
	if (!v) {
		fprintf(stderr, "out of memory\n");
		exit(-1);
	}
	return v;
}
