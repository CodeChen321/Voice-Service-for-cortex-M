#include "avs_private.h"
#include "kws.h"

static int32_t afe_voice_trigger_found( uint32_t commandIndex)
{
  AVS_instance_handle *pInstance = avs_core_get_instance();
  avs_core_message_send(pInstance, EVT_WAKEUP, commandIndex);
  return AVS_OK;
}

static void avs_kws_task(const void *argument);
static void avs_kws_task(const void *argument)
{
  int kws_flag = 0;
  int ret = 0;

  AVS_audio_handle *pAHandle = (AVS_audio_handle *)(uint32_t)argument;
  AVS_instance_handle *pHandle = pAHandle->pInstance;

  pAHandle->kwsStatus = 0;
  pAHandle->runKwsRuning = 1;

  kws_inst *kws_handle = kws_init();
 // kws_flag = kws_process(kws_handle);
  /*Wait the instance is created !*/
  while(avs_core_atomic_read(&pHandle->bInstanceStarted) == 0){
     avs_core_task_delay(500);
  }
  /*Leave some time to print message without events at the start-up*/
  avs_core_task_delay(2000);
  

  // HERE is the kws code
  while(pAHandle->runKwsRuning) {
    avs_core_event_wait(&pAHandle->newDataReceived, INFINITE_DELAY);
    uint32_t sizeStreamSpeaker = pAHandle->recognizerPipe.outBuffer.szConsumer;
    uint32_t blkSize           = 16000 - (kws_handle->write_pos - kws_handle->read_pos);
    if(blkSize > sizeStreamSpeaker){
      blkSize = sizeStreamSpeaker;
    }
    switch (pAHandle->hKwsState) {
      case 0x01:   
        ret = avs_avs_capture_audio_stream_buffer(pAHandle,kws_handle->audio_buffer + kws_handle->write_pos, blkSize);
        kws_handle->write_pos += ret;
        kws_flag = kws_process(kws_handle);
      break;
      case 0x00:
        kws_reset(kws_handle);
      default:
        kws_reset(kws_handle);
        kws_flag = 0;
      break;
    }

    if(kws_flag) {
      pAHandle->hKwsState = 0;
      afe_voice_trigger_found( 0);
    }
  }
  return;
}

AVS_Result avs_kws_task_create(AVS_audio_handle *pHandle)
{
  /*Start the thread*/
  pHandle->hKwsState = 0x01;
  pHandle->hKwsTask = avs_core_task_create(KWS_TASK_NAME, avs_kws_task, pHandle, KWS_TASK_STACK_SIZE, KWS_TASK_PRIORITY);
  AVS_ASSERT(pHandle->hKwsTask );
  if(pHandle->hKwsTask == 0) {
    AVS_TRACE_ERROR("Create task %s", KWS_TASK_NAME);
    return AVS_ERROR;
  }
  return AVS_OK;
}

AVS_Result avs_kws_task_delete(AVS_audio_handle *pHandle)
{
  AVS_TRACE_DEBUG("Enter kws delete");
 
  if(pHandle->hKwsTask != 0) {
    avs_core_task_delete(pHandle->hKwsTask);
  }
  pHandle->hKwsTask = 0;
  return AVS_OK;
}
