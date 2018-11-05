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
 * Description: Keyword spotting example code using MFCC feature extraction
 * and DNN model.
 */
#include <stdio.h>
#include <stdlib.h>
#include "kws.h"
#include "../MFCC/mfcc.h"
#include "../DNN/dnn.h"
   
#include "wav_olivia_1.h"
int16_t audio_buff[16000] = KWS_DATA;

int get_top_detection(q7_t* prediction)
{
  int max_ind=0;
  int max_val=-128;
  for(int i=0;i<OUT_DIM;i++) {
    if(max_val<prediction[i]) {
      max_val = prediction[i];
      max_ind = i;
    }
  }
  return max_ind;
}

void average_predictions(int window_len, q7_t (*predictions)[OUT_DIM], q7_t *output, q7_t *averaged_output)
{
  //shift right old predictions
  for(int i=window_len-1;i>0;i--) {
    for(int j=0;j<OUT_DIM;j++)
      predictions[i][j]=predictions[i-1][j];
  }
  //add new predictions
  for(int j=0;j<OUT_DIM;j++)
    predictions[0][j]=output[j];
  //compute averages
  int sum;
  for(int j=0;j<OUT_DIM;j++) {
    sum=0;
    for(int i=0;i<window_len;i++)
      sum += predictions[i][j];
    averaged_output[j] = (q7_t)(sum/window_len);
  }
}

char output_class[12][8] = {"Silence", "Unknow", "yes", "no", "down", "left", "right", "on", "off", "stop", "go"};

kws_inst *kws_init(void) 
{
  kws_inst *handle = (kws_inst*)malloc(sizeof(kws_inst));
  if(handle == NULL) {
    printf("malloc error\n");
    return NULL;
  }
  handle->mfcc_inst = create_MFCC();
  handle->dnn_inst  = create_nn(handle->scratch_buffer);
  
  memset(handle->audio_buffer, 0, 8000*sizeof(int16_t));
  memset(handle->mfcc_buffer, 0, MFCC_BUFFER_SIZE * sizeof(q7_t));
  handle->mfcc_buffer_head = 0;
  handle->read_pos  = 0;
  handle->write_pos = 0;
  return handle;
}

int kws_process(kws_inst *handle)
{
  int num_frames = 0;
  int max_ind  = 0;
  int averaging_window_len = 3;
  int detection_threshold = 0;
  q7_t output[OUT_DIM];
  q7_t averaged_output[OUT_DIM];
  memcpy(handle->audio_buffer + handle->write_pos, audio_buff, 16000);
  handle->write_pos = 16000;
  
  
  num_frames = (handle->write_pos - handle->read_pos) / (FRAME_SHIFT);
  for(unsigned int f=0; f<num_frames; ++f) {
    memmove(handle->mfcc_buffer, handle->mfcc_buffer + (num_frames * NUM_MFCC_COEFFS), (NUM_FRAMES-num_frames) * NUM_MFCC_COEFFS);
    int32_t mfcc_buffer_head = (NUM_FRAMES - num_frames) * NUM_MFCC_COEFFS;
    for(uint16_t f=0; f < num_frames; ++f) {
      mfcc_compute(handle->audio_buffer + f*FRAME_LEN, 2, handle->mfcc_buffer + mfcc_buffer_head, handle->mfcc_inst);
      mfcc_buffer_head += NUM_MFCC_COEFFS;
    }
    run_nn(handle->mfcc_buffer, output, handle->dnn_inst);
    arm_softmax_q7(output, OUT_DIM, output);
    
    average_predictions(averaging_window_len, handle->predictions, output, averaged_output);
    max_ind = get_top_detection(averaged_output);
    if(averaged_output[max_ind] > detection_threshold *128/100) {
      printf("Detected %s (%d%%), (%d%%)\n", output_class[max_ind], ((int)averaged_output[max_ind]*100),
             ((int)output[max_ind]*100/128));
    }  
  }
  if(max_ind == 2)
      return 1;
  else 
      return 0;
}

void kws_reset(kws_inst *handle)
{
  memset(handle->audio_buffer, 0, 8000*sizeof(int16_t));
  memset(handle->mfcc_buffer, 0, MFCC_BUFFER_SIZE * sizeof(q7_t));
  handle->mfcc_buffer_head = 0;
  handle->read_pos         = 0;
  handle->write_pos        = 0;
}
  
void kws_destory(kws_inst *handle) 
{
  destory_MFCC(handle->mfcc_inst);
  if(handle->dnn_inst) free(handle->dnn_inst);
  if(handle) free(handle);
}




