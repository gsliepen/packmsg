#include <stdio.h>
#include <check.h>
#include <limits.h>
#include <math.h>

#include "packmsg.h"

#define TEST_OUTPUT(statement, expected, size) {\
	uint8_t buf[size + 64];\
	memcpy(buf + size, "Canary!", 8);\
	struct packmsg_output out = {buf, size};\
	statement;\
	ck_assert(packmsg_output_ok(&out));\
	ck_assert_int_eq(packmsg_output_size(&out, buf), size);\
	ck_assert_mem_eq(buf, expected, size);\
	ck_assert_mem_eq(buf + size, "Canary!", 8);\
	memcpy(buf + size - 1, "Canary!", 8);\
	out.ptr = buf;\
	out.len = size - 1;\
	statement;\
	ck_assert(!packmsg_output_ok(&out));\
	ck_assert_int_eq(packmsg_output_size(&out, buf), 0);\
	ck_assert_mem_eq(buf + size - 1, "Canary!", 8);\
}

START_TEST(add_nil)
{
	TEST_OUTPUT(packmsg_add_nil(&out), "\xc0", 1);
}
END_TEST

START_TEST(add_bool)
{
	TEST_OUTPUT(packmsg_add_bool(&out, true), "\xc3", 1);
	TEST_OUTPUT(packmsg_add_bool(&out, false), "\xc2", 1);
}
END_TEST

START_TEST(add_int8)
{
	TEST_OUTPUT(packmsg_add_int8(&out,        0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_int8(&out, INT8_MAX), "\x7f", 1);

	TEST_OUTPUT(packmsg_add_int8(&out,       -1), "\xff", 1);
	TEST_OUTPUT(packmsg_add_int8(&out,      -32), "\xe0", 1);
	TEST_OUTPUT(packmsg_add_int8(&out,      -33), "\xd0\xdf", 2);
	TEST_OUTPUT(packmsg_add_int8(&out, INT8_MIN), "\xd0\x80", 2);
}
END_TEST

START_TEST(add_int16)
{
	TEST_OUTPUT(packmsg_add_int16(&out,             0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_int16(&out,      INT8_MAX), "\x7f", 1);
	TEST_OUTPUT(packmsg_add_int16(&out,  1 + INT8_MAX), "\xd1\x80\x00", 3);
	TEST_OUTPUT(packmsg_add_int16(&out,     INT16_MAX), "\xd1\xff\x7f", 3);

	TEST_OUTPUT(packmsg_add_int16(&out,            -1), "\xff", 1);
	TEST_OUTPUT(packmsg_add_int16(&out,           -32), "\xe0", 1);
	TEST_OUTPUT(packmsg_add_int16(&out,           -33), "\xd0\xdf", 2);
	TEST_OUTPUT(packmsg_add_int16(&out,      INT8_MIN), "\xd0\x80", 2);
	TEST_OUTPUT(packmsg_add_int16(&out, -1 + INT8_MIN), "\xd1\x7f\xff", 3);
	TEST_OUTPUT(packmsg_add_int16(&out,     INT16_MIN), "\xd1\x00\x80", 3);
}
END_TEST

START_TEST(add_int32)
{
	TEST_OUTPUT(packmsg_add_int32(&out,              0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_int32(&out,       INT8_MAX), "\x7f", 1);
	TEST_OUTPUT(packmsg_add_int32(&out,            128), "\xd1\x80\x00", 3);
	TEST_OUTPUT(packmsg_add_int32(&out,      INT16_MAX), "\xd1\xff\x7f", 3);
	TEST_OUTPUT(packmsg_add_int32(&out,  1 + INT16_MAX), "\xd2\x00\x80\x00\x00", 5);
	TEST_OUTPUT(packmsg_add_int32(&out,      INT32_MAX), "\xd2\xff\xff\xff\x7f", 5);

	TEST_OUTPUT(packmsg_add_int32(&out,             -1), "\xff", 1);
	TEST_OUTPUT(packmsg_add_int32(&out,            -32), "\xe0", 1);
	TEST_OUTPUT(packmsg_add_int32(&out,            -33), "\xd0\xdf", 2);
	TEST_OUTPUT(packmsg_add_int32(&out,       INT8_MIN), "\xd0\x80", 2);
	TEST_OUTPUT(packmsg_add_int32(&out,  -1 + INT8_MIN), "\xd1\x7f\xff", 3);
	TEST_OUTPUT(packmsg_add_int32(&out,      INT16_MIN), "\xd1\x00\x80", 3);
	TEST_OUTPUT(packmsg_add_int32(&out, -1 + INT16_MIN), "\xd2\xff\x7f\xff\xff", 5);
	TEST_OUTPUT(packmsg_add_int32(&out,      INT32_MIN), "\xd2\x00\x00\x00\x80", 5);
}
END_TEST

START_TEST(add_int64)
{
	TEST_OUTPUT(packmsg_add_int64(&out,                0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_int64(&out,         INT8_MAX), "\x7f", 1);
	TEST_OUTPUT(packmsg_add_int64(&out,              128), "\xd1\x80\x00", 3);
	TEST_OUTPUT(packmsg_add_int64(&out,        INT16_MAX), "\xd1\xff\x7f", 3);
	TEST_OUTPUT(packmsg_add_int64(&out,    1 + INT16_MAX), "\xd2\x00\x80\x00\x00", 5);
	TEST_OUTPUT(packmsg_add_int64(&out,        INT32_MAX), "\xd2\xff\xff\xff\x7f", 5);
	TEST_OUTPUT(packmsg_add_int64(&out,  1LL + INT32_MAX), "\xd3\x00\x00\x00\x80\x00\x00\x00\x00", 9);
	TEST_OUTPUT(packmsg_add_int64(&out,        INT64_MAX), "\xd3\xff\xff\xff\xff\xff\xff\xff\x7f", 9);

	TEST_OUTPUT(packmsg_add_int64(&out,               -1), "\xff", 1);
	TEST_OUTPUT(packmsg_add_int64(&out,              -32), "\xe0", 1);
	TEST_OUTPUT(packmsg_add_int64(&out,              -33), "\xd0\xdf", 2);
	TEST_OUTPUT(packmsg_add_int64(&out,         INT8_MIN), "\xd0\x80", 2);
	TEST_OUTPUT(packmsg_add_int64(&out,    -1 + INT8_MIN), "\xd1\x7f\xff", 3);
	TEST_OUTPUT(packmsg_add_int64(&out,        INT16_MIN), "\xd1\x00\x80", 3);
	TEST_OUTPUT(packmsg_add_int64(&out,   -1 + INT16_MIN), "\xd2\xff\x7f\xff\xff", 5);
	TEST_OUTPUT(packmsg_add_int64(&out,        INT32_MIN), "\xd2\x00\x00\x00\x80", 5);
	TEST_OUTPUT(packmsg_add_int64(&out, -1LL + INT32_MIN), "\xd3\xff\xff\xff\x7f\xff\xff\xff\xff", 9);
	TEST_OUTPUT(packmsg_add_int64(&out,        INT64_MIN), "\xd3\x00\x00\x00\x00\x00\x00\x00\x80", 9);
}
END_TEST

START_TEST(add_uint8)
{
	TEST_OUTPUT(packmsg_add_uint8(&out,            0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_uint8(&out,     INT8_MAX), "\x7f", 1);

	TEST_OUTPUT(packmsg_add_uint8(&out, 1 + INT8_MAX), "\xcc\x80", 2);
	TEST_OUTPUT(packmsg_add_uint8(&out,    UINT8_MAX), "\xcc\xff", 2);
}
END_TEST

START_TEST(add_uint16)
{
	TEST_OUTPUT(packmsg_add_uint16(&out,             0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_uint16(&out,      INT8_MAX), "\x7f", 1);

	TEST_OUTPUT(packmsg_add_uint16(&out,  1 + INT8_MAX), "\xcc\x80", 2);
	TEST_OUTPUT(packmsg_add_uint16(&out,     UINT8_MAX), "\xcc\xff", 2);

	TEST_OUTPUT(packmsg_add_uint16(&out, 1 + UINT8_MAX), "\xcd\x00\x01", 3);
	TEST_OUTPUT(packmsg_add_uint16(&out, 1 + INT16_MAX), "\xcd\x00\x80", 3);
	TEST_OUTPUT(packmsg_add_uint16(&out,    UINT16_MAX), "\xcd\xff\xff", 3);
}
END_TEST

START_TEST(add_uint32)
{
	TEST_OUTPUT(packmsg_add_uint32(&out,               0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_uint32(&out,        INT8_MAX), "\x7f", 1);

	TEST_OUTPUT(packmsg_add_uint32(&out,    1 + INT8_MAX), "\xcc\x80", 2);
	TEST_OUTPUT(packmsg_add_uint32(&out,       UINT8_MAX), "\xcc\xff", 2);

	TEST_OUTPUT(packmsg_add_uint32(&out,   1 + UINT8_MAX), "\xcd\x00\x01", 3);
	TEST_OUTPUT(packmsg_add_uint32(&out,   1 + INT16_MAX), "\xcd\x00\x80", 3);

	TEST_OUTPUT(packmsg_add_uint32(&out,  1 + UINT16_MAX), "\xce\x00\x00\x01\x00", 5);
	TEST_OUTPUT(packmsg_add_uint32(&out, 1UL + INT32_MAX), "\xce\x00\x00\x00\x80", 5);
	TEST_OUTPUT(packmsg_add_uint32(&out,      UINT32_MAX), "\xce\xff\xff\xff\xff", 5);
}
END_TEST

START_TEST(add_uint64)
{
	TEST_OUTPUT(packmsg_add_uint64(&out,                 0), "\x00", 1);
	TEST_OUTPUT(packmsg_add_uint64(&out,          INT8_MAX), "\x7f", 1);

	TEST_OUTPUT(packmsg_add_uint64(&out,      1 + INT8_MAX), "\xcc\x80", 2);
	TEST_OUTPUT(packmsg_add_uint64(&out,         UINT8_MAX), "\xcc\xff", 2);

	TEST_OUTPUT(packmsg_add_uint64(&out,     1 + UINT8_MAX), "\xcd\x00\x01", 3);
	TEST_OUTPUT(packmsg_add_uint64(&out,     1 + INT16_MAX), "\xcd\x00\x80", 3);

	TEST_OUTPUT(packmsg_add_uint64(&out,    1 + UINT16_MAX), "\xce\x00\x00\x01\x00", 5);
	TEST_OUTPUT(packmsg_add_uint64(&out,   1UL + INT32_MAX), "\xce\x00\x00\x00\x80", 5);
	TEST_OUTPUT(packmsg_add_uint64(&out,        UINT32_MAX), "\xce\xff\xff\xff\xff", 5);

	TEST_OUTPUT(packmsg_add_uint64(&out, 1ULL + UINT32_MAX), "\xcf\x00\x00\x00\x00\x01\x00\x00\x00", 9);
	TEST_OUTPUT(packmsg_add_uint64(&out,  1ULL + INT64_MAX), "\xcf\x00\x00\x00\x00\x00\x00\x00\x80", 9);
	TEST_OUTPUT(packmsg_add_uint64(&out,        UINT64_MAX), "\xcf\xff\xff\xff\xff\xff\xff\xff\xff", 9);
}
END_TEST

START_TEST(add_float)
{
	TEST_OUTPUT(packmsg_add_float(&out,            0), "\xca\x00\x00\x00\x00", 5);
	TEST_OUTPUT(packmsg_add_float(&out,      FLT_MIN), "\xca\x00\x00\x80\x00", 5);
	TEST_OUTPUT(packmsg_add_float(&out,  FLT_EPSILON), "\xca\x00\x00\x00\x34", 5);
	TEST_OUTPUT(packmsg_add_float(&out,            1), "\xca\x00\x00\x80\x3f", 5);
	TEST_OUTPUT(packmsg_add_float(&out,      FLT_MAX), "\xca\xff\xff\x7f\x7f", 5);

	TEST_OUTPUT(packmsg_add_float(&out,           -1), "\xca\x00\x00\x80\xbf", 5);
	TEST_OUTPUT(packmsg_add_float(&out, FLT_TRUE_MIN), "\xca\x01\x00\x00\x00", 5);

	TEST_OUTPUT(packmsg_add_float(&out,     INFINITY), "\xca\x00\x00\x80\x7f", 5);
	TEST_OUTPUT(packmsg_add_float(&out,    -INFINITY), "\xca\x00\x00\x80\xff", 5);
	TEST_OUTPUT(packmsg_add_float(&out,          NAN), "\xca\x00\x00\xc0\x7f", 5);
}
END_TEST

START_TEST(add_double)
{
	TEST_OUTPUT(packmsg_add_double(&out,            0), "\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_OUTPUT(packmsg_add_double(&out,      DBL_MIN), "\xcb\x00\x00\x00\x00\x00\x00\x10\x00", 9);
	TEST_OUTPUT(packmsg_add_double(&out,  DBL_EPSILON), "\xcb\x00\x00\x00\x00\x00\x00\xb0\x3c", 9);
	TEST_OUTPUT(packmsg_add_double(&out,            1), "\xcb\x00\x00\x00\x00\x00\x00\xf0\x3f", 9);
	TEST_OUTPUT(packmsg_add_double(&out,      DBL_MAX), "\xcb\xff\xff\xff\xff\xff\xff\xef\x7f", 9);

	TEST_OUTPUT(packmsg_add_double(&out,           -1), "\xcb\x00\x00\x00\x00\x00\x00\xf0\xbf", 9);
	TEST_OUTPUT(packmsg_add_double(&out, DBL_TRUE_MIN), "\xcb\x01\x00\x00\x00\x00\x00\x00\x00", 9);

	TEST_OUTPUT(packmsg_add_double(&out,     INFINITY), "\xcb\x00\x00\x00\x00\x00\x00\xf0\x7f", 9);
	TEST_OUTPUT(packmsg_add_double(&out,    -INFINITY), "\xcb\x00\x00\x00\x00\x00\x00\xf0\xff", 9);
	TEST_OUTPUT(packmsg_add_double(&out,          NAN), "\xcb\x00\x00\x00\x00\x00\x00\xf8\x7f", 9);
}
END_TEST

START_TEST(add_str)
{
	TEST_OUTPUT(packmsg_add_str(&out, ""                                ), "\xa0", 1);
	TEST_OUTPUT(packmsg_add_str(&out, ":"                               ), "\xa1:", 2);
	TEST_OUTPUT(packmsg_add_str(&out, ":foo"                            ), "\xa4:foo", 5);
	TEST_OUTPUT(packmsg_add_str(&out, ":123456789abcdef123456789abcdef" ), "\xbf:123456789abcdef123456789abcdef", 32);
	TEST_OUTPUT(packmsg_add_str(&out, ":123456789abcdef123456789abcdef1"), "\xd9\x20:123456789abcdef123456789abcdef1", 34);

	uint8_t *str = (uint8_t *)malloc(0x10000 + 1 + 1 + 4);
	ck_assert_ptr_nonnull(str);

	str[0] = 0xd9;
	str[1] = 0xff;
	memset(str + 2, 'x', 0xff);
	str[2 + 0xff] = 0x00;
	TEST_OUTPUT(packmsg_add_str(&out, (char *)str + 2), str, 2 + 0xff);

	str[0] = 0xda;
	str[1] = 0x00;
	str[2] = 0x01;
	memset(str + 3, 'x', 0x100);
	str[3 + 0x100] = 0x00;
	TEST_OUTPUT(packmsg_add_str(&out, (char *)str + 3), str, 3 + 0x100);

	str[0] = 0xda;
	str[1] = 0xff;
	str[2] = 0xff;
	memset(str + 3, 'x', 0xffff);
	str[3 + 0xffff] = 0x00;
	TEST_OUTPUT(packmsg_add_str(&out, (char *)str + 3), str, 3 + 0xffff);

	str[0] = 0xdb;
	str[1] = 0x00;
	str[2] = 0x00;
	str[3] = 0x01;
	str[4] = 0x00;
	memset(str + 5, 'x', 0x10000);
	str[5 + 0x10000] = 0x00;
	TEST_OUTPUT(packmsg_add_str(&out, (char *)str + 5), str, 5 + 0x10000);

	// TODO: add a test for 4 GB strings?

	free(str);
}
END_TEST

START_TEST(add_bin)
{
	uint8_t *bin = (uint8_t *)calloc(1, 0x10000 + 1 + 4);
	ck_assert_ptr_nonnull(bin);

	bin[0] = 0xc4;
	bin[1] = 0x00;
        TEST_OUTPUT(packmsg_add_bin(&out, bin + 2, 0), bin, 2);

	bin[0] = 0xc4;
	bin[1] = 0x01;
        TEST_OUTPUT(packmsg_add_bin(&out, bin + 2, 0x1), bin, 2 + 0x1);

	bin[0] = 0xc4;
	bin[1] = 0xff;
        TEST_OUTPUT(packmsg_add_bin(&out, bin + 2, 0xff), bin, 2 + 0xff);

	bin[0] = 0xc5;
	bin[1] = 0x00;
	bin[2] = 0x01;
        TEST_OUTPUT(packmsg_add_bin(&out, bin + 3, 0x100), bin, 3 + 0x100);

	bin[0] = 0xc5;
	bin[1] = 0xff;
	bin[2] = 0xff;
        TEST_OUTPUT(packmsg_add_bin(&out, bin + 3, 0xffff), bin, 3 + 0xffff);

	bin[0] = 0xc6;
	bin[1] = 0x00;
	bin[2] = 0x00;
	bin[3] = 0x01;
	bin[4] = 0x00;
        TEST_OUTPUT(packmsg_add_bin(&out, bin + 5, 0x10000), bin, 5 + 0x10000);

	// TODO: add a test case for 4 GB binary data?

	free(bin);
}
END_TEST

START_TEST(add_fixext)
{
	TEST_OUTPUT(packmsg_add_fixext1(&out, 1, ":"), "\xd4\x01:", 3);
	TEST_OUTPUT(packmsg_add_fixext2(&out, 2, ":f"), "\xd5\x02:f", 4);
	TEST_OUTPUT(packmsg_add_fixext4(&out, 3, ":fix"), "\xd6\x03:fix", 6);
	TEST_OUTPUT(packmsg_add_fixext8(&out, 4, ":fixext8"), "\xd7\x04:fixext8", 10);
	TEST_OUTPUT(packmsg_add_fixext16(&out, 5, ":fixext16fixext16"), "\xd8\x05:fixext16fixext16", 18);
}
END_TEST

START_TEST(add_map)
{
	TEST_OUTPUT(packmsg_add_map(&out, 0), "\x80", 1);
	TEST_OUTPUT(packmsg_add_map(&out, 1), "\x81", 1);
	TEST_OUTPUT(packmsg_add_map(&out, 0xf), "\x8f", 1);
	TEST_OUTPUT(packmsg_add_map(&out, 0x10), "\xde\x10\x00", 3);
	TEST_OUTPUT(packmsg_add_map(&out, 0xff), "\xde\xff\x00", 3);
	TEST_OUTPUT(packmsg_add_map(&out, 0x100), "\xde\x00\x01", 3);
	TEST_OUTPUT(packmsg_add_map(&out, 0xffff), "\xde\xff\xff", 3);
	TEST_OUTPUT(packmsg_add_map(&out, 0x10000), "\xdf\x00\x00\x01\x00", 5);
	TEST_OUTPUT(packmsg_add_map(&out, 0xffffffff), "\xdf\xff\xff\xff\xff", 5);
}
END_TEST

START_TEST(add_array)
{
	TEST_OUTPUT(packmsg_add_array(&out, 0), "\x90", 1);
	TEST_OUTPUT(packmsg_add_array(&out, 1), "\x91", 1);
	TEST_OUTPUT(packmsg_add_array(&out, 0xf), "\x9f", 1);
	TEST_OUTPUT(packmsg_add_array(&out, 0x10), "\xdc\x10\x00", 3);
	TEST_OUTPUT(packmsg_add_array(&out, 0xff), "\xdc\xff\x00", 3);
	TEST_OUTPUT(packmsg_add_array(&out, 0x100), "\xdc\x00\x01", 3);
	TEST_OUTPUT(packmsg_add_array(&out, 0xffff), "\xdc\xff\xff", 3);
	TEST_OUTPUT(packmsg_add_array(&out, 0x10000), "\xdd\x00\x00\x01\x00", 5);
	TEST_OUTPUT(packmsg_add_array(&out, 0xffffffff), "\xdd\xff\xff\xff\xff", 5);
}
END_TEST

START_TEST(add_ext)
{
	uint8_t *ext = (uint8_t *)calloc(1, 0x10000 + 2 + 4);
	ck_assert_ptr_nonnull(ext);

	ext[0] = 0xc7;
	ext[1] = 0x00;
	ext[2] = 0x01;
        TEST_OUTPUT(packmsg_add_ext(&out, 1, ext + 3, 0), ext, 3);

	ext[0] = 0xc7;
	ext[1] = 0x01;
	ext[2] = 0x02;
        TEST_OUTPUT(packmsg_add_ext(&out, 2, ext + 3, 0x1), ext, 3 + 0x1);

	ext[0] = 0xc7;
	ext[1] = 0xff;
	ext[2] = 0x03;
        TEST_OUTPUT(packmsg_add_ext(&out, 3, ext + 3, 0xff), ext, 3 + 0xff);

	ext[0] = 0xc8;
	ext[1] = 0x00;
	ext[2] = 0x01;
	ext[3] = 0x04;
        TEST_OUTPUT(packmsg_add_ext(&out, 4, ext + 4, 0x100), ext, 4 + 0x100);

	ext[0] = 0xc8;
	ext[1] = 0xff;
	ext[2] = 0xff;
	ext[3] = 0x05;
        TEST_OUTPUT(packmsg_add_ext(&out, 5, ext + 4, 0xffff), ext, 4 + 0xffff);

	ext[0] = 0xc9;
	ext[1] = 0x00;
	ext[2] = 0x00;
	ext[3] = 0x01;
	ext[4] = 0x00;
	ext[5] = 0x06;
        TEST_OUTPUT(packmsg_add_ext(&out, 6, ext + 6, 0x10000), ext, 6 + 0x10000);

	// TODO: add a test case for 4 GB ext data?

	free(ext);
}
END_TEST

#define TEST_INPUT(statement, buf, size) {\
	struct packmsg_input in = {(uint8_t *)buf, size};\
	statement;\
	ck_assert(packmsg_done(&in));\
}

#define TEST_INPUT_FAILURE(statement, buf, size) {\
	struct packmsg_input in = {(uint8_t *)buf, size};\
	statement;\
	ck_assert(!packmsg_done(&in));\
}

START_TEST(get_nil)
{
	TEST_INPUT(packmsg_get_nil(&in), "\xc0", 1);
}
END_TEST

START_TEST(get_bool)
{
	TEST_INPUT(ck_assert(packmsg_get_bool(&in) == false), "\xc2", 1);
	TEST_INPUT(ck_assert(packmsg_get_bool(&in) == true), "\xc3", 1);
}
END_TEST

START_TEST(get_int8)
{
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==        0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==        1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) == INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==        0), "\xd0\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==        1), "\xd0\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) == INT8_MAX), "\xd0\x7f", 2);

	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==       -1), "\xff", 1);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==      -32), "\xe0", 1);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) ==       -1), "\xd0\xff", 2);
	TEST_INPUT(ck_assert(packmsg_get_int8(&in) == INT8_MIN), "\xd0\x80", 2);

	/* Fail on larger ints or unsigned ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int8(&in) == 0), "\xcc\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int8(&in) == 0), "\xd1\x00\x00", 3);
}
END_TEST

START_TEST(get_int16)
{
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==              0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==              1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==       INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==              0), "\xd0\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==              1), "\xd0\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==       INT8_MAX), "\xd0\x7f", 2);

	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==              0), "\xd1\x00\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==              1), "\xd1\x01\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==       INT8_MAX), "\xd1\x7f\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==   1 + INT8_MAX), "\xd1\x80\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==      INT16_MAX), "\xd1\xff\x7f", 3);

	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==             -1), "\xff", 1);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==            -32), "\xe0", 1);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==             -1), "\xd0\xff", 2);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==       INT8_MIN), "\xd0\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==  -1 + INT8_MIN), "\xd1\x7f\xff", 3);
	TEST_INPUT(ck_assert(packmsg_get_int16(&in) ==      INT16_MIN), "\xd1\x00\x80", 3);

	/* Fail on larger ints or unsigned ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int16(&in) == 0), "\xcc\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int16(&in) == 0), "\xce\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int16(&in) == 0), "\xd2\x00\x00\x00\x00", 5);
}
END_TEST

START_TEST(get_int32)
{
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==       INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              0), "\xd0\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              1), "\xd0\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==       INT8_MAX), "\xd0\x7f", 2);

	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              0), "\xd1\x00\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              1), "\xd1\x01\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==       INT8_MAX), "\xd1\x7f\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==   1 + INT8_MAX), "\xd1\x80\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==      INT16_MAX), "\xd1\xff\x7f", 3);

	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              0), "\xd2\x00\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==              1), "\xd2\x01\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==       INT8_MAX), "\xd2\x7f\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==  1 + INT16_MAX), "\xd2\x00\x80\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==      INT32_MAX), "\xd2\xff\xff\xff\x7f", 5);

	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==             -1), "\xff", 1);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==            -32), "\xe0", 1);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==             -1), "\xd0\xff", 2);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==       INT8_MIN), "\xd0\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==  -1 + INT8_MIN), "\xd1\x7f\xff", 3);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==      INT16_MIN), "\xd1\x00\x80", 3);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) == -1 + INT16_MIN), "\xd2\xff\x7f\xff\xff", 5);
	TEST_INPUT(ck_assert(packmsg_get_int32(&in) ==      INT32_MIN), "\xd2\x00\x00\x00\x80", 5);

	/* Fail on larger ints or unsigned ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int32(&in) == 0), "\xcc\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int32(&in) == 0), "\xce\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int32(&in) == 0), "\xcd\x00\x00\x00\x00", 4);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int32(&in) == 0), "\xd3\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_int64)
{
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==         INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                0), "\xd0\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                1), "\xd0\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==         INT8_MAX), "\xd0\x7f", 2);

	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                0), "\xd1\x00\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                1), "\xd1\x01\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==         INT8_MAX), "\xd1\x7f\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==     1 + INT8_MAX), "\xd1\x80\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==        INT16_MAX), "\xd1\xff\x7f", 3);

	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                0), "\xd2\x00\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                1), "\xd2\x01\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==         INT8_MAX), "\xd2\x7f\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==    1 + INT16_MAX), "\xd2\x00\x80\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==        INT32_MAX), "\xd2\xff\xff\xff\x7f", 5);

	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                0), "\xd3\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==                1), "\xd3\x01\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==         INT8_MAX), "\xd3\x7f\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==    1 + INT16_MAX), "\xd3\x00\x80\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==  1LL + INT32_MAX), "\xd3\x00\x00\x00\x80\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==        INT64_MAX), "\xd3\xff\xff\xff\xff\xff\xff\xff\x7f", 9);

	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==               -1), "\xff", 1);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==              -32), "\xe0", 1);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==               -1), "\xd0\xff", 2);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==         INT8_MIN), "\xd0\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==    -1 + INT8_MIN), "\xd1\x7f\xff", 3);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==        INT16_MIN), "\xd1\x00\x80", 3);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==   -1 + INT16_MIN), "\xd2\xff\x7f\xff\xff", 5);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==        INT32_MIN), "\xd2\x00\x00\x00\x80", 5);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) == -1LL + INT32_MIN), "\xd3\xff\xff\xff\x7f\xff\xff\xff\xff", 9);
	TEST_INPUT(ck_assert(packmsg_get_int64(&in) ==        INT64_MIN), "\xd3\x00\x00\x00\x00\x00\x00\x00\x80", 9);

	/* Fail on unsigned ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int64(&in) == 0), "\xcc\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int64(&in) == 0), "\xce\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int64(&in) == 0), "\xcd\x00\x00\x00\x00", 4);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_int64(&in) == 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_uint8)
{
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==            0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==            1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==     INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==            0), "\xcc\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==            1), "\xcc\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==     INT8_MAX), "\xcc\x7f", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) == 1 + INT8_MAX), "\xcc\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint8(&in) ==    UINT8_MAX), "\xcc\xff", 2);

	/* Fail on larger ints or signed ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint8(&in) == 0), "\xff", 1);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint8(&in) == 0), "\xd0\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint8(&in) == 0), "\xcd\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint8(&in) == 0), "\xce\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint8(&in) == 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_uint16)
{
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==             0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==             1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==      INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==             0), "\xcc\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==             1), "\xcc\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==      INT8_MAX), "\xcc\x7f", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==  1 + INT8_MAX), "\xcc\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==     UINT8_MAX), "\xcc\xff", 2);

	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==             0), "\xcd\x00\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==             1), "\xcd\x01\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==      INT8_MAX), "\xcd\x7f\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==  1 + INT8_MAX), "\xcd\x80\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==     UINT8_MAX), "\xcd\xff\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) == 1 + UINT8_MAX), "\xcd\x00\x01", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==     INT16_MAX), "\xcd\xff\x7f", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) == 1 + INT16_MAX), "\xcd\x00\x80", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint16(&in) ==    UINT16_MAX), "\xcd\xff\xff", 3);

	/* Fail on larger ints or signed ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint16(&in) == 0), "\xff", 1);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint16(&in) == 0), "\xd0\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint16(&in) == 0), "\xd1\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint16(&in) == 0), "\xce\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint16(&in) == 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_uint32)
{
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==             0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==             1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==      INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==             0), "\xcc\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==             1), "\xcc\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==      INT8_MAX), "\xcc\x7f", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==  1 + INT8_MAX), "\xcc\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==     UINT8_MAX), "\xcc\xff", 2);

	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==             0), "\xcd\x00\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==             1), "\xcd\x01\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==      INT8_MAX), "\xcd\x7f\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==  1 + INT8_MAX), "\xcd\x80\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==     UINT8_MAX), "\xcd\xff\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) == 1 + UINT8_MAX), "\xcd\x00\x01", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==     INT16_MAX), "\xcd\xff\x7f", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) == 1 + INT16_MAX), "\xcd\x00\x80", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==    UINT16_MAX), "\xcd\xff\xff", 3);

	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==               0), "\xce\x00\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==               1), "\xce\x01\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==        INT8_MAX), "\xce\x7f\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==    1 + INT8_MAX), "\xce\x80\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==       UINT8_MAX), "\xce\xff\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==   1 + UINT8_MAX), "\xce\x00\x01\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==       INT16_MAX), "\xce\xff\x7f\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==   1 + INT16_MAX), "\xce\x00\x80\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==      UINT16_MAX), "\xce\xff\xff\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==  1 + UINT16_MAX), "\xce\x00\x00\x01\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==       INT32_MAX), "\xce\xff\xff\xff\x7f", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) == 1UL + INT32_MAX), "\xce\x00\x00\x00\x80", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint32(&in) ==      UINT32_MAX), "\xce\xff\xff\xff\xff", 5);

	/* Fail on larger ints or signed ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint32(&in) == 0), "\xff", 1);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint32(&in) == 0), "\xd0\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint32(&in) == 0), "\xd1\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint32(&in) == 0), "\xd2\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint32(&in) == 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_uint64)
{
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==             0), "\x00", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==             1), "\x01", 1);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==      INT8_MAX), "\x7f", 1);

	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==             0), "\xcc\x00", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==             1), "\xcc\x01", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==      INT8_MAX), "\xcc\x7f", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==  1 + INT8_MAX), "\xcc\x80", 2);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==     UINT8_MAX), "\xcc\xff", 2);

	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==             0), "\xcd\x00\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==             1), "\xcd\x01\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==      INT8_MAX), "\xcd\x7f\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==  1 + INT8_MAX), "\xcd\x80\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==     UINT8_MAX), "\xcd\xff\x00", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) == 1 + UINT8_MAX), "\xcd\x00\x01", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==     INT16_MAX), "\xcd\xff\x7f", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) == 1 + INT16_MAX), "\xcd\x00\x80", 3);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==    UINT16_MAX), "\xcd\xff\xff", 3);

	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==               0), "\xce\x00\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==               1), "\xce\x01\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==        INT8_MAX), "\xce\x7f\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==    1 + INT8_MAX), "\xce\x80\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==       UINT8_MAX), "\xce\xff\x00\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==   1 + UINT8_MAX), "\xce\x00\x01\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==       INT16_MAX), "\xce\xff\x7f\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==   1 + INT16_MAX), "\xce\x00\x80\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==      UINT16_MAX), "\xce\xff\xff\x00\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==  1 + UINT16_MAX), "\xce\x00\x00\x01\x00", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==       INT32_MAX), "\xce\xff\xff\xff\x7f", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) == 1UL + INT32_MAX), "\xce\x00\x00\x00\x80", 5);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==      UINT32_MAX), "\xce\xff\xff\xff\xff", 5);

	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==                 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==                 1), "\xcf\x01\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==          INT8_MAX), "\xcf\x7f\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==      1 + INT8_MAX), "\xcf\x80\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==         UINT8_MAX), "\xcf\xff\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==     1 + UINT8_MAX), "\xcf\x00\x01\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==         INT16_MAX), "\xcf\xff\x7f\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==     1 + INT16_MAX), "\xcf\x00\x80\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==        UINT16_MAX), "\xcf\xff\xff\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==    1 + UINT16_MAX), "\xcf\x00\x00\x01\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==         INT32_MAX), "\xcf\xff\xff\xff\x7f\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==   1UL + INT32_MAX), "\xcf\x00\x00\x00\x80\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==        UINT32_MAX), "\xcf\xff\xff\xff\xff\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) == 1ULL + UINT32_MAX), "\xcf\x00\x00\x00\x00\x01\x00\x00\x00", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==         INT64_MAX), "\xcf\xff\xff\xff\xff\xff\xff\xff\x7f", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==  1ULL + INT64_MAX), "\xcf\x00\x00\x00\x00\x00\x00\x00\x80", 9);
	TEST_INPUT(ck_assert(packmsg_get_uint64(&in) ==        UINT64_MAX), "\xcf\xff\xff\xff\xff\xff\xff\xff\xff", 9);

	/* Fail on larger ints or signed ints */
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint64(&in) == 0), "\xff", 1);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint64(&in) == 0), "\xd0\x00", 2);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint64(&in) == 0), "\xd1\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint64(&in) == 0), "\xd2\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert(packmsg_get_uint64(&in) == 0), "\xd3\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_float)
{
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),            0), "\xca\x00\x00\x00\x00", 5);
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),      FLT_MIN), "\xca\x00\x00\x80\x00", 5);
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),  FLT_EPSILON), "\xca\x00\x00\x00\x34", 5);
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),            1), "\xca\x00\x00\x80\x3f", 5);
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),      FLT_MAX), "\xca\xff\xff\x7f\x7f", 5);

	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),           -1), "\xca\x00\x00\x80\xbf", 5);
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in), FLT_TRUE_MIN), "\xca\x01\x00\x00\x00", 5);

	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),     INFINITY), "\xca\x00\x00\x80\x7f", 5);
	TEST_INPUT(ck_assert_float_eq(packmsg_get_float(&in),    -INFINITY), "\xca\x00\x00\x80\xff", 5);
	TEST_INPUT(ck_assert_float_nan(packmsg_get_float(&in)),              "\xca\x00\x00\xc0\x7f", 5);

	/* Fail on ints and doubles */
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\x00", 1);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xcc\x00", 2);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xcd\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xce\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xd0\x00", 2);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xd1\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xd2\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xd3\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT_FAILURE(ck_assert_float_eq(packmsg_get_float(&in), 0), "\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9);
}
END_TEST

START_TEST(get_double)
{
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),            0), "\xcb\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),      DBL_MIN), "\xcb\x00\x00\x00\x00\x00\x00\x10\x00", 9);
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),  DBL_EPSILON), "\xcb\x00\x00\x00\x00\x00\x00\xb0\x3c", 9);
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),            1), "\xcb\x00\x00\x00\x00\x00\x00\xf0\x3f", 9);
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),      DBL_MAX), "\xcb\xff\xff\xff\xff\xff\xff\xef\x7f", 9);

	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),           -1), "\xcb\x00\x00\x00\x00\x00\x00\xf0\xbf", 9);
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in), DBL_TRUE_MIN), "\xcb\x01\x00\x00\x00\x00\x00\x00\x00", 9);

	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),     INFINITY), "\xcb\x00\x00\x00\x00\x00\x00\xf0\x7f", 9);
	TEST_INPUT(ck_assert_double_eq(packmsg_get_double(&in),    -INFINITY), "\xcb\x00\x00\x00\x00\x00\x00\xf0\xff", 9);
	TEST_INPUT(ck_assert_double_nan(packmsg_get_double(&in)),              "\xcb\x00\x00\x00\x00\x00\x00\xf8\x7f", 9);

	/* Fail on ints and float */
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\x00", 1);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xcc\x00", 2);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xcd\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xce\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xcf\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xd0\x00", 2);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xd1\x00\x00", 3);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xd2\x00\x00\x00\x00", 5);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xd3\x00\x00\x00\x00\x00\x00\x00\x00", 9);
	TEST_INPUT_FAILURE(ck_assert_double_eq(packmsg_get_double(&in), 0), "\xca\x00\x00\x00\x00", 5);
}
END_TEST

#define TEST_INPUT_STRING(hdrsize, size, str) {\
	const char *inbuf = (const char *)str;\
\
	struct packmsg_input in1 = {(uint8_t *)inbuf, hdrsize + size};\
	const char *rawptr = NULL;\
	ck_assert_int_eq(packmsg_get_str_raw(&in1, &rawptr), size);\
	ck_assert(packmsg_done(&in1));\
	ck_assert_ptr_nonnull(rawptr);\
	ck_assert_mem_eq(rawptr, inbuf + hdrsize, size);\
\
	struct packmsg_input in2 = {(uint8_t *)inbuf, hdrsize + size};\
	char *dupptr = packmsg_get_str_dup(&in2);\
	ck_assert_ptr_nonnull(dupptr);\
	ck_assert(packmsg_done(&in2));\
	ck_assert_mem_eq(dupptr, inbuf + hdrsize, size);\
	ck_assert(dupptr[size] == 0);\
	free(dupptr);\
\
	struct packmsg_input in3 = {(uint8_t *)inbuf, hdrsize + size};\
	char outbuf[size + 1 + 64];\
	memcpy(outbuf + size + 1, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_str_copy(&in3, &outbuf, size + 1), size);\
	ck_assert(packmsg_done(&in3));\
	ck_assert_mem_eq(outbuf, inbuf + hdrsize, size);\
	ck_assert(outbuf[size] == 0);\
	ck_assert_mem_eq(outbuf + size + 1, "Canary!", 8);\
\
	struct packmsg_input in4 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	ck_assert_int_eq(packmsg_get_str_raw(&in4, &rawptr), 0);\
	ck_assert_ptr_null(rawptr);\
	ck_assert(!packmsg_done(&in4));\
\
	struct packmsg_input in5 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	dupptr = packmsg_get_str_dup(&in5);\
	ck_assert_ptr_null(dupptr);\
	ck_assert(!packmsg_done(&in5));\
\
	struct packmsg_input in6 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	memcpy(outbuf, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_str_copy(&in6, &outbuf, size), 0);\
	ck_assert(!packmsg_done(&in6));\
	ck_assert_mem_eq(outbuf, size ? "\0anary!" : "Canary!", 8);\
\
	if (size) {\
		struct packmsg_input in7 = {(uint8_t *)inbuf, hdrsize + size};\
		memcpy(outbuf, "Canary!", 8);\
		ck_assert_int_eq(packmsg_get_str_copy(&in7, &outbuf, size - 1), 0);\
		ck_assert(!packmsg_done(&in7));\
		ck_assert_mem_eq(outbuf, (size - 1) ? "\0anary!" : "Canary!", 8);\
	}\
}

START_TEST(get_str)
{
	TEST_INPUT_STRING(1, 0, "\xa0");
	TEST_INPUT_STRING(1, 1, "\xa1:");
	TEST_INPUT_STRING(1, 4, "\xa4:foo");
	TEST_INPUT_STRING(1, 31, "\xbf:123456789abcdef123456789abcdef");
	TEST_INPUT_STRING(2, 32, "\xd9\x20:123456789abcdef123456789abcdef1");

	uint8_t *str = (uint8_t *)malloc(0x10000 + 1 + 1 + 4);
	ck_assert_ptr_nonnull(str);

	str[0] = 0xd9;
	str[1] = 0xff;
	memset(str + 2, 'x', 0xff);
	str[2 + 0xff] = 0x00;
	TEST_INPUT_STRING(2, 0xff, str);

	str[0] = 0xda;
	str[1] = 0x00;
	str[2] = 0x01;
	memset(str + 3, 'x', 0x100);
	str[3 + 0x100] = 0x00;
	TEST_INPUT_STRING(3, 0x100, str);

	str[0] = 0xda;
	str[1] = 0xff;
	str[2] = 0xff;
	memset(str + 3, 'x', 0xffff);
	str[3 + 0xffff] = 0x00;
	TEST_INPUT_STRING(3, 0xffff, str);

	str[0] = 0xdb;
	str[1] = 0x00;
	str[2] = 0x00;
	str[3] = 0x01;
	str[4] = 0x00;
	memset(str + 5, 'x', 0x10000);
	str[5 + 0x10000] = 0x00;
	TEST_INPUT_STRING(5, 0x10000, str);

	free(str);
}
END_TEST

#define TEST_INPUT_BIN(hdrsize, size, bin) {\
	const char *inbuf = (const char *)bin;\
\
	struct packmsg_input in1 = {(uint8_t *)inbuf, hdrsize + size};\
	const void *rawptr = NULL;\
	ck_assert_int_eq(packmsg_get_bin_raw(&in1, &rawptr), size);\
	ck_assert(packmsg_done(&in1));\
	ck_assert_ptr_nonnull(rawptr);\
	ck_assert_mem_eq(rawptr, inbuf + hdrsize, size);\
\
	struct packmsg_input in2 = {(uint8_t *)inbuf, hdrsize + size};\
	void *dupptr = packmsg_get_bin_dup(&in2);\
	ck_assert_ptr_nonnull(dupptr);\
	ck_assert(packmsg_done(&in2));\
	ck_assert_mem_eq(dupptr, inbuf + hdrsize, size);\
	free(dupptr);\
\
	struct packmsg_input in3 = {(uint8_t *)inbuf, hdrsize + size};\
	char outbuf[size + 64];\
	memcpy(outbuf + size, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_bin_copy(&in3, &outbuf, size + 1), size);\
	ck_assert(packmsg_done(&in3));\
	ck_assert_mem_eq(outbuf, inbuf + hdrsize, size);\
	ck_assert_mem_eq(outbuf + size, "Canary!", 8);\
\
	struct packmsg_input in4 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	ck_assert_int_eq(packmsg_get_bin_raw(&in4, &rawptr), 0);\
	ck_assert_ptr_null(rawptr);\
	ck_assert(!packmsg_done(&in4));\
\
	struct packmsg_input in5 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	dupptr = packmsg_get_bin_dup(&in5);\
	ck_assert_ptr_null(dupptr);\
	ck_assert(!packmsg_done(&in5));\
\
	struct packmsg_input in6 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	memcpy(outbuf, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_bin_copy(&in6, &outbuf, size), 0);\
	ck_assert(!packmsg_done(&in6));\
	ck_assert_mem_eq(outbuf, "Canary!", 8);\
\
	if (size) {\
		struct packmsg_input in7 = {(uint8_t *)inbuf, hdrsize + size};\
		memcpy(outbuf, "Canary!", 8);\
		ck_assert_int_eq(packmsg_get_bin_copy(&in7, &outbuf, size - 1), 0);\
		ck_assert(!packmsg_done(&in7));\
		ck_assert_mem_eq(outbuf, "Canary!", 8);\
	}\
}

START_TEST(get_bin)
{
	uint8_t *bin = (uint8_t *)calloc(1, 0x10000 + 1 + 4);
	ck_assert_ptr_nonnull(bin);

	bin[0] = 0xc4;
	bin[1] = 0x00;
        TEST_INPUT_BIN(2, 0, bin);

	bin[0] = 0xc4;
	bin[1] = 0x01;
        TEST_INPUT_BIN(2, 0x1, bin);

	bin[0] = 0xc4;
	bin[1] = 0xff;
        TEST_INPUT_BIN(2, 0xff, bin);

	bin[0] = 0xc5;
	bin[1] = 0x00;
	bin[2] = 0x01;
        TEST_INPUT_BIN(3, 0x100, bin);

	bin[0] = 0xc5;
	bin[1] = 0xff;
	bin[2] = 0xff;
        TEST_INPUT_BIN(3, 0xffff, bin);

	bin[0] = 0xc6;
	bin[1] = 0x00;
	bin[2] = 0x00;
	bin[3] = 0x01;
	bin[4] = 0x00;
        TEST_INPUT_BIN(5, 0x10000, bin);

	free(bin);
}
END_TEST

#define TEST_INPUT_EXT(hdrsize, expected, size, bin) {\
	const char *inbuf = (const char *)bin;\
	int8_t type;\
\
	struct packmsg_input in1 = {(uint8_t *)inbuf, hdrsize + size};\
	const void *rawptr = NULL;\
	ck_assert_int_eq(packmsg_get_ext_raw(&in1, &type, &rawptr), size);\
	ck_assert(packmsg_done(&in1));\
	ck_assert_int_eq(type, expected);\
	ck_assert_ptr_nonnull(rawptr);\
	ck_assert_mem_eq(rawptr, inbuf + hdrsize, size);\
\
	struct packmsg_input in2 = {(uint8_t *)inbuf, hdrsize + size};\
	void *dupptr = packmsg_get_ext_dup(&in2, &type);\
	ck_assert(packmsg_done(&in2));\
	ck_assert_int_eq(type, expected);\
	ck_assert_ptr_nonnull(dupptr);\
	ck_assert_mem_eq(dupptr, inbuf + hdrsize, size);\
	free(dupptr);\
\
	struct packmsg_input in3 = {(uint8_t *)inbuf, hdrsize + size};\
	char outbuf[size + 64];\
	memcpy(outbuf + size, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_ext_copy(&in3, &type, &outbuf, size + 1), size);\
	ck_assert(packmsg_done(&in3));\
	ck_assert_int_eq(type, expected);\
	ck_assert_mem_eq(outbuf, inbuf + hdrsize, size);\
	ck_assert_mem_eq(outbuf + size, "Canary!", 8);\
\
	struct packmsg_input in4 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	ck_assert_int_eq(packmsg_get_ext_raw(&in4, &type, &rawptr), 0);\
	ck_assert_ptr_null(rawptr);\
	ck_assert_int_eq(type, 0);\
	ck_assert(!packmsg_done(&in4));\
\
	struct packmsg_input in5 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	dupptr = packmsg_get_ext_dup(&in5, &type);\
	ck_assert_ptr_null(dupptr);\
	ck_assert(!packmsg_done(&in5));\
\
	struct packmsg_input in6 = {(uint8_t *)inbuf, hdrsize + size - 1};\
	memcpy(outbuf, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_ext_copy(&in6, &type, &outbuf, size), 0);\
	ck_assert_int_eq(type, 0);\
	ck_assert(!packmsg_done(&in6));\
	ck_assert_mem_eq(outbuf, "Canary!", 8);\
\
	if (size) {\
		struct packmsg_input in7 = {(uint8_t *)inbuf, hdrsize + size};\
		memcpy(outbuf, "Canary!", 8);\
		ck_assert_int_eq(packmsg_get_ext_copy(&in7, &type, &outbuf, size - 1), 0);\
		ck_assert_int_eq(type, 0);\
		ck_assert(!packmsg_done(&in7));\
		ck_assert_mem_eq(outbuf, "Canary!", 8);\
	}\
}

START_TEST(get_ext)
{
	uint8_t *ext = (uint8_t *)calloc(1, 0x10000 + 2 + 4);
	ck_assert_ptr_nonnull(ext);

	ext[0] = 0xc7;
	ext[1] = 0x00;
	ext[2] = 0x01;
        TEST_INPUT_EXT(3, 1, 0, ext);

	ext[0] = 0xc7;
	ext[1] = 0x01;
	ext[2] = 0x02;
        TEST_INPUT_EXT(3, 2, 0x1, ext);

	ext[0] = 0xc7;
	ext[1] = 0xff;
	ext[2] = 0x03;
        TEST_INPUT_EXT(3, 3, 0xff, ext);

	ext[0] = 0xc8;
	ext[1] = 0x00;
	ext[2] = 0x01;
	ext[3] = 0x04;
        TEST_INPUT_EXT(4, 4, 0x100, ext);

	ext[0] = 0xc8;
	ext[1] = 0xff;
	ext[2] = 0xff;
	ext[3] = 0x05;
        TEST_INPUT_EXT(4, 5, 0xffff, ext);

	ext[0] = 0xc9;
	ext[1] = 0x00;
	ext[2] = 0x00;
	ext[3] = 0x01;
	ext[4] = 0x00;
	ext[5] = 0x06;
        TEST_INPUT_EXT(6, 6, 0x10000, ext);

	free(ext);
}
END_TEST

#define TEST_INPUT_FIXEXT(size, ext, data) {\
	const char *buf = data;\
	struct packmsg_input in1 = {(uint8_t *)buf, size + 2};\
	char outbuf[size + 64];\
	memcpy(outbuf + size, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_fixext ## size(&in1, outbuf), ext);\
	ck_assert(packmsg_done(&in1));\
	ck_assert_mem_eq(outbuf, buf + 2, size);\
	ck_assert_mem_eq(outbuf + size, "Canary!", 8);\
	struct packmsg_input in2 = {(uint8_t *)buf, size + 1};\
	memcpy(outbuf, "Canary!", 8);\
	ck_assert_int_eq(packmsg_get_fixext ## size(&in2, outbuf), 0);\
	ck_assert_mem_eq(outbuf, "Canary!", 8);\
	ck_assert(!packmsg_done(&in2));\
}

START_TEST(get_fixext)
{
	TEST_INPUT_FIXEXT(1,  1, "\xd4\x01:");
	TEST_INPUT_FIXEXT(2,  2, "\xd5\x02:f");
	TEST_INPUT_FIXEXT(4,  3, "\xd6\x03:fix");
	TEST_INPUT_FIXEXT(8,  4, "\xd7\x04:fixext8");
	TEST_INPUT_FIXEXT(16, 5, "\xd8\x05:fixext16fixext16");
}
END_TEST

START_TEST(get_map)
{
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),          0), "\x80", 1)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),          1), "\x81", 1)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),        0xf), "\x8f", 1)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),          0), "\xde\x00\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),          1), "\xde\x01\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),        0xf), "\xde\x0f\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),       0x10), "\xde\x10\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),       0xff), "\xde\xff\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),      0x100), "\xde\x00\x01", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),     0xffff), "\xde\xff\xff", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),          0), "\xdf\x00\x00\x00\x00", 5)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),      0x100), "\xdf\x00\x01\x00\x00", 5)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in),    0x10000), "\xdf\x00\x00\x01\x00", 5)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_map(&in), 0xffffffff), "\xdf\xff\xff\xff\xff", 5)
}
END_TEST

START_TEST(get_array)
{
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),          0), "\x90", 1)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),          1), "\x91", 1)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),        0xf), "\x9f", 1)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),          0), "\xdc\x00\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),          1), "\xdc\x01\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),        0xf), "\xdc\x0f\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),       0x10), "\xdc\x10\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),       0xff), "\xdc\xff\x00", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),      0x100), "\xdc\x00\x01", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),     0xffff), "\xdc\xff\xff", 3)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),          0), "\xdd\x00\x00\x00\x00", 5)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),      0x100), "\xdd\x00\x01\x00\x00", 5)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in),    0x10000), "\xdd\x00\x00\x01\x00", 5)
	TEST_INPUT(ck_assert_int_eq(packmsg_get_array(&in), 0xffffffff), "\xdd\xff\xff\xff\xff", 5)
}
END_TEST

int main(void)
{
	Suite *s = suite_create("packmsg");
	SRunner *sr = srunner_create(s);

	TCase *tc_add = tcase_create("add");
	{
		tcase_add_test(tc_add, add_nil);
		tcase_add_test(tc_add, add_bool);

		tcase_add_test(tc_add, add_int8);
		tcase_add_test(tc_add, add_int16);
		tcase_add_test(tc_add, add_int32);
		tcase_add_test(tc_add, add_int64);

		tcase_add_test(tc_add, add_uint8);
		tcase_add_test(tc_add, add_uint16);
		tcase_add_test(tc_add, add_uint32);
		tcase_add_test(tc_add, add_uint64);

		tcase_add_test(tc_add, add_float);
		tcase_add_test(tc_add, add_double);
		tcase_add_test(tc_add, add_str);
		tcase_add_test(tc_add, add_bin);
		tcase_add_test(tc_add, add_ext);
		tcase_add_test(tc_add, add_fixext);
		tcase_add_test(tc_add, add_map);
		tcase_add_test(tc_add, add_array);
	}
	suite_add_tcase(s, tc_add);

	TCase *tc_get = tcase_create("get");
	{
		tcase_add_test(tc_get, get_nil);
		tcase_add_test(tc_get, get_bool);

		tcase_add_test(tc_get, get_int8);
		tcase_add_test(tc_get, get_int16);
		tcase_add_test(tc_get, get_int32);
		tcase_add_test(tc_get, get_int64);

		tcase_add_test(tc_get, get_uint8);
		tcase_add_test(tc_get, get_uint16);
		tcase_add_test(tc_get, get_uint32);
		tcase_add_test(tc_get, get_uint64);

		tcase_add_test(tc_get, get_float);
		tcase_add_test(tc_get, get_double);
		tcase_add_test(tc_get, get_str);
		tcase_add_test(tc_get, get_bin);
		tcase_add_test(tc_get, get_ext);
		tcase_add_test(tc_get, get_fixext);
		tcase_add_test(tc_get, get_map);
		tcase_add_test(tc_get, get_array);
	}
	suite_add_tcase(s, tc_get);

	srunner_run_all(sr, CK_NORMAL);
	int failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return failed;
}
