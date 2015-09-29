/*
 *  hamming_ecc.c
 *
 *  Copyright (c) 2012-2013 STMicroelectronics Limited
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

#ifndef NO_STDLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdint.h>
#include "hamming_ecc.h"


/* Parity look up table */
const uint8_t byte_parity_table[] = {
	0x00, 0x2B, 0x2D, 0x06, 0x33, 0x18, 0x1E, 0x35,
	0x35, 0x1E, 0x18, 0x33, 0x06, 0x2D, 0x2B, 0x00,
	0x4B, 0x60, 0x66, 0x4D, 0x78, 0x53, 0x55, 0x7E,
	0x7E, 0x55, 0x53, 0x78, 0x4D, 0x66, 0x60, 0x4B,
	0x4D, 0x66, 0x60, 0x4B, 0x7E, 0x55, 0x53, 0x78,
	0x78, 0x53, 0x55, 0x7E, 0x4B, 0x60, 0x66, 0x4D,
	0x06, 0x2D, 0x2B, 0x00, 0x35, 0x1E, 0x18, 0x33,
	0x33, 0x18, 0x1E, 0x35, 0x00, 0x2B, 0x2D, 0x06,
	0x53, 0x78, 0x7E, 0x55, 0x60, 0x4B, 0x4D, 0x66,
	0x66, 0x4D, 0x4B, 0x60, 0x55, 0x7E, 0x78, 0x53,
	0x18, 0x33, 0x35, 0x1E, 0x2B, 0x00, 0x06, 0x2D,
	0x2D, 0x06, 0x00, 0x2B, 0x1E, 0x35, 0x33, 0x18,
	0x1E, 0x35, 0x33, 0x18, 0x2D, 0x06, 0x00, 0x2B,
	0x2B, 0x00, 0x06, 0x2D, 0x18, 0x33, 0x35, 0x1E,
	0x55, 0x7E, 0x78, 0x53, 0x66, 0x4D, 0x4B, 0x60,
	0x60, 0x4B, 0x4D, 0x66, 0x53, 0x78, 0x7E, 0x55,
	0x55, 0x7E, 0x78, 0x53, 0x66, 0x4D, 0x4B, 0x60,
	0x60, 0x4B, 0x4D, 0x66, 0x53, 0x78, 0x7E, 0x55,
	0x1E, 0x35, 0x33, 0x18, 0x2D, 0x06, 0x00, 0x2B,
	0x2B, 0x00, 0x06, 0x2D, 0x18, 0x33, 0x35, 0x1E,
	0x18, 0x33, 0x35, 0x1E, 0x2B, 0x00, 0x06, 0x2D,
	0x2D, 0x06, 0x00, 0x2B, 0x1E, 0x35, 0x33, 0x18,
	0x53, 0x78, 0x7E, 0x55, 0x60, 0x4B, 0x4D, 0x66,
	0x66, 0x4D, 0x4B, 0x60, 0x55, 0x7E, 0x78, 0x53,
	0x06, 0x2D, 0x2B, 0x00, 0x35, 0x1E, 0x18, 0x33,
	0x33, 0x18, 0x1E, 0x35, 0x00, 0x2B, 0x2D, 0x06,
	0x4D, 0x66, 0x60, 0x4B, 0x7E, 0x55, 0x53, 0x78,
	0x78, 0x53, 0x55, 0x7E, 0x4B, 0x60, 0x66, 0x4D,
	0x4B, 0x60, 0x66, 0x4D, 0x78, 0x53, 0x55, 0x7E,
	0x7E, 0x55, 0x53, 0x78, 0x4D, 0x66, 0x60, 0x4B,
	0x00, 0x2B, 0x2D, 0x06, 0x33, 0x18, 0x1E, 0x35,
	0x35, 0x1E, 0x18, 0x33, 0x06, 0x2D, 0x2B, 0x00
};

/* Parity look up table */

const uint8_t  bits_set[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

/*******************************************************************************/
#define COL_LOOP_STEP(c__f, c__e, c__o, c__t)	\
	c__o ^= (c__f ? c__t : 1);		\
	c__e ^= (c__f ? 1 : c__t);

/* Generate 3 byte ECC code for ecc_size block p_data.  "p_data" is a pointer to
 * the date and must be 4-byte aligned.  "size" gives length of "p_data" - one
 * of enum ecc_size.
 */
int ecc_gen(uint8_t* p_data, uint8_t ecc[3], enum ecc_size size)
{
	uint32_t* p_data_long = (uint32_t*)p_data;
	uint32_t parity_bits[18];  /* maximum number */
	uint32_t reg32;
	uint32_t temp;
	uint32_t int_cnt;
	uint32_t bit_cnt;

	unsigned int num_parity_bits;

	uint8_t* p_byt;
	uint8_t byte_reg;
	uint8_t byte_a;
	uint8_t byte_b;
	uint8_t byte_c;
	uint8_t byte_d;

	switch (size) {
	case ECC_128:
		num_parity_bits = 14;
		break;
	case ECC_256:
		num_parity_bits = 16;
		break;
	case ECC_512:
		num_parity_bits = 18;
		break;
	default:
#ifdef NO_STDLIB
		return 1;
#else
		fprintf(stderr, "Internal error in ecc_gen: unknown format\n");
		exit(1);
#endif
	}

	/* Initialize variables */
	byte_reg = 0;
	reg32 = 0;

	ecc[0] = ecc[1] = ecc[2] = 0;

	for (bit_cnt = 0; bit_cnt < num_parity_bits; bit_cnt ++) {
		parity_bits[bit_cnt] = 0;
	}

	/* Build up column parity */
	for (int_cnt = 0; int_cnt < size/sizeof(uint32_t); int_cnt++) {
		temp = p_data_long[int_cnt];
		switch (size) {
		case ECC_512:
			COL_LOOP_STEP((int_cnt & 0x40), parity_bits[16], parity_bits[17], temp);
			/* fall through */
		case ECC_256:
			COL_LOOP_STEP((int_cnt & 0x20), parity_bits[14], parity_bits[15], temp);
			/* fall through */
		case ECC_128:
			COL_LOOP_STEP((int_cnt & 0x01), parity_bits[4], parity_bits[5], temp);
			COL_LOOP_STEP((int_cnt & 0x02), parity_bits[6], parity_bits[7], temp);
			COL_LOOP_STEP((int_cnt & 0x04), parity_bits[8], parity_bits[9], temp);
			COL_LOOP_STEP((int_cnt & 0x08), parity_bits[10], parity_bits[11], temp);
			COL_LOOP_STEP((int_cnt & 0x10), parity_bits[12], parity_bits[13], temp);
		}
	}

	reg32 = parity_bits[12] ^ parity_bits[13];

	p_byt = (uint8_t*)&reg32;
#if __LITTLE_ENDIAN__
	byte_a = p_byt[3];
	byte_b = p_byt[2];
	byte_c = p_byt[1];
	byte_d = p_byt[0];
#else
#error "Big endian?  Really?"
	byte_a = p_byt[0];
	byte_b = p_byt[1];
	byte_c = p_byt[2];
	byte_d = p_byt[3];
#endif

	byte_reg = byte_a ^ byte_b ^ byte_c ^ byte_d;
	byte_reg = byte_parity_table[byte_reg] >> 1;

	/* Create line parity */
	parity_bits[0] = byte_d ^ byte_b;
	parity_bits[1] = byte_c ^ byte_a;
	parity_bits[2] = byte_d ^ byte_c;
	parity_bits[3] = byte_b ^ byte_a;

	for (bit_cnt = 4; bit_cnt < num_parity_bits; bit_cnt++) {
		p_byt = (uint8_t*)(parity_bits + bit_cnt);
		/* NB Only LS Byte of parity_bits used from now on */
		p_byt[0] ^= (p_byt[1] ^ p_byt[2] ^ p_byt[3]);
	}

	/* Calculate final ECC code */
	for (bit_cnt = 0; bit_cnt < 8; bit_cnt ++)
		ecc[0] |= (byte_parity_table[(uint8_t)parity_bits[bit_cnt]]
			   & 0x01) << bit_cnt;
	for (; bit_cnt < 16 && bit_cnt < num_parity_bits; bit_cnt ++)
		ecc[1] |= (byte_parity_table[(uint8_t)parity_bits[bit_cnt]]
			   & 0x01) << (bit_cnt - 8);
	for (; bit_cnt < num_parity_bits; bit_cnt ++)
		ecc[2] |= (byte_parity_table[(uint8_t)parity_bits[bit_cnt]]
			   & 0x01) << (bit_cnt - 16);

	ecc[2] = (uint8_t)(byte_reg << 2) | (ecc[2] & 0x03);

	return 0;
}


/* Detect and correct a 1 bit error in a 128, 256 or 512 byte block.
 *
 *  "p_data" is a pointer to the data.
 *  "old_ecc" is the proper ECC for the data (stored)
 *  "new_ecc" is the ECC generated from the (possibly) corrupted data.
 *  The size of the block is given in "size".
 *
 * Returns whether the data needed correcting, or was not correctable.  If the
 * data was corrected, then the byte:bit position is set.
 */
enum ecc_check ecc_correct(uint8_t *p_data,
			   uint8_t old_ecc[3],
			   uint8_t new_ecc[3],
			   enum ecc_size size,
			   int *byte, int *bit)
{
	uint8_t bit_cnt02;
	uint8_t bit_addr02;
	unsigned int byte_addr02;

	uint8_t ecc_xor[3];

	uint8_t error_bit_count;

	switch (size) {
	case ECC_128:
		error_bit_count = 10;
		break;
	case ECC_256:
		error_bit_count = 11;
		break;
	case ECC_512:
		error_bit_count = 12;
		break;
	default:
#ifdef NO_STDLIB
		return E_FATAL;
#else
		fprintf(stderr, "Internal error in ecc_correct: unknown format\n");
		exit(1);
#endif
	}

	/* Basic Error Detection phase */
	ecc_xor[0] = new_ecc[0] ^ old_ecc[0];
	ecc_xor[1] = new_ecc[1] ^ old_ecc[1];
	ecc_xor[2] = new_ecc[2] ^ old_ecc[2];

	if ((ecc_xor[0] | ecc_xor[1] | ecc_xor[2]) == 0) {
			return E_NO_CHK;  /* No errors */
	}

	/* If we get here then there were errors */

	if (size == ECC_512) {
		/* 512-byte error correction requires a little more than 128 or
		 * 256.  If there is a correctable error then the xor will have
		 * 12 bits set, but there can also be 12 bits set in some
		 * uncorrectable errors.  This can be solved by xoring the odd
		 * and even numbered bits.
		 *
		 *  0xAA = 10101010
		 *  0x55 = 01010101
		 */
		bit_cnt02  = bits_set[((ecc_xor[0] & 0xAA) >> 1) ^
				      (ecc_xor[0] & 0x55)];
		bit_cnt02 += bits_set[((ecc_xor[1] & 0xAA) >> 1) ^
				      (ecc_xor[1] & 0x55)];
		bit_cnt02 += bits_set[((ecc_xor[2] & 0xAA) >> 1) ^
				      (ecc_xor[2] & 0x55)];
	} else {
		/* Counts the number of bits set in ecc code */
		bit_cnt02  = bits_set[ecc_xor[0]];
		bit_cnt02 += bits_set[ecc_xor[1]];
		bit_cnt02 += bits_set[ecc_xor[2]];
	}

	if (bit_cnt02 == error_bit_count) {
		/* Set the bit address */
		bit_addr02 = ((ecc_xor[2] >> 3) & 0x01) |
			((ecc_xor[2] >> 4) & 0x02) |
			((ecc_xor[2] >> 5) & 0x04);

		/* Evaluate 2 LS bits of address */
		byte_addr02 = ((ecc_xor[0] >> 1) & 0x01) |
			((ecc_xor[0] >> 2) & 0x02);

		/* Add in remaining bits of address */
		switch (size) {
		case ECC_512:
			byte_addr02 |= (((unsigned int)ecc_xor[2]) << 7) &
				0x100;
			/* Fall through */
		case ECC_256:
			byte_addr02 |= (ecc_xor[1] & 0x80);
			/* Fall through */
		case ECC_128:
			byte_addr02 |= ((ecc_xor[0] >> 3) & 0x04) |
				((ecc_xor[0] >> 4) & 0x08) |
				((ecc_xor[1] << 3) & 0x10) |
				((ecc_xor[1] << 2) & 0x20) |
				((ecc_xor[1] << 1) & 0x40);
		}


		/* Correct bit error in the data */
		p_data[byte_addr02] ^= (0x01 << bit_addr02);

		/* NB p_old_code is okay, p_new_code is corrupt */
		if (byte)
			*byte = byte_addr02;
		if (bit)
			*bit = bit_addr02;
		return E_D1_CHK;  /* Data had 1-bit error (now corrected) */
	} else if (bit_cnt02 == 1) {
		return E_C1_CHK;  /* ECC code has 1-bit error, data is okay */
	} else {
		return E_UN_CHK;  /* Uncorrectable Error */
	}
}



/*******************************************************************************/
#ifdef TESTING
/* To test this code, compile as follows:
 *
 *  sh4gcc -mboard=<board> -g ecc.c -DTESTING
 *
 * This provides a sanity test ONLY. It gives complete coverage of the code, but
 * does not fully exercise the mathmatics behind the algorithm.
 */
#include <stdlib.h>
#include <time.h>

int main()
{
	uint8_t data[512];
	uint8_t ecc1[3];
	uint8_t ecc2[3];
	int i, size;

	int byte, bit;

	int status;
	uint8_t data1[512];
	uint8_t data2[512];


	for (size = 128; size <= 512; size *= 2) {
		printf("Testing size %d\n", size);

		/* Create a data set */
		for (i = 0; i < size; i++)
			data[i] = i & 0xFF;

		/* Create the initial ecc */
		ecc_gen(data, ecc1, size);

		/* Make sure that it doesn't try to "correct" it before it's
		 * corrupted
		 */
		if (ecc_correct(data, ecc1, ecc1, size, &byte, &bit) != E_NO_CHK) {
			printf("Error: failed to detect good data.\n");
			exit(1);
		}

		for (i = 0; i < size; i++) {
			if (data[i] != (i & 0xFF)) {
				printf("Error: corrupted good data.\n");
				exit(1);
			}
		}

		/* Deliberately corrupt the data with 1 bit error only */
		data[size-42] ^= 1;

		/* Create a new ecc for the bad data */
		ecc_gen(data, ecc2, size);

		/* Make sure that it can fix the issues */
		if (ecc_correct(data, ecc1, ecc2, size, &byte, &bit) != E_D1_CHK) {
			printf("Error: failed to diagnose 1 bit data error.\n");
			exit(1);
		}

		for (i = 0; i < size; i++)
			if (data[i] != (i & 0xFF)) {
				printf("Error: did not correct bad data.\n");
				exit(1);
			}
		/* Data is now GOOD */

		/* Deliberately corrupt the ECC with 1 bit error */
		ecc2[0] = ecc1[0];
		ecc2[1] = ecc1[1] ^ 4;
		ecc2[2] = ecc1[2];

		/* Simulate reading bad ECC from flash, but calculating good ECC from data */
		if (ecc_correct(data, ecc2, ecc1, size, &byte, &bit) != E_C1_CHK) {
			printf("Error: failed to diagnose 1 bit ECC error.\n");
			exit(1);
		}

		/* Deliberately corrupt the data with a 2-bit error */
		data[size-42] ^= 3;

		/* Create a new ecc for the bad data */
		ecc_gen(data, ecc2, size);

		/* Check that it reports that it cannot correct the issue */
		if (ecc_correct(data, ecc1, ecc2, size, &byte, &bit) != E_UN_CHK) {
			printf("Error: failed to diagnose 2-bit corruption.\n");
			exit(1);
		}

		printf("ECC %d working as expected\n", size);
		getchar();
	}

	/* The above tests show that all is well with the various code paths.
	 * However, it does not test detection of 2-bit error detection so well.
	 * These tests generate and corrupt random data sets and ensures that
	 * the algorithm does not mis-diagnose the condition.
	 *
	 *  Notes:
	 *  1. There is nothing to stop it corrupting the same bit twice - in
	 *     which case it will correctly diagnose a 1-bit error.
	 *  2. 3-bit errors and worse cause undefined behaviour, so we don't
	 *     test those.
	 */
	printf("\nECC 512 random data tests (interrupt when satisfied)\n");

	srand(time(NULL));
	while (1) {
		for (i = 0; i < 512; i++)
			/* Random number 0..255 */
			data1[i] = (uint8_t)(256.0 * rand() / (RAND_MAX + 1.0));

		ecc_gen(data1, ecc1, ECC_512);

		memcpy(data2, data1, 512);

		for (i = 0; i < 1; i++) {
			/* Random number 0..514 */
			int corruptbyte = (int)(515.0 * rand() / (RAND_MAX + 1.0));
			/* Random number 0..7 */
			int corruptbit = (int)(8.0 * rand() / (RAND_MAX + 1.0));

			if (corruptbyte < 512)
				data2[corruptbyte] ^= 1 << corruptbit;
			else
				ecc1[corruptbyte - 512] ^= 1 << corruptbit;

			printf("%3d:%d ", corruptbyte, corruptbit);
		}

		ecc_gen(data2, ecc2, ECC_512);

		status = ecc_correct(data2, ecc1, ecc2, ECC_512, &byte, &bit);
		printf("ECC correction: %d\n", status);
		for (i = 0; i < 512; i++) {
			if (data1[i] != data2[i])
				printf("data1[%d] = 0x%02x !=  data2[i] = 0x%02x\n",
				       i, data1[i], i, data2[i]);
		}
		getchar();

		/*
		  if (ecc_correct(data, ecc1, ecc2, ECC_512) != E_UN_CHK)
		  printf("Error: failed to diagnose 2-bit-corruption\n");
		  else
		  printf("OK\n");
		*/
	}

	return 0;
}
#endif
