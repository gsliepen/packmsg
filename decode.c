/*
    decode.c -- PackMessage decoder example
    Copyright (C) 2018 Guus Sliepen <guus@tinc-vpn.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "packmsg.h"

static void decode_something(struct packmsg_input *in);

static void decode_scalar(struct packmsg_input *in) {
	switch (packmsg_get_type(in)) {
	case PACKMSG_ERROR:
	case PACKMSG_DONE:
	case PACKMSG_ARRAY:
	case PACKMSG_MAP:
		// Get something to ensure we get into an invalid state.
		packmsg_get_float(in);
		return;
	case PACKMSG_NIL:
		packmsg_get_nil(in);
		printf("<nil>");
		return;
	case PACKMSG_BOOL:
		printf(packmsg_get_bool(in) ? "true" : "false");
		return;
	case PACKMSG_POSITIVE_FIXINT:
	case PACKMSG_NEGATIVE_FIXINT:
	case PACKMSG_INT8:
	case PACKMSG_INT16:
	case PACKMSG_INT32:
	case PACKMSG_INT64:
		printf("%ld", packmsg_get_int64(in));
		break;
	case PACKMSG_UINT8:
	case PACKMSG_UINT16:
	case PACKMSG_UINT32:
	case PACKMSG_UINT64:
		printf("%ld", packmsg_get_uint64(in));
		break;
	case PACKMSG_FLOAT:
		printf("%f", packmsg_get_float(in));
		break;
	case PACKMSG_DOUBLE:
		printf("%f", packmsg_get_double(in));
		break;
	case PACKMSG_STR: {
		const char *str;
		uint32_t size = packmsg_get_str_raw(in, &str);
		printf("\"%.*s\"", size, str);
		break;
	}
	case PACKMSG_BIN: {
		const void *bin;
		uint32_t size = packmsg_get_bin_raw(in, &bin);
		printf("<%u bytes binary>", size);
		break;
	}
	case PACKMSG_EXT: {
		const void *bin;
		int8_t ext;
		uint32_t size = packmsg_get_ext_raw(in, &ext, &bin);
		printf("<%u bytes extension %d>", size, ext);
	}
	}
}

static void decode_array(struct packmsg_input *in) {
	uint32_t count = packmsg_get_array(in);
	printf("{");
	for (uint32_t i = 0; i < count && packmsg_input_ok(in); i++) {
		if (i)
			printf(", ");
		decode_something(in);
	}
	printf("}");
}

static void decode_map(struct packmsg_input *in) {
	uint32_t count = packmsg_get_map(in);
	printf("{");
	for (uint32_t i = 0; i < count && packmsg_input_ok(in); i++) {
		if (i)
			printf(", ");
		decode_something(in);
		printf(": ");
		decode_something(in);
	}
	printf("}");
}

static void decode_something(struct packmsg_input *in) {
	if (packmsg_is_map(in))
		decode_map(in);
	else if (packmsg_is_array(in))
		decode_array(in);
	else
		decode_scalar(in);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
		return 1;
	}

	FILE *f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "Could not open %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	long size;

	if (fseek(f, 0, SEEK_END) || (size = ftell(f)) == -1 || fseek(f, 0, SEEK_SET)) {
		fprintf(stderr, "Could not seek in %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	uint8_t *buf = (uint8_t *)malloc(size);

	if (!buf) {
		fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
		fclose(f);
		return 1;
	}

	if (fread(buf, size, 1, f) != 1) {
		fprintf(stderr, "Could not read %s: %s\n", argv[1], strerror(errno));
		free(buf);
		fclose(f);
		return 1;
	}

	fclose(f);

	struct packmsg_input in = {buf, size};

	while (!packmsg_done(&in)) {
		if (!packmsg_input_ok(&in)) {
			fprintf(stderr, "Error parsing %s\n", argv[1]);
			return 1;
		}

		decode_something(&in);
	}

	printf("\n");

	free(buf);
}
