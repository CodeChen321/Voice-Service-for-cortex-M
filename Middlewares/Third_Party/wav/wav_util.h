#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Write wav file utils */
typedef struct wav_writer_state {
    void * wav_file;
    int total_num_samples;
    int num_channels;
    int sampling_frequency;
    int frame_count;
} wav_writer_t;
typedef struct {
	uint32_t cksize;
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgByesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
	uint16_t wValidBitsPerSample;
	uint32_t dwChannelMask;
	uint8_t SubFormat[16];
}FMT_CHUNK;
typedef struct {
	int wav_reader_fd;
	FMT_CHUNK *fmtChunk;
	uint32_t data_chunk_size;
}wav_reader_t;
/**
 * Open singleton wav writer
 * @return wav_writer_t * if ok
 */
wav_writer_t * wav_writer_open(const char * filepath, int num_channels, int sampling_frequency);
/**
 * Write Int8 samples
 * @return 0 if ok
 */
int wav_writer_write_int8(wav_writer_t *wav_writer_state,int num_samples, int8_t * data);
/**
 * Write Int16 samples (host endianess)
 * @return 0 if ok
 */
int wav_writer_write_int16(wav_writer_t *wav_writer_state,int num_samples, int16_t * data);
/**
 * Write Little Endian Int16 samples
 * @return 0 if ok
 */
int wav_writer_write_le_int16(wav_writer_t *wav_writer_state,int num_samples, int16_t * data);
/**
 * Close wav writer and update wav file header
 * @return 0 if ok
 */
int wav_writer_close(wav_writer_t *wav_writer_state);


