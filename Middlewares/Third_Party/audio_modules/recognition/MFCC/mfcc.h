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

#ifndef KWS_MFCC_H
#define KWS_MFCC_H

#include "arm_math.h"
#include <string.h>

#define SAMP_FREQ 16000
#define FRAME_SHIFT_MS 40
#define FRAME_LENGTH_MS 40

#define NUM_FBANK_BINS 40
#define NUM_MFCC_COEFFS 10
#define MEL_LOW_FREQ 20
#define MEL_HIGH_FREQ 4000

#define FRAME_SHIFT ((int16_t)(SAMP_FREQ * 0.001 * FRAME_SHIFT_MS))
#define FRAME_LEN ((int16_t)(SAMP_FREQ * 0.001 * FRAME_LENGTH_MS))

#define NUM_FRAMES 25
#define MFCC_BUFFER_SIZE (NUM_FRAMES*NUM_MFCC_COEFFS)

#define M_2PI 6.283185307179586476925286766559005

typedef struct MFCC_Inst_{
    int32_t frame_len_padded;
    float * frame;
    float * buffer;
    float * mel_energies;
    float * window_func;
    int32_t * fbank_filter_first;
    int32_t * fbank_filter_last;
    float ** mel_fbank;
    float * dct_matrix;
    arm_rfft_fast_instance_f32 * rfft;
  //  arm_rfft_instance_q15 *rfft;
}MFCC_Inst;

MFCC_Inst * create_MFCC();
void destory_MFCC(MFCC_Inst *handle);

float InverseMelScale(float mel_freq);
float MelScale(float freq);

float * create_dct_matrix(int32_t input_length, int32_t coefficient_count);
void create_mel_fbank(MFCC_Inst *handle);

void mfcc_compute(const int16_t * data, uint16_t dec_bits, int8_t * mfcc_out, MFCC_Inst *handle);
#endif
