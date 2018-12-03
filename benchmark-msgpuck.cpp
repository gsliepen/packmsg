#include "benchmark-packmsg.h"

#include "packmsg.h"

void packmsg_encode_nil(benchmark::State &state) {
	uint8_t buf[1];

	for (auto _: state) {
		struct packmsg_output out = {buf, sizeof buf};

		packmsg_add_nil(&out);

		assert(packmsg_is_ok(&out));
		benchmark::ClobberMemory();
	}
}

void packmsg_decode_nil(benchmark::State &state) {
	const uint8_t buf[1] = {0xc0};

	for (auto _: state) {
		struct packmsg_input in = {buf, sizeof buf};

		packmsg_get_nil(&in);

		assert(packmsg_is_done(&in));
		benchmark::ClobberMemory();
	}
}

void packmsg_encode_hello(benchmark::State &state) {
	uint8_t buf[18];

	for (auto _: state) {
		struct packmsg_output out = {buf, sizeof buf};

		packmsg_add_array(&out, 2);
		packmsg_add_str(&out, "compact");
		packmsg_add_bool(&out, true);
		packmsg_add_str(&out, "schema");
		packmsg_add_int32(&out, 0);

		assert(packmsg_is_ok(&out));
		benchmark::ClobberMemory();
	}
}

void packmsg_decode_hello(benchmark::State &state) {
	const uint8_t buf[18] = "\x82\xa7" "compact" "\xc3\xa6" "schema";

	for (auto _: state) {
		struct packmsg_input in = {buf, sizeof buf};
		const char *key1;
		const char *key2;

		packmsg_get_map(&in);
		packmsg_get_str_raw(&in, &key1);
		packmsg_get_bool(&in);
		packmsg_get_str_raw(&in, &key2);
		packmsg_get_int32(&in);
		
		assert(packmsg_is_done(&in));
		benchmark::ClobberMemory();
	}
}
