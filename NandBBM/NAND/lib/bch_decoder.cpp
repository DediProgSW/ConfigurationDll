/*
 *  bch_decoder.c
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

/**
 * Includes
 */
#include "stdafx.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


#include "gf_precomputed.h"

#define GF2_N 16383
#define M_VALUE 14

/**
 * MACROs
 */


/*------------------------------------------------------------------------
 * Variables
 *----------------------------------------------------------------------*/
/******************************************************************************/
static inline int mul_gf( int a, int b) {
  /*---------------------------------------------------------------------------
   * GF multiplier
   ****************************************************************************/
    int out;
    if ((a==0)||(b==0)) {
       out =0;
    }
    else {
       out =GF2_alpha_to[(GF2_index_of[a]+GF2_index_of[b])%GF2_N];
    }

    return(out);
}






/*************************************************************************/
void bch_decoder(int* error_pos_dec,
            char* output,
            char* chien_out,
			char* input,
			int length_in,
			int length_out,
			int* stat_decoder)
/*------------------------------------------------------------------------
 * LDPC encoder
 * IN:
 *   input	 Input stream
 * OUT:
 *   output	  Output stream
 * PRE-CONDITIONS:
 *************************************************************************/
{


  int k,l;


  int t;
  int test_syndrome;
  int * synd;
  int *  sigma;
  int *  alpha_chien;
  int *  mul_sigma;
  int *  mul_beta;
  int *  beta;
  int *  beta_sav;
  int *  sigma_new;
  int val_chien,nb_errors;
  int dp,dr,l_bm,r_bm, min_val;   /* variables for bm_block */
  int sigma_degree;

  t= (length_in-length_out)/M_VALUE;

  synd=(int *) malloc((2*t-1) * sizeof(int));
  sigma      =(int *) malloc((t+1) * sizeof(int));
  mul_sigma  =(int *) malloc((t+1) * sizeof(int));
  mul_beta   =(int *) malloc((t+1) * sizeof(int));
  beta       =(int *) malloc((t+1) * sizeof(int));
  beta_sav   =(int *) malloc((t+1) * sizeof(int));
  sigma_new  =(int *) malloc((t+1) * sizeof(int));
  alpha_chien=(int *) malloc((t+1) * sizeof(int));

  /* Init syndromes */

  for (k=0;k<2*t-1;k++){
    synd[k]=0;
  }


    /* SYNDROME *******************************/

    for (l=0;l<length_in;l++) {
      for (k=0;k<2*t-1;k++){
        if (synd[k]==0)
          synd[k]=input[l];
        else
          synd[k]=GF2_alpha_to[(GF2_index_of[synd[k]]+k+1)%GF2_N]^input[l];
      }
    }

    /* test if syndromes are zeros : if test_syndrom0, received frame is a codeword -> no errors or at least 2t+1 erros */

    test_syndrome=0;
    for (k=0;k<2*t-1;k++){
      if (synd[k]>0) test_syndrome=1;
    }


    /* BM_BLOCK *******************************/
    /*init*/

    if (t==2) {  // simplified BM block for t=2
      if (test_syndrome==0) { sigma[0]=1; sigma[1]=0; sigma[2]=0;}
      else {
        sigma[0] = synd[0];
        sigma[1]=mul_gf(synd[0],synd[0]);
        sigma[2]=mul_gf(sigma[1],synd[0])^synd[2];
       }
    }


    else {     // recursive Berlekamp Masse algo

     for (k=0;k<t+1;k++){
        mul_sigma[k]=0;
        sigma_new[k]=0;
        mul_beta[k]=0;
        sigma[k]=0;
        beta[k]=0;
      }
      sigma[0]=1;
      sigma[1]=synd[0];
      if (synd[0]==0){
        dp=1; l_bm=0; beta[3]=1;
      }
      else {
        dp=synd[0]; l_bm=1; beta[2]=1;
      }

      for (r_bm=1;r_bm<t;r_bm++){
        dr=0; min_val=((2*r_bm<t) ? 2*r_bm : t);

        for (k=0;k< min_val+1;k++){
            dr=dr ^ mul_gf(sigma[k],synd[2*r_bm-k]);
        }

        for (k=0;k<t+1;k++){
          mul_sigma[k]  = mul_gf(dp,sigma[k]);
          mul_beta[k]   = mul_gf(dr,beta[k]);
          sigma_new[k]  = mul_sigma[k] ^ mul_beta[k];
        }
        if ((dr==0)||(r_bm<l_bm)) {
          for (k=2;k<t+1;k++)
            beta_sav[k]=beta[k-2];
          for (k=2;k<t+1;k++)
            beta[k]=beta_sav[k];
        }
        else {
          for (k=2;k<t+1;k++)
            beta[k]=sigma[k-2];
          l_bm=2*r_bm-l_bm+1;
          dp=dr;
        }

        for (k=0;k<t+1;k++)  sigma[k]=sigma_new[k];

     }  /* end loop (r_bm=1;r_bm<t;r_bm++) */

    } /* end Berlekamp */


    /* CHIEN SEARCH *******************************/
    /* multiplication by alpha(GF2_N-N_BCH) */
    sigma_degree=0;
    for (k=t;k>=0;k--) {
      if (sigma_degree==0 && sigma[k]) sigma_degree=k;
    }



    for (k=0;k<t+1;k++){
       alpha_chien[k]=GF2_alpha_to[(k*(GF2_N+1-length_in))%GF2_N];
       sigma[k]=mul_gf(sigma[k],alpha_chien[k]);
    }



    nb_errors = 0;
    for (l=0;l<length_in;l++) {
      val_chien = 0;
      for (k=0;k<t+1;k++){
        val_chien^=sigma[k];
      }

      for (k=0;k<t+1;k++){
        if (sigma[k]!=0)
          sigma[k]=GF2_alpha_to[(GF2_index_of[sigma[k]]+k)%GF2_N];
      }
      chien_out[l]=((val_chien==0)?1:0);
      if (val_chien==0) error_pos_dec[nb_errors]=l;
      if (val_chien==0) nb_errors++;

      output[l]=(input[l]+ chien_out[l])%2;
    }



 /* DEBUG */
/*  printf("** nb_errors found in Chien search = %d\n",nb_errors);
    printf("** sigma degree = %d\n",sigma_degree);*/
    stat_decoder[0]=sigma_degree;  // error_number
    stat_decoder[1]=test_syndrome; // error_found
    stat_decoder[2]=(nb_errors!=sigma_degree);  // uncorrectable


  free(synd);
  free(sigma);
  free(mul_sigma);
  free(mul_beta);
  free(beta);
  free(beta_sav);
  free(sigma_new);
  free(alpha_chien);

}
