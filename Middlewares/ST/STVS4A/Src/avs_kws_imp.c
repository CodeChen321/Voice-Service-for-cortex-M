#include "avs_private.h"
#include "kws.h"

#define EXPORT_WAV

#ifdef EXPORT_WAV
#include "ff.h"
#include "wav_util.h"

FRESULT fr;
FATFS fs;
FIL fil;

void * wav_open(const char *filename,
                    const char * mode)
{
	FIL *fp=NULL;
	printf("mode:%s,%d\n\r",mode,strlen(mode));
	if(strlen(mode)==2&&!strncmp(mode,"wb",2)){
		fp=malloc(sizeof(FIL));
		fr=f_open(fp,filename,FA_WRITE | FA_CREATE_ALWAYS);
		if(fr!=FR_OK){
			free(fp);
			return NULL;
		}
	}
	return (void *)fp;
}
int wav_close(void * stream)
{
	f_close((FIL *)stream);
	return 0;
}
size_t wav_write(void * ptr,
                    size_t size, size_t nmemb, void * stream)
{
	uint32_t num;
	FIL *fp=(FIL *)stream;
	f_write(fp,ptr,size*nmemb,&num);
	return num;
}
void wav_rewind(void * stream)
{
	f_rewind((FIL *)stream);
}

/*-----------------------------------------------------------*/
void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
  static DMA_HandleTypeDef dma_rx_handle;
  static DMA_HandleTypeDef dma_tx_handle;
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable SDMMC2 clock */
  __HAL_RCC_SDMMC2_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  
  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_HIGH;

  /* GPIOB configuration */
  gpio_init_structure.Alternate = GPIO_AF10_SDMMC2;  
  gpio_init_structure.Pin = GPIO_PIN_3 | GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &gpio_init_structure);

  /* GPIOD configuration */
  gpio_init_structure.Alternate = GPIO_AF11_SDMMC2;  
  gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);
  
  /* GPIOG configuration */ 
  gpio_init_structure.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);
  
  /* NVIC configuration for SDMMC2 interrupts */
  HAL_NVIC_SetPriority(SDMMC2_IRQn, 0x0E, 0);
  HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
}

#endif

 

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
 
#ifdef EXPORT_WAV
  f_mount(&fs, "", 0);
  fr = f_open(&fil, "logfile.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if(fr != FR_OK) {
    for(;;) {
      avs_core_task_delay(500);
    }
  }
  f_printf(&fil, "%s\t%s\n", __DATE__, __TIME__);
  f_close(&fil);
  wav_writer_t *wav_writer = wav_writer_open("record.wav", 1, 16000);
#endif

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

 #ifdef EXPORT_WAV
        wav_writer_write_int16(wav_writer, blkSize, kws_handle->audio_buffer + kws_handle->write_pos);
#endif
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
