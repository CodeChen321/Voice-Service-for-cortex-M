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

#ifndef __KWS_H__
#define __KWS_H__

#include "arm_math.h"
#include "../DNN/dnn.h"
#include "../MFCC/mfcc.h"

#define MAX_SLIDING_WINDOW 10
   
typedef struct _kws_inst {
    int16_t audio_buffer[640*2]; // should be large enough
    q7_t scratch_buffer[SCRATCH_BUFFER_SIZE];
    MFCC_Inst *mfcc_inst;
    NN_Inst *dnn_inst;
    q7_t mfcc_buffer[MFCC_BUFFER_SIZE];
    q7_t output[OUT_DIM];
    int32_t mfcc_buffer_head;
    q7_t predictions[MAX_SLIDING_WINDOW][OUT_DIM];
    int read_pos;
    int write_pos;
    int count;
    int last_maxid;
}kws_inst;

kws_inst *kws_init(void);

int kws_process(kws_inst *handle);
void kws_reset(kws_inst *handle);
void kws_destory(kws_inst *handle);

#endif /*_kws_*/
