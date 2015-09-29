/*
 *  xprintf.h
 *
 *  Standard macros for eprintf(), dprintf(), and vprintf().  vprintf() requires
 *  'int verbose' within scope.
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

#ifndef XPRINTF_H
#define XPRINTF_H

#ifndef NAME
#define NAME ""
#endif

extern int verbose;

/* Macros for errors, verbose messages, and debug */
#define eprintf(format, ...)					\
	do {								\
	fprintf(stderr, "%s:%s(): " format, NAME, ##__VA_ARGS__); \
		fflush(stderr);						\
	} while (0)
#define vbprintf(format, ...)				\
	if (verbose)						\
{printf(format, ##__VA_ARGS__); fflush(stdout); }
#ifdef DEBUG
#define dprintf(format, args...)				  \
	do {printf ("%s:%s(): " format , NAME, __func__, ##args); \
		fflush(stdout);					  \
	} while (0)
#else
#define dprintf(format, ...) do {} while (0)
#endif

#endif /* XPRINTF_H */
