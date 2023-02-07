/* Copyright (C)2012 Xiph.Org Foundation
   File: opus_header.h

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OPUS_HEADER_H
#define OPUS_HEADER_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  int version;
  int channels; /* Number of channels: 1..255 */
  int preskip;
  uint32_t input_sample_rate;
  int32_t gain; /* in dB S7.8 should be zero whenever possible */
  int channel_mapping;
  /* The rest is only used if channel_mapping != 0 */
  int nb_streams;
  int nb_coupled;
  unsigned char stream_map[255];
} OpusHeader;

int opeint_opus_header_get_size(const OpusHeader *h);

int opeint_opus_header_to_packet(const OpusHeader *h, unsigned char *packet, int len);

void opeint_comment_init(char **comments, int* length, const char *vendor_string);

int opeint_comment_add(char **comments, int* length, const char *tag, const char *val);

void opeint_comment_pad(char **comments, int* length, int amount);

#endif
