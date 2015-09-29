/*
 *  bch_encoder.c
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
#include <time.h>
#include <math.h>

#include "bch_encoder.h"
#include "bch_c_parameters.h"




/*
 * Bit-true BCH encoder
 */




/*************************************************************************/
void bch_encoder(char* output,
	      char* input,
	      int sel_code,
	      int input_length,
	      int output_length
		  )
/*------------------------------------------------------------------------
 * BCH encoder
 * IN:
 *   input	 Input stream
 * OUT:
 *   output	  Output stream
 * PRE-CONDITIONS:
 *  QPSK only
 *************************************************************************/
{

int k,l;

char * prbs_state;
char * prbs_state_sav;
char * prbs_state2;
char * prbs_state_sav2;
char * prbs_state3;
char * prbs_state_sav3;

char * output1;
char * output2;
char * output3;

prbs_state=(char *) malloc((output_length-input_length) * sizeof(char));
prbs_state_sav=(char *) malloc((output_length-input_length) * sizeof(char));
prbs_state2=(char *) malloc((output_length-input_length) * sizeof(char));
prbs_state_sav2=(char *) malloc((output_length-input_length) * sizeof(char));
prbs_state3=(char *) malloc((output_length-input_length) * sizeof(char));
prbs_state_sav3=(char *) malloc((output_length-input_length) * sizeof(char));

output1 = (char *) malloc((output_length) * sizeof(char));
output2 = (char *) malloc((output_length) * sizeof(char));
output3 = (char *) malloc((output_length) * sizeof(char));

char S[4];
char g[435];



// mux code
  if (sel_code==0){
    for (k=0;k<output_length-input_length+1;k++){
      g[k]=g_1[k];
    }
  }
  else {
    for (k=0;k<output_length-input_length+1;k++){
      g[k]=g_2[k];
    }
  }


#if 0
//  version 1
//*************************************************************************

  for (l=0;l<output_length-input_length;l++){
        prbs_state[l]= 0;
  }

  for (k=0;k<input_length;k++){

      prbs_state_sav[output_length-input_length-1] = input[k] ^ prbs_state[0];

      for (l=1;l<output_length-input_length;l++){
        prbs_state_sav[l-1]= (prbs_state[l]^(g[l]&prbs_state[0]));
      }
	  for (l=0;l<output_length-input_length;l++){
	    prbs_state[l]= prbs_state_sav[l];
      }
  }

  for (k=0;k<output_length-input_length;k++){

      prbs_state_sav[output_length-input_length-1] =  prbs_state[0];

      for (l=1;l<output_length-input_length;l++){
        prbs_state_sav[l-1]= (prbs_state[l]^(g[l]&prbs_state[0]));
      }
	  for (l=0;l<output_length-input_length;l++){
	    prbs_state[l]= prbs_state_sav[l];
      }
  }



  for (k=0;k<input_length;k++){
    output1[k]=input[k];
  }
  for (k=input_length;k<output_length;k++){
    output1[k]=prbs_state[k-input_length];
  }
#endif

//  version 2
//*************************************************************************
/*
  for (l=0;l<output_length-input_length;l++){
        prbs_state2[l]= 0;
  }

  for (k=0;k<input_length;k++){

      prbs_state_sav2[output_length-input_length-1] = input[k] ^ prbs_state2[0];

      for (l=1;l<output_length-input_length;l++){
        prbs_state_sav2[l-1] = prbs_state2[l] ^ (g[l] & (input[k] ^ prbs_state2[0]));
      }

      for (l=0;l<output_length-input_length;l++){
        prbs_state2[l]= prbs_state_sav2[l];
      }
  }

  for (k=0;k<input_length;k++){
    output2[k]=input[k];
  }
  for (k=input_length;k<output_length;k++){
    output2[k]=prbs_state2[k-input_length];
  }
*/


//  version 3    parallelisation by 4
//*************************************************************************


  for (l=0;l<output_length-input_length;l++) {
        prbs_state3[l]= 0;
  }

  // prbs loop
  for (k=0;k<ceil((float)(input_length)/4);k++) {

    S[0] = input[4*k] ^ prbs_state3[0];
    S[1] = input[4*k+1] ^ (prbs_state3[1] ^ (g[1] & S[0]));
    S[2] = input[4*k+2] ^ (prbs_state3[2] ^ (g[2] & S[0]) ^ (g[1] & S[1]));
    S[3] = input[4*k+3] ^ (prbs_state3[3] ^ (g[3] & S[0]) ^ (g[2] & S[1]) ^ (g[1] & S[2]));


    // the 4 last data of the prbs are computed differently
    prbs_state_sav3[output_length-input_length-1] = S[3];
    prbs_state_sav3[output_length-input_length-2] = S[2] ^ (g[output_length-input_length-1] & S[3]);
    prbs_state_sav3[output_length-input_length-3] = S[1] ^ (g[output_length-input_length-1] & S[2]) ^ (g[output_length-input_length-2] & S[3]);
    prbs_state_sav3[output_length-input_length-4] = S[0] ^ (g[output_length-input_length-1] & S[1]) ^ (g[output_length-input_length-2] & S[2]) ^ (g[output_length-input_length-3] & S[3]);


    // next prbs state calculation (4 "standard" cycles later)
    for (l=0;l<output_length-input_length-4;l++){
      prbs_state_sav3[l] = prbs_state3[l+4] ^ (g[l+4] & S[0]) ^ (g[l+3] & S[1]) ^ (g[l+2] & S[2]) ^ (g[l+1] & S[3]);
    }

    // update state
    for (l=0;l<output_length-input_length;l++){
      prbs_state3[l] = prbs_state_sav3[l];
    }
  }  // for k


  // generate output data
  for (k=0;k<input_length;k++){
    output3[k]=input[k];
  }

  for (k=input_length;k<output_length;k++) output3[k]=prbs_state3[k-input_length];



//  output
//*************************************************************************

  for (k=0;k<output_length;k++){
    output[k] = output3[k];
  }
#if 0
  // check the parallelized model
  for (k=0;k<output_length;k++){
    if (output1[k] != output3[k]) {
      printf("C model difference at %d\n",k);
    }
  }
#endif


free(prbs_state);
free(prbs_state_sav);
free(prbs_state2);
free(prbs_state_sav2);
free(prbs_state3);
free(prbs_state_sav3);
free(output1);
free(output2);
free(output3);

}
