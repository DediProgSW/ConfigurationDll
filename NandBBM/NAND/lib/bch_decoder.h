/*
 *  bch_decoder.h
 *
 *    Extracted from BCH_M2R-rel-6-0 package
 *
 *  Copyright (c) 2012-2013 STMicroelectronics Limited
 *  Author: HED - HVD - R&D FE design
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


/************************************************************************
 * BCH DECODER
 */
void bch_decoder(int* error_pos_dec,
		 char* output,
		 char* chien_out,
		 char* input,
		 int length_in,
		 int length_out,
		 int* stat_decoder);
