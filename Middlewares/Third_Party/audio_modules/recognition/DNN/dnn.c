/*
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Description: Keyword spotting DNN model example code
 */

/* Network Structure

  10x25 input features
    |
   IP1 : Innerproduct (weights: 250x144)
    |
   IP2 : Innerproduct (weights: 144x144)
    |
   IP3 : Innerproduct (weights: 144x144)
    |
   IP4 : Innerproduct (weights: 144x12)
    |
   12 outputs

*/

#include "dnn.h"
#include <stdlib.h>
#include <stdio.h>


const q7_t ip1_wt[IP1_WT_DIM]=IP1_WT;
const q7_t ip1_bias[IP1_OUT_DIM]=IP1_BIAS;
const q7_t ip2_wt[IP2_WT_DIM]=IP2_WT;
const q7_t ip2_bias[IP2_OUT_DIM]=IP2_BIAS;
const q7_t ip3_wt[IP3_WT_DIM]=IP3_WT;
const q7_t ip3_bias[IP3_OUT_DIM]=IP3_BIAS;
const q7_t ip4_wt[IP4_WT_DIM]=IP4_WT;
const q7_t ip4_bias[OUT_DIM]=IP4_BIAS;

NN_Inst* create_nn(q7_t* scratch_pad)
{
  NN_Inst *Inst = (NN_Inst *)malloc(sizeof(NN_Inst));
  if(Inst == NULL){
    printf("Malloc NN failed\n");
  }
  Inst->ip1_out = scratch_pad;
  Inst->ip2_out = Inst->ip1_out+IP1_OUT_DIM;
  Inst->ip3_out = Inst->ip1_out;
  Inst->vec_buffer = (q15_t*)(Inst->ip1_out+IP1_OUT_DIM+IP2_OUT_DIM);

  return Inst;
}

void run_nn(q7_t* in_data, q7_t* out_data, NN_Inst* handle)
{
	// Run all layers

	// IP1
	arm_fully_connected_q7(in_data, ip1_wt, IN_DIM, IP1_OUT_DIM, 1, 7, ip1_bias, handle->ip1_out, handle->vec_buffer);

    /*for (int i=0; i< 144; i=i+8) {
    	printf("ip1_out[%d]:%d\n\r", i,handle->ip1_out[i]);
    }*/

        // RELU1
	arm_relu_q7(handle->ip1_out, IP1_OUT_DIM);

	// IP2
	arm_fully_connected_q7(handle->ip1_out, ip2_wt, IP1_OUT_DIM, IP2_OUT_DIM, 2, 8, ip2_bias, handle->ip2_out, handle->vec_buffer);
/*
    for (int i=0; i< 144; i=i+8) {
    	printf("ip2_out[%d]:%d\n\r", i,handle->ip2_out[i]);
    }*/
        // RELU2
	arm_relu_q7(handle->ip2_out, IP2_OUT_DIM);

	// IP3
	arm_fully_connected_q7(handle->ip2_out, ip3_wt, IP2_OUT_DIM, IP3_OUT_DIM, 2, 9, ip3_bias, handle->ip3_out, handle->vec_buffer);

   /* for (int i=0; i< 144; i=i+8) {
    	printf("ip3_out[%d]:%d\n\r", i,handle->ip3_out[i]);
    }*/
        // RELU3
	arm_relu_q7(handle->ip3_out, IP3_OUT_DIM);

	// IP4
	arm_fully_connected_q7(handle->ip3_out, ip4_wt, IP3_OUT_DIM, OUT_DIM, 0, 6, ip4_bias, out_data, handle->vec_buffer);
    /*for (int i=0; i< 12; ++i) {
    	printf("out_data[%d]:%d\n\r", i,out_data[i]);
    }*/

}


