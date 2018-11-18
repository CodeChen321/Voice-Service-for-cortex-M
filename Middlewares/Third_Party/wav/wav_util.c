#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "wav_util.h"
__weak void * wav_open(const char *filename,
                    const char * mode)
{
	
}
__weak int wav_close(void * stream)
{
	
}
__weak size_t wav_write(void * ptr,
                    size_t size, size_t nmemb, void * stream)
{
	
}
__weak void wav_rewind(void * stream)
{

}
static void little_endian_fstore_16(void *wav_file, uint16_t value){
    uint8_t buf[2];
    little_endian_store_16(buf, 0, value);
    wav_write(&buf, 1, 2, wav_file);
}

static void little_endian_fstore_32(void *wav_file, uint32_t value){
    uint8_t buf[4];
    little_endian_store_32(buf, 0, value);
    wav_write(&buf, 1, 4, wav_file);
}


static void write_wav_header(void * wav_file,  int total_num_samples, int num_channels, int sample_rate){
    unsigned int write_with_bytes_per_sample = 2;
    /* write RIFF header */
    wav_write("RIFF", 1, 4, wav_file);
    // num_samples = blocks * subbands
    uint32_t data_bytes = (uint32_t) (write_with_bytes_per_sample * total_num_samples);//already computed * num_channels);
    little_endian_fstore_32(wav_file, data_bytes + 36); 
    wav_write("WAVE", 1, 4, wav_file);

    int byte_rate = sample_rate * num_channels * write_with_bytes_per_sample;
    int bits_per_sample = 8 * write_with_bytes_per_sample;
    int block_align = num_channels * bits_per_sample;
    int fmt_length = 16;
    int fmt_format_tag = 1; // PCM

    /* write fmt chunk */
    wav_write("fmt ", 1, 4, wav_file);
    little_endian_fstore_32(wav_file, fmt_length);
    little_endian_fstore_16(wav_file, fmt_format_tag);
    little_endian_fstore_16(wav_file, num_channels);
    little_endian_fstore_32(wav_file, sample_rate);
    little_endian_fstore_32(wav_file, byte_rate);
    little_endian_fstore_16(wav_file, block_align);   
    little_endian_fstore_16(wav_file, bits_per_sample);
    
    /* write data chunk */
    wav_write("data", 1, 4, wav_file); 
    little_endian_fstore_32(wav_file, data_bytes);
}

wav_writer_t * wav_writer_open(const char * filepath, int num_channels, int sampling_frequency){
    void * wav_file = wav_open(filepath, "wb");
    if (!wav_file) return NULL;
    wav_writer_t *wav_writer=malloc(sizeof(wav_writer_t));
    wav_writer->wav_file = wav_file;
    wav_writer->frame_count = 0;
    wav_writer->total_num_samples = 0;
    wav_writer->num_channels = num_channels;
    wav_writer->sampling_frequency = sampling_frequency;
    write_wav_header(wav_writer->wav_file, 0, num_channels, sampling_frequency);
    return wav_writer;
}

int wav_writer_close(wav_writer_t *wav_writer){
    wav_rewind(wav_writer->wav_file);
    write_wav_header(wav_writer->wav_file, wav_writer->total_num_samples,
    wav_writer->num_channels, wav_writer->sampling_frequency);
    wav_close(wav_writer->wav_file);
    free(wav_writer);
    return 0;
}

int wav_writer_write_int8(wav_writer_t *wav_writer,int num_samples, int8_t * data){
    if (data == NULL) return 1;
    int i = 0;
    int8_t zero_byte = 0;
    for (i=0; i<num_samples; i++){
        wav_write(&zero_byte, 1, 1, wav_writer->wav_file);
        uint8_t byte_value = (uint8_t)data[i];
        wav_write(&byte_value, 1, 1, wav_writer->wav_file);
    }
    
    wav_writer->total_num_samples+=num_samples;
    wav_writer->frame_count++;
    return 0;
}

int wav_writer_write_le_int16(wav_writer_t *wav_writer,int num_samples, int16_t * data){
    if (data == NULL) return 1;
    wav_write(data, num_samples, 2, wav_writer->wav_file);
    
    wav_writer->total_num_samples+=num_samples;
    wav_writer->frame_count++;
    return 0;
}

int wav_writer_write_int16(wav_writer_t *wav_writer,int num_samples, int16_t * data){
    if (btstack_is_little_endian()){
        return wav_writer_write_le_int16(wav_writer,num_samples, data);
    }
    if (data == NULL) return 1;

    int i;
    for (i=0;i<num_samples;i++){
        uint16_t sample = btstack_flip_16(data[i]);
        wav_write(&sample, 1, 2, wav_writer->wav_file);
    }    

    wav_writer->total_num_samples+=num_samples;
    wav_writer->frame_count++;
    return 0;
}
