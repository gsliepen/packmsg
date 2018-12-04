#pragma once

/*
    SPDX-License-Identifier: BSD-3-Clause

    packmsg.h -- Little-endian MessagePack implementation, optimized for speed
    Copyright (C) 2018 Guus Sliepen <guus@tinc-vpn.org>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the University nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
    DAMAGE.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Buffer iterators */

struct packmsg_output {
	uint8_t *ptr;
	ptrdiff_t len;
};

struct packmsg_input {
	const uint8_t *ptr;
	ptrdiff_t len;
};

/* Checks */

static inline bool packmsg_output_ok(struct packmsg_output *buf)
{
	assert(buf);

	return likely(buf->len >= 0);
}

static inline size_t packmsg_output_size(struct packmsg_output *buf, uint8_t * start)
{
	if (likely(packmsg_output_ok(buf)))
		return buf->ptr - start;
	else
		return 0;
}

static inline bool packmsg_input_ok(struct packmsg_input *buf)
{
	assert(buf);

	return likely(buf->len >= 0);
}

static inline bool packmsg_done(struct packmsg_input *buf)
{
	assert(buf);

	return buf->len == 0;
}

/* Encoding functions */

static inline void packmsg_write_hdr_(struct packmsg_output *buf, uint8_t hdr)
{
	assert(buf);
	assert(buf->ptr);

	if (likely(buf->len > 0)) {
		*buf->ptr = hdr;
		buf->ptr++;
		buf->len--;
	} else {
		buf->len = -1;
	}
}

static inline void packmsg_write_data_(struct packmsg_output *buf, const void *data, uint32_t dlen)
{
	assert(buf);
	assert(buf->ptr);
	assert(data);

	if (likely(buf->len >= dlen)) {
		memcpy(buf->ptr, data, dlen);
		buf->ptr += dlen;
		buf->len -= dlen;
	} else {
		buf->len = -1;
	}
}

static inline void packmsg_write_hdrdata_(struct packmsg_output *buf, uint8_t hdr, const void *data, uint32_t dlen)
{
	assert(buf);
	assert(buf->ptr);
	assert(data);

	if (likely(buf->len > dlen)) {
		*buf->ptr = hdr;
		buf->ptr++;
		buf->len--;

		memcpy(buf->ptr, data, dlen);
		buf->ptr += dlen;
		buf->len -= dlen;
	} else {
		buf->len = -1;
	}
}

static inline void packmsg_add_nil(struct packmsg_output *buf)
{
	packmsg_write_hdr_(buf, 0xc0);
}

static inline void packmsg_add_bool(struct packmsg_output *buf, bool val)
{
	packmsg_write_hdr_(buf, val ? 0xc3 : 0xc2);
}

static inline void packmsg_add_int8(struct packmsg_output *buf, int8_t val)
{
	if (val >= -32)		// fixint
		packmsg_write_hdr_(buf, val);
	else			// TODO: negative fixint
		packmsg_write_hdrdata_(buf, 0xd0, &val, 1);
}

static inline void packmsg_add_int16(struct packmsg_output *buf, int16_t val)
{
	if ((int8_t) val != val)
		packmsg_write_hdrdata_(buf, 0xd1, &val, 2);
	else
		packmsg_add_int8(buf, val);
}

static inline void packmsg_add_int32(struct packmsg_output *buf, int32_t val)
{
	if ((int16_t) val != val)
		packmsg_write_hdrdata_(buf, 0xd2, &val, 4);
	else
		packmsg_add_int16(buf, val);
}

static inline void packmsg_add_int64(struct packmsg_output *buf, int64_t val)
{
	if ((int32_t) val != val)
		packmsg_write_hdrdata_(buf, 0xd3, &val, 8);
	else
		packmsg_add_int32(buf, val);
}

static inline void packmsg_add_uint8(struct packmsg_output *buf, uint8_t val)
{
	if (val < 0x80)		// fixint
		packmsg_write_hdr_(buf, val);
	else
		packmsg_write_hdrdata_(buf, 0xcc, &val, 1);
}

static inline void packmsg_add_uint16(struct packmsg_output *buf, uint16_t val)
{
	if (val & 0xff00)
		packmsg_write_hdrdata_(buf, 0xcd, &val, 2);
	else
		packmsg_add_uint8(buf, val);
}

static inline void packmsg_add_uint32(struct packmsg_output *buf, uint32_t val)
{
	if (val & 0xffff0000)
		packmsg_write_hdrdata_(buf, 0xce, &val, 4);
	else
		packmsg_add_uint16(buf, val);
}

static inline void packmsg_add_uint64(struct packmsg_output *buf, uint64_t val)
{
	if (val & 0xffffffff00000000)
		packmsg_write_hdrdata_(buf, 0xcf, &val, 8);
	else
		packmsg_add_uint32(buf, val);
}

static inline void packmsg_add_float(struct packmsg_output *buf, float val)
{
	packmsg_write_hdrdata_(buf, 0xca, &val, 4);
}

static inline void packmsg_add_double(struct packmsg_output *buf, double val)
{
	packmsg_write_hdrdata_(buf, 0xcb, &val, 8);
}

static inline void packmsg_add_str(struct packmsg_output *buf, const char *str)
{
	size_t slen = strlen(str);
	if (slen < 32) {
		packmsg_write_hdr_(buf, 0xa0 | (uint8_t) slen);
	} else if (slen <= 0xff) {
		packmsg_write_hdrdata_(buf, 0xd9, &slen, 1);
	} else if (slen <= 0xffff) {
		packmsg_write_hdrdata_(buf, 0xda, &slen, 2);
	} else if (slen <= 0xffffffff) {
		packmsg_write_hdrdata_(buf, 0xdb, &slen, 4);
	} else {
		buf->len = -1;
		return;
	}
	packmsg_write_data_(buf, str, slen);
}

static inline void packmsg_add_bin(struct packmsg_output *buf, const void *data, uint32_t dlen)
{
	if (dlen <= 0xff) {
		packmsg_write_hdrdata_(buf, 0xc4, &dlen, 1);
	} else if (dlen <= 0xffff) {
		packmsg_write_hdrdata_(buf, 0xc5, &dlen, 2);
	} else if (dlen <= 0xffffffff) {
		packmsg_write_hdrdata_(buf, 0xc6, &dlen, 4);
	} else {
		buf->len = -1;
		return;
	}
	packmsg_write_data_(buf, data, dlen);
}

static inline void packmsg_add_ext(struct packmsg_output *buf, int8_t type, const void *data, uint32_t dlen)
{
	if (dlen <= 0xff) {
		packmsg_write_hdrdata_(buf, 0xc7, &dlen, 1);
		packmsg_write_data_(buf, &type, 1);
	} else if (dlen <= 0xffff) {
		packmsg_write_hdrdata_(buf, 0xc8, &dlen, 2);
		packmsg_write_data_(buf, &type, 1);
	} else if (dlen <= 0xffffffff) {
		packmsg_write_hdrdata_(buf, 0xc9, &dlen, 4);
		packmsg_write_data_(buf, &type, 1);
	} else {
		buf->len = -1;
		return;
	}
	packmsg_write_data_(buf, data, dlen);
}

static inline void packmsg_add_fixext1(struct packmsg_output *buf, int8_t type, const void *data)
{
	packmsg_write_hdrdata_(buf, 0xd4, &type, 1);
	packmsg_write_data_(buf, data, 1);
}

static inline void packmsg_add_fixext2(struct packmsg_output *buf, int8_t type, const void *data)
{
	packmsg_write_hdrdata_(buf, 0xd5, &type, 1);
	packmsg_write_data_(buf, data, 2);
}

static inline void packmsg_add_fixext4(struct packmsg_output *buf, int8_t type, const void *data)
{
	packmsg_write_hdrdata_(buf, 0xd6, &type, 1);
	packmsg_write_data_(buf, data, 4);
}

static inline void packmsg_add_fixext8(struct packmsg_output *buf, int8_t type, const void *data)
{
	packmsg_write_hdrdata_(buf, 0xd7, &type, 1);
	packmsg_write_data_(buf, data, 8);
}

static inline void packmsg_add_fixext16(struct packmsg_output *buf, int8_t type, const void *data)
{
	packmsg_write_hdrdata_(buf, 0xd8, &type, 1);
	packmsg_write_data_(buf, data, 16);
}

static inline void packmsg_add_map(struct packmsg_output *buf, uint32_t dlen)
{
	if (dlen <= 0xf) {
		packmsg_write_hdr_(buf, 0x80 | (uint8_t) dlen);
	} else if (dlen <= 0xffff) {
		packmsg_write_hdrdata_(buf, 0xde, &dlen, 2);
	} else {
		packmsg_write_hdrdata_(buf, 0xdf, &dlen, 4);
	}
}

static inline void packmsg_add_array(struct packmsg_output *buf, uint32_t dlen)
{
	if (dlen <= 0xf) {
		packmsg_write_hdr_(buf, 0x90 | (uint8_t) dlen);
	} else if (dlen <= 0xffff) {
		packmsg_write_hdrdata_(buf, 0xdc, &dlen, 2);
	} else {
		packmsg_write_hdrdata_(buf, 0xdd, &dlen, 4);
	}
}

/* Decoding functions */

static inline uint8_t packmsg_read_hdr_(struct packmsg_input *buf)
{
	assert(buf);
	assert(buf->ptr);

	if (likely(buf->len > 0)) {
		uint8_t hdr = *buf->ptr;
		buf->ptr++;
		buf->len--;
		return hdr;
	} else {
		buf->len = -1;
		return 0xc1;
	}
}

static inline void packmsg_read_data_(struct packmsg_input *buf, void *data, uint32_t dlen)
{
	assert(buf);
	assert(buf->ptr);
	assert(data);

	if (likely(buf->len >= dlen)) {
		memcpy(data, buf->ptr, dlen);
		buf->ptr += dlen;
		buf->len -= dlen;
	} else {
		buf->len = -1;
	}
}

static inline uint8_t packmsg_peek_hdr_(struct packmsg_input *buf)
{
	assert(buf);
	assert(buf->ptr);

	if (likely(buf->len > 0)) {
		return *buf->ptr;
	} else {
		return 0xc1;
	}
}

static inline void packmsg_get_nil(struct packmsg_input *buf)
{
	if (packmsg_read_hdr_(buf) != 0xc0)
		buf->len = -1;
}

static inline bool packmsg_get_bool(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);

	if (hdr == 0xc2) {
		return false;
	} else if (hdr == 0xc3) {
		return true;
	} else {
		buf->len = -1;
		return false;
	}
}

static inline int8_t packmsg_get_int8(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80 || hdr >= 0xe0) {
		return (int8_t)hdr;
	} else if (hdr == 0xd0) {
		return packmsg_read_hdr_(buf);
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline int16_t packmsg_get_int16(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80 || hdr >= 0xe0) {
		return (int8_t)hdr;
	} else if (hdr == 0xd0) {
		return (int8_t) packmsg_read_hdr_(buf);
	} else if (hdr == 0xd1) {
		int16_t val = 0;
		packmsg_read_data_(buf, &val, 2);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline int32_t packmsg_get_int32(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80 || hdr >= 0xe0) {
		return (int8_t)hdr;
	} else if (hdr == 0xd0) {
		return (int8_t) packmsg_read_hdr_(buf);
	} else if (hdr == 0xd1) {
		int16_t val = 0;
		packmsg_read_data_(buf, &val, 2);
		return val;
	} else if (hdr == 0xd2) {
		int32_t val = 0;
		packmsg_read_data_(buf, &val, 4);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline int64_t packmsg_get_int64(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80 || hdr >= 0xe0) {
		return (int8_t)hdr;
	} else if (hdr == 0xd0) {
		return (int8_t) packmsg_read_hdr_(buf);
	} else if (hdr == 0xd1) {
		int16_t val = 0;
		packmsg_read_data_(buf, &val, 2);
		return val;
	} else if (hdr == 0xd2) {
		int32_t val = 0;
		packmsg_read_data_(buf, &val, 4);
		return val;
	} else if (hdr == 0xd3) {
		int64_t val = 0;
		packmsg_read_data_(buf, &val, 8);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline uint8_t packmsg_get_uint8(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80) {
		return hdr;
	} else if (hdr == 0xcc) {
		return packmsg_read_hdr_(buf);
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline uint16_t packmsg_get_uint16(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80) {
		return hdr;
	} else if (hdr == 0xcc) {
		return packmsg_read_hdr_(buf);
	} else if (hdr == 0xcd) {
		uint16_t val = 0;
		packmsg_read_data_(buf, &val, 2);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline uint32_t packmsg_get_uint32(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80) {
		return hdr;
	} else if (hdr == 0xcc) {
		return packmsg_read_hdr_(buf);
	} else if (hdr == 0xcd) {
		uint16_t val = 0;
		packmsg_read_data_(buf, &val, 2);
		return val;
	} else if (hdr == 0xce) {
		uint32_t val = 0;
		packmsg_read_data_(buf, &val, 4);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline uint64_t packmsg_get_uint64(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr < 0x80) {
		return hdr;
	} else if (hdr == 0xcc) {
		return packmsg_read_hdr_(buf);
	} else if (hdr == 0xcd) {
		uint16_t val = 0;
		packmsg_read_data_(buf, &val, 2);
		return val;
	} else if (hdr == 0xce) {
		uint32_t val = 0;
		packmsg_read_data_(buf, &val, 4);
		return val;
	} else if (hdr == 0xcf) {
		uint64_t val = 0;
		packmsg_read_data_(buf, &val, 8);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline float packmsg_get_float(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr == 0xca) {
		float val;
		packmsg_read_data_(buf, &val, 4);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline double packmsg_get_double(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if (hdr == 0xcb) {
		double val;
		packmsg_read_data_(buf, &val, 8);
		return val;
	} else if (hdr == 0xca) {
		float val;
		packmsg_read_data_(buf, &val, 4);
		return val;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline uint32_t packmsg_get_str_raw(struct packmsg_input *buf, const char **str)
{
	assert(str);

	uint8_t hdr = packmsg_read_hdr_(buf);
	uint32_t slen = 0;

	if ((hdr & 0xe0) == 0xa0) {
		slen = hdr & 0x1f;
	} else if (hdr == 0xd9) {
		packmsg_read_data_(buf, &slen, 1);
	} else if (hdr == 0xda) {
		packmsg_read_data_(buf, &slen, 2);
	} else if (hdr == 0xdb) {
		packmsg_read_data_(buf, &slen, 4);
	} else {
		buf->len = -1;
		*str = NULL;
		return 0;
	}

	if (likely(buf->len >= slen)) {
		*str = (const char *)buf->ptr;
		buf->ptr += slen;
		buf->len -= slen;
		return slen;
	} else {
		buf->len = -1;
		*str = NULL;
		return 0;
	}
}

static inline char *packmsg_get_str_dup(struct packmsg_input *buf)
{
	const char *str;
	uint32_t slen = packmsg_get_str_raw(buf, &str);
	if (likely(packmsg_input_ok(buf))) {
		char *dup = (char *)malloc((size_t) slen + 1);
		if (likely(dup)) {
			memcpy(dup, str, slen);
			dup[slen] = 0;
			return dup;
		} else {
			buf->len = -1;
			return NULL;
		}
	} else {
		return NULL;
	}
}

static inline uint32_t packmsg_get_str_copy(struct packmsg_input *buf, void *data, uint32_t dlen)
{
	assert(data);

	const char *str;
	uint32_t slen = packmsg_get_str_raw(buf, &str);
	if (likely(packmsg_input_ok(buf))) {
		if (likely(slen < dlen)) {
			memcpy(data, str, slen);
			((char *)data)[slen] = 0;
			return slen;
		} else {
			if (dlen)
				*(char *)data = 0;
			buf->len = -1;
			return 0;
		}
	} else {
		if (dlen)
			*(char *)data = 0;
		return 0;
	}
}

static inline uint32_t packmsg_get_bin_raw(struct packmsg_input *buf, const void **data)
{
	assert(data);

	uint8_t hdr = packmsg_read_hdr_(buf);
	uint32_t dlen = 0;

	if (hdr == 0xc4) {
		packmsg_read_data_(buf, &dlen, 1);
	} else if (hdr == 0xc5) {
		packmsg_read_data_(buf, &dlen, 2);
	} else if (hdr == 0xc6) {
		packmsg_read_data_(buf, &dlen, 4);
	} else {
		buf->len = -1;
		*data = NULL;
		return 0;
	}

	if (likely(buf->len >= dlen)) {
		*data = buf->ptr;
		buf->ptr += dlen;
		buf->len -= dlen;
		return dlen;
	} else {
		buf->len = -1;
		*data = NULL;
		return 0;
	}
}

static inline void *packmsg_get_bin_dup(struct packmsg_input *buf)
{
	const void *data;
	uint32_t dlen = packmsg_get_bin_raw(buf, &data);
	if (likely(packmsg_input_ok(buf))) {
		char *dup = (char *)malloc(dlen);
		if (likely(dup)) {
			memcpy(dup, data, dlen);
			return dup;
		} else {
			buf->len = -1;
			return NULL;
		}
	} else {
		return NULL;
	}
}

static inline uint32_t packmsg_get_bin_copy(struct packmsg_input *buf, void *rawbuf, uint32_t rlen)
{
	assert(rawbuf);

	const void *data;
	uint32_t dlen = packmsg_get_bin_raw(buf, &data);
	if (likely(packmsg_input_ok(buf))) {
		if (likely(dlen <= rlen)) {
			memcpy(rawbuf, data, dlen);
			return dlen;
		} else {
			buf->len = -1;
			return 0;
		}
	} else {
		return 0;
	}
}

static inline uint32_t packmsg_get_ext_raw(struct packmsg_input *buf, int8_t *type, const void **data)
{
	assert(type);
	assert(data);

	uint8_t hdr = packmsg_read_hdr_(buf);
	uint32_t dlen = 0;

	if (hdr == 0xc7) {
		packmsg_read_data_(buf, &dlen, 1);
	} else if (hdr == 0xc8) {
		packmsg_read_data_(buf, &dlen, 2);
	} else if (hdr == 0xc9) {
		packmsg_read_data_(buf, &dlen, 4);
	} else {
		buf->len = -1;
		*type = 0;
		*data = NULL;
		return 0;
	}

	*type = packmsg_read_hdr_(buf);

	if (likely(buf->len >= dlen)) {
		*data = buf->ptr;
		buf->ptr += dlen;
		buf->len -= dlen;
		return dlen;
	} else {
		buf->len = -1;
		*type = 0;
		*data = NULL;
		return 0;
	}
}

static inline void *packmsg_get_ext_dup(struct packmsg_input *buf, int8_t * type)
{
	assert(type);

	const void *data;
	uint32_t dlen = packmsg_get_ext_raw(buf, type, &data);
	if (likely(packmsg_input_ok(buf))) {
		char *dup = (char *)malloc(dlen);
		if (likely(dup)) {
			memcpy(dup, data, dlen);
			return dup;
		} else {
			*type = 0;
			buf->len = -1;
			return NULL;
		}
	} else {
		return NULL;
	}
}

static inline uint32_t packmsg_get_ext_copy(struct packmsg_input *buf, int8_t *type, void *rawbuf, uint32_t rlen)
{
	assert(type);
	assert(rawbuf);

	const void *data;
	uint32_t dlen = packmsg_get_ext_raw(buf, type, &data);
	if (likely(packmsg_input_ok(buf))) {
		if (likely(dlen <= rlen)) {
			memcpy(rawbuf, data, dlen);
			return dlen;
		} else {
			*type = 0;
			buf->len = -1;
			return 0;
		}
	} else {
		return 0;
	}
}

static inline int8_t packmsg_get_fixext1(struct packmsg_input *buf, void *data)
{
	assert(data);

	if (packmsg_read_hdr_(buf) != 0xd4) {
		buf->len = -1;
		return 0;
	}

	int8_t ext = packmsg_read_hdr_(buf);
	packmsg_read_data_(buf, data, 1);
	return likely(packmsg_input_ok(buf)) ? ext : 0;
}

static inline int8_t packmsg_get_fixext2(struct packmsg_input *buf, void *data)
{
	assert(data);

	if (packmsg_read_hdr_(buf) != 0xd5) {
		buf->len = -1;
		return 0;
	}

	int8_t ext = packmsg_read_hdr_(buf);
	packmsg_read_data_(buf, data, 2);
	return likely(packmsg_input_ok(buf)) ? ext : 0;
}

static inline int8_t packmsg_get_fixext4(struct packmsg_input *buf, void *data)
{
	assert(data);

	if (packmsg_read_hdr_(buf) != 0xd6) {
		buf->len = -1;
		return 0;
	}

	int8_t ext = packmsg_read_hdr_(buf);
	packmsg_read_data_(buf, data, 4);
	return likely(packmsg_input_ok(buf)) ? ext : 0;
}

static inline int8_t packmsg_get_fixext8(struct packmsg_input *buf, void *data)
{
	assert(data);

	if (packmsg_read_hdr_(buf) != 0xd7) {
		buf->len = -1;
		return 0;
	}

	int8_t ext = packmsg_read_hdr_(buf);
	packmsg_read_data_(buf, data, 8);
	return likely(packmsg_input_ok(buf)) ? ext : 0;
}

static inline int8_t packmsg_get_fixext16(struct packmsg_input *buf, void *data)
{
	assert(data);

	if (packmsg_read_hdr_(buf) != 0xd8) {
		buf->len = -1;
		return 0;
	}

	int8_t ext = packmsg_read_hdr_(buf);
	packmsg_read_data_(buf, data, 16);
	return likely(packmsg_input_ok(buf)) ? ext : 0;
}

static inline uint32_t packmsg_get_map(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if ((hdr & 0xf0) == 0x80) {
		return hdr & 0xf;
	} else if (hdr == 0xde) {
		uint32_t dlen = 0;
		packmsg_read_data_(buf, &dlen, 2);
		return dlen;
	} else if (hdr == 0xdf) {
		uint32_t dlen = 0;
		packmsg_read_data_(buf, &dlen, 4);
		return dlen;
	} else {
		buf->len = -1;
		return 0;
	}
}

static inline uint32_t packmsg_get_array(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_read_hdr_(buf);
	if ((hdr & 0xf0) == 0x90) {
		return hdr & 0xf;
	} else if (hdr == 0xdc) {
		uint32_t dlen = 0;
		packmsg_read_data_(buf, &dlen, 2);
		return dlen;
	} else if (hdr == 0xdd) {
		uint32_t dlen = 0;
		packmsg_read_data_(buf, &dlen, 4);
		return dlen;
	} else {
		buf->len = -1;
		return 0;
	}
}

/* Type checking */

enum packmsg_type {
	PACKMSG_ERROR,
	PACKMSG_NIL,
	PACKMSG_BOOL,
	PACKMSG_POSITIVE_FIXINT,
	PACKMSG_NEGATIVE_FIXINT,
	PACKMSG_INT8,
	PACKMSG_INT16,
	PACKMSG_INT32,
	PACKMSG_INT64,
	PACKMSG_UINT8,
	PACKMSG_UINT16,
	PACKMSG_UINT32,
	PACKMSG_UINT64,
	PACKMSG_FLOAT,
	PACKMSG_DOUBLE,
	PACKMSG_STR,
	PACKMSG_BIN,
	PACKMSG_EXT,
	PACKMSG_FIXEXT1,
	PACKMSG_FIXEXT2,
	PACKMSG_FIXEXT4,
	PACKMSG_FIXEXT8,
	PACKMSG_FIXEXT16,
	PACKMSG_MAP,
	PACKMSG_ARRAY,
	PACKMSG_DONE,
};

static inline bool packmsg_is_nil(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xc0;
}

static inline bool packmsg_is_bool(struct packmsg_input *buf)
{
	return (packmsg_peek_hdr_(buf) & 0xfe) == 0xc2;
}

static inline bool packmsg_is_int8(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xd0;
}

static inline bool packmsg_is_int16(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xd0 || hdr == 0xd1;
}

static inline bool packmsg_is_int32(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xd0 || hdr == 0xd1 || hdr == 0xd2;
}

static inline bool packmsg_is_int64(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xd0 || hdr == 0xd1 || hdr == 0xd2 || hdr == 0xd3;
}

static inline bool packmsg_is_uint8(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xcc;
}

static inline bool packmsg_is_uint16(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xcc || hdr == 0xcd;
}

static inline bool packmsg_is_uint32(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xcc || hdr == 0xcd || hdr == 0xce;
}

static inline bool packmsg_is_uint64(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return hdr < 0x80 || hdr == 0xcc || hdr == 0xcd || hdr == 0xce || hdr == 0xcf;
}

static inline bool packmsg_is_float(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xca;
}

static inline bool packmsg_is_double(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xcb;
}

static inline bool packmsg_is_str(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return (hdr & 0xe0) == 0xa0 || hdr == 0xd9 || hdr == 0xda || hdr == 0xdb;
}

static inline bool packmsg_is_bin(struct packmsg_input *buf)
{
	return (packmsg_peek_hdr_(buf) & 0xfc) == 0xc4;
}

static inline bool packmsg_is_fixext1(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xd4;
}

static inline bool packmsg_is_fixext2(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xd5;
}

static inline bool packmsg_is_fixext4(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xd6;
}

static inline bool packmsg_is_fixext8(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xd7;
}

static inline bool packmsg_is_fixext16(struct packmsg_input *buf)
{
	return packmsg_peek_hdr_(buf) == 0xd8;
}

static inline bool packmsg_is_map(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return (hdr & 0xf0) == 0x80 || hdr == 0xde || hdr == 0xdf;
}

static inline bool packmsg_is_array(struct packmsg_input *buf)
{
	uint8_t hdr = packmsg_peek_hdr_(buf);
	return (hdr & 0xf0) == 0x90 || hdr == 0xdc || hdr == 0xdd;
}

static inline enum packmsg_type packmsg_get_type(struct packmsg_input *buf) {
	if (unlikely(packmsg_done(buf)))
		return PACKMSG_DONE;

	uint8_t hdr = packmsg_peek_hdr_(buf);

	switch (hdr >> 4) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7: return PACKMSG_POSITIVE_FIXINT;
	case 0x8: return PACKMSG_MAP;
	case 0x9: return PACKMSG_ARRAY;
	case 0xa:
	case 0xb: return PACKMSG_STR;
	case 0xc: switch (hdr & 0xf) {
		case 0x0: return PACKMSG_NIL;
		case 0x1: return PACKMSG_ERROR;
		case 0x2:
		case 0x3: return PACKMSG_BOOL;
		case 0x4:
		case 0x5:
		case 0x6: return PACKMSG_BIN;
		case 0x7:
		case 0x8:
		case 0x9: return PACKMSG_EXT;
		case 0xa: return PACKMSG_FLOAT;
		case 0xb: return PACKMSG_DOUBLE;
		case 0xc: return PACKMSG_UINT8;
		case 0xd: return PACKMSG_UINT16;
		case 0xe: return PACKMSG_UINT32;
		case 0xf: return PACKMSG_UINT64;
		default: return PACKMSG_ERROR;
		}
	case 0xd: switch (hdr & 0xf) {
		case 0x0: return PACKMSG_INT8;
		case 0x1: return PACKMSG_INT16;
		case 0x2: return PACKMSG_INT32;
		case 0x3: return PACKMSG_INT64;
		case 0x4: return PACKMSG_FIXEXT1;
		case 0x5: return PACKMSG_FIXEXT2;
		case 0x6: return PACKMSG_FIXEXT4;
		case 0x7: return PACKMSG_FIXEXT8;
		case 0x8: return PACKMSG_FIXEXT16;
		case 0x9:
		case 0xa:
		case 0xb: return PACKMSG_STR;
		case 0xc:
		case 0xd: return PACKMSG_ARRAY;
		case 0xe:
		case 0xf: return PACKMSG_MAP;
		default: return PACKMSG_ERROR;
		}
	case 0xe:
	case 0xf: return PACKMSG_NEGATIVE_FIXINT;
	default: return PACKMSG_ERROR;
	}
}

#undef likely
#undef unlikely

#ifdef __cplusplus
}
#endif
