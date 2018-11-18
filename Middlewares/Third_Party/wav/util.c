/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define __BTSTACK_FILE__ "btstack_util.c"

/*
 *  btstack_util.c
 *
 *  General utility functions
 *
 *  Created by Matthias Ringwald on 7/23/09.
 */
#include "util.h"

#include <stdio.h>
#include <string.h>

uint16_t little_endian_read_16(const uint8_t * buffer, int pos){
    return (uint16_t)(((uint16_t) buffer[pos]) | (((uint16_t)buffer[(pos)+1]) << 8));
}
uint32_t little_endian_read_24(const uint8_t * buffer, int pos){
    return ((uint32_t) buffer[pos]) | (((uint32_t)buffer[(pos)+1]) << 8) | (((uint32_t)buffer[(pos)+2]) << 16);
}
uint32_t little_endian_read_32(const uint8_t * buffer, int pos){
    return ((uint32_t) buffer[pos]) | (((uint32_t)buffer[(pos)+1]) << 8) | (((uint32_t)buffer[(pos)+2]) << 16) | (((uint32_t) buffer[(pos)+3]) << 24);
}

void little_endian_store_16(uint8_t *buffer, uint16_t pos, uint16_t value){
    buffer[pos++] = (uint8_t)value;
    buffer[pos++] = (uint8_t)(value >> 8);
}

void little_endian_store_32(uint8_t *buffer, uint16_t pos, uint32_t value){
    buffer[pos++] = (uint8_t)(value);
    buffer[pos++] = (uint8_t)(value >> 8);
    buffer[pos++] = (uint8_t)(value >> 16);
    buffer[pos++] = (uint8_t)(value >> 24);
}

uint32_t big_endian_read_16( const uint8_t * buffer, int pos) {
    return (uint16_t)(((uint16_t) buffer[(pos)+1]) | (((uint16_t)buffer[ pos   ]) << 8));
}

uint32_t big_endian_read_24( const uint8_t * buffer, int pos) {
    return ( ((uint32_t)buffer[(pos)+2]) | (((uint32_t)buffer[(pos)+1]) << 8) | (((uint32_t) buffer[pos]) << 16));
}

uint32_t big_endian_read_32( const uint8_t * buffer, int pos) {
    return ((uint32_t) buffer[(pos)+3]) | (((uint32_t)buffer[(pos)+2]) << 8) | (((uint32_t)buffer[(pos)+1]) << 16) | (((uint32_t) buffer[pos]) << 24);
}

void big_endian_store_16(uint8_t *buffer, uint16_t pos, uint16_t value){
    buffer[pos++] = (uint8_t)(value >> 8);
    buffer[pos++] = (uint8_t)(value);
}

void big_endian_store_24(uint8_t *buffer, uint16_t pos, uint32_t value){
    buffer[pos++] = (uint8_t)(value >> 16);
    buffer[pos++] = (uint8_t)(value >> 8);
    buffer[pos++] = (uint8_t)(value);
}

void big_endian_store_32(uint8_t *buffer, uint16_t pos, uint32_t value){
    buffer[pos++] = (uint8_t)(value >> 24);
    buffer[pos++] = (uint8_t)(value >> 16);
    buffer[pos++] = (uint8_t)(value >> 8);
    buffer[pos++] = (uint8_t)(value);
}

// general swap/endianess utils
void reverse_bytes(const uint8_t *src, uint8_t *dst, int len){
    int i;
    for (i = 0; i < len; i++)
        dst[len - 1 - i] = src[i];
}
void reverse_24(const uint8_t * src, uint8_t * dst){
    reverse_bytes(src, dst, 3);
}
void reverse_48(const uint8_t * src, uint8_t * dst){
    reverse_bytes(src, dst, 6);
}
void reverse_56(const uint8_t * src, uint8_t * dst){
    reverse_bytes(src, dst, 7);
}
void reverse_64(const uint8_t * src, uint8_t * dst){
    reverse_bytes(src, dst, 8);
}
void reverse_128(const uint8_t * src, uint8_t * dst){
    reverse_bytes(src, dst, 16);
}
void reverse_256(const uint8_t * src, uint8_t * dst){
    reverse_bytes(src, dst, 32);
}
uint32_t btstack_min(uint32_t a, uint32_t b){
    return a < b ? a : b;
}

uint32_t btstack_max(uint32_t a, uint32_t b){
    return a > b ? a : b;
}

char char_for_nibble(int nibble){
    if (nibble < 10) return (char)('0' + nibble);
    nibble -= 10;
    if (nibble < 6) return (char)('A' + nibble);
    return '?';
}

static char char_for_high_nibble(int value){
    return char_for_nibble((value >> 4) & 0x0f);
}

static char char_for_low_nibble(int value){
    return char_for_nibble(value & 0x0f);
}

int nibble_for_char(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void printf_hexdump(const void *data, int size){
    if (size <= 0) return;
    int i;
    for (i=0; i<size;i++){
        printf("%02X ", ((uint8_t *)data)[i]);
    }
    printf("\n");
}

#if defined(ENABLE_LOG_INFO) || defined(ENABLE_LOG_DEBUG)
static void log_hexdump(int level, const void * data, int size){
#define ITEMS_PER_LINE 16
// template '0x12, '
#define BYTES_PER_BYTE  6
    char buffer[BYTES_PER_BYTE*ITEMS_PER_LINE+1];
    int i, j;
    j = 0;
    for (i=0; i<size;i++){

        // help static analyzer proof that j stays within bounds
        if (j > BYTES_PER_BYTE * (ITEMS_PER_LINE-1)){
            j = 0;
        }

        uint8_t byte = ((uint8_t *)data)[i];
        buffer[j++] = '0';
        buffer[j++] = 'x';
        buffer[j++] = char_for_high_nibble(byte);
        buffer[j++] = char_for_low_nibble(byte);
        buffer[j++] = ',';
        buffer[j++] = ' ';     

        if (j >= BYTES_PER_BYTE * ITEMS_PER_LINE ){
            buffer[j] = 0;
            HCI_DUMP_LOG(level, "%s", buffer);
            j = 0;
        }
    }
    if (j != 0){
        buffer[j] = 0;
        HCI_DUMP_LOG(level, "%s", buffer);
    }
}
#endif
static int scan_hex_byte(const char * byte_string){
    int upper_nibble = nibble_for_char(*byte_string++);
    if (upper_nibble < 0) return -1;
    int lower_nibble = nibble_for_char(*byte_string);
    if (lower_nibble < 0) return -1;
    return (upper_nibble << 4) | lower_nibble;
}

uint32_t btstack_atoi(const char *str){
    uint32_t val = 0;
    while (1){
        char chr = *str;
        if (!chr || chr < '0' || chr > '9')
            return val;
        val = (val * 10) + (uint8_t)(chr - '0');
        str++;
    }
}
