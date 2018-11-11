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
 * Description: MFCC feature extraction to match with TensorFlow MFCC Op
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mfcc.h"
#include "arm_math.h"

#define M_PI 3.14159265358979323846

inline float InverseMelScale(float mel_freq) {
  return 700.0f * (expf (mel_freq / 1127.0f) - 1.0f);
}

inline float MelScale(float freq) {
  return 1127.0f * logf (1.0f + freq / 700.0f);
}

MFCC_Inst * create_MFCC()
{

  MFCC_Inst *Inst  = (MFCC_Inst*)malloc(sizeof(MFCC_Inst));
  if(Inst == NULL) {
    printf("Malloc MFCC instance Error\n");
  }
  // Round-up to nearest power of 2.
  Inst->frame_len_padded = (int)pow(2,ceil((log(FRAME_LEN)/log(2))));

  Inst->frame = (float*)malloc(Inst->frame_len_padded * sizeof(float));
  if(Inst->frame_len_padded == NULL) {
    printf("MFCC Inst->frame \n");
  }

  Inst->buffer = (float*)malloc(Inst->frame_len_padded * sizeof(float));
  if(Inst->buffer == NULL) {
    printf("Malloc Inst->buffer failed\n");
  }
  Inst->mel_energies = (float *)malloc(NUM_FBANK_BINS * sizeof(float));
   if(Inst->mel_energies == NULL) {
    printf("Malloc Inst->mel_energies failed\n");
  } 

  //create window function
  Inst->window_func = (float *)malloc(FRAME_LEN * sizeof(float));
  for (int i = 0; i < FRAME_LEN; i++) {
    Inst->window_func[i] = (0.5 - 0.5*cos(M_2PI * ((float)i) / (FRAME_LEN)));
  //  printf("window[%d]=%d\n", i, Inst->window_func[i]);
  }


  //create mel filterbank
  Inst->fbank_filter_first = (int32_t *)malloc(NUM_FBANK_BINS * sizeof(int32_t));
  Inst->fbank_filter_last = (int32_t *)malloc(NUM_FBANK_BINS * sizeof(int32_t));

  create_mel_fbank(Inst);

  //create DCT matrix
  Inst->dct_matrix = create_dct_matrix(NUM_FBANK_BINS, NUM_MFCC_COEFFS);

  //initialize FFT
 // Inst->rfft = (arm_rfft_instance_q15 *)malloc(sizeof(arm_rfft_instance_q15));
 // arm_rfft_init_q15(Inst->rfft, Inst->frame_len_padded, 0, 1);

  Inst->rfft = (arm_rfft_fast_instance_f32 *)malloc(sizeof(arm_rfft_fast_instance_f32));
  if(Inst->rfft == NULL) {
    printf("Malloc Inst->rfft failed\n");
  }
  arm_rfft_fast_init_f32(Inst->rfft, Inst->frame_len_padded);
 return Inst;
}

void destory_MFCC(MFCC_Inst *handle) {
  free(handle->frame);
  free(handle->buffer);
  free(handle->mel_energies);
  free(handle->window_func);
  free(handle->fbank_filter_first);
  free(handle->fbank_filter_last);
  free(handle->dct_matrix);
  free(handle->rfft);
  for(int i=0;i<NUM_FBANK_BINS;i++)
    free(handle->mel_fbank[i]);
  free(handle->mel_fbank);
}

float * create_dct_matrix(int32_t input_length, int32_t coefficient_count) {
  int32_t k, n;
  float *M = (float*)malloc(input_length*coefficient_count * sizeof(float));
  if(M== NULL){
    printf("MFCC DCT failed\n");
  }
  float normalizer;
  arm_sqrt_f32(2.0/input_length,&normalizer);
  for (k = 0; k < coefficient_count; k++) {
    for (n = 0; n < input_length; n++) {
      M[k*input_length+n] = normalizer * cos( ((double)M_PI)/input_length * (n + 0.5) * k );
    }
  }
  return M;
}

void create_mel_fbank(MFCC_Inst *handle) {

  int32_t bin, i;

  int32_t num_fft_bins = handle->frame_len_padded/2;
  float fft_bin_width = ((float)SAMP_FREQ) / handle->frame_len_padded;
  float mel_low_freq = MelScale(MEL_LOW_FREQ);
  float mel_high_freq = MelScale(MEL_HIGH_FREQ);
  float mel_freq_delta = (mel_high_freq - mel_low_freq) / (NUM_FBANK_BINS+1);

  float *this_bin = (float*)malloc(num_fft_bins * sizeof(float));

  handle->mel_fbank = (float **)malloc(NUM_FBANK_BINS * sizeof(float*));

  for (bin = 0; bin < NUM_FBANK_BINS; bin++) {

    float left_mel = mel_low_freq + bin * mel_freq_delta;
    float center_mel = mel_low_freq + (bin + 1) * mel_freq_delta;
    float right_mel = mel_low_freq + (bin + 2) * mel_freq_delta;

    int32_t first_index = -1, last_index = -1;

    for (i = 0; i < num_fft_bins; i++) {

      float freq = (fft_bin_width * i);  // center freq of this fft bin.
      float mel = MelScale(freq);
      this_bin[i] = 0.0;

      if (mel > left_mel && mel < right_mel) {
        float weight;
        if (mel <= center_mel) {
          weight = (mel - left_mel) / (center_mel - left_mel);
        } else {
          weight = (right_mel-mel) / (right_mel-center_mel);
        }
        this_bin[i] = weight;
        if (first_index == -1)
          first_index = i;
        last_index = i;
      }
    }

    handle->fbank_filter_first[bin] = first_index;
    handle->fbank_filter_last[bin] = last_index;
    handle->mel_fbank[bin] = (float*)malloc((last_index-first_index+1) * sizeof(float));

    int32_t j = 0;
    //copy the part we care about
    for (i = first_index; i <= last_index; i++) {
      handle->mel_fbank[bin][j++] = this_bin[i];
    }
  }
  free(this_bin);
  return ;
}

void mfcc_compute(const int16_t * data, uint16_t dec_bits, int8_t * mfcc_out, MFCC_Inst *handle) {

  int32_t i, j, bin;

  //TensorFlow way of normalizing .wav data to (-1,1)
  for (i = 0; i < FRAME_LEN; i++) {
    handle->frame[i] = (float)data[i];
  }
  for(i=FRAME_LEN; i< handle->frame_len_padded; ++i) {
    handle->frame[i] = 0.0f;
  }

  for (i = 0; i < FRAME_LEN; i++) {
    handle->frame[i] *= handle->window_func[i];
  }

  //arm_rfft_q15(handle->rfft, handle->frame, handle->buffer);
  //Compute FFT
 arm_rfft_fast_f32(handle->rfft, handle->frame, handle->buffer, 0);

  //Convert to power spectrum
  //buffer is stored as [real0, realN/2-1, real1, im1, real2, im2, ...]
  int32_t half_dim = handle->frame_len_padded/2;
  float first_energy = handle->buffer[0] * handle->buffer[0],
        last_energy =  handle->buffer[1] * handle->buffer[1];  // handle this special case
  for (i = 1; i < half_dim; i++) {
    float real = handle->buffer[i*2], im = handle->buffer[i*2 + 1];
    handle->buffer[i] = real*real + im*im;
   // printf("float:%f int:%d\n", real*real+im*im, buffer[i]);
  }
  handle->buffer[0] = first_energy;
  handle->buffer[half_dim] = last_energy;

  float sqrt_data;
  //Apply mel filterbanks
  for (bin = 0; bin < NUM_FBANK_BINS; bin++) {
    j = 0;
    float mel_energy = 0;
    int32_t first_index = handle->fbank_filter_first[bin];
    int32_t last_index = handle->fbank_filter_last[bin];

    for (i = first_index; i <= last_index; i++) {
      arm_sqrt_f32(handle->buffer[i],&sqrt_data);
      mel_energy += (sqrt_data) * handle->mel_fbank[bin][j++];
    }
    handle->mel_energies[bin] = mel_energy;

    //avoid log of zero
    if (mel_energy == 0.0)
      handle->mel_energies[bin] = 0.000001;
  }

  //Take log
  for (bin = 0; bin < NUM_FBANK_BINS; bin++)
    handle->mel_energies[bin] = logf(handle->mel_energies[bin]);

  //Take DCT. Uses matrix mul.
  for (i = 0; i < NUM_MFCC_COEFFS; i++) {
    float sum = 0.0;
    for (j = 0; j < NUM_FBANK_BINS; j++) {
      sum += handle->dct_matrix[i*NUM_FBANK_BINS+j] * handle->mel_energies[j];
    }

    //Input is Qx.dec_bits (from quantization step)
    sum *= (0x1<<dec_bits);
    sum = round(sum);
    if(sum >= 127)
      mfcc_out[i] = 127;
    else if(sum <= -128)
      mfcc_out[i] = -128;
    else
      mfcc_out[i] = (int8_t)sum;
  }

}
