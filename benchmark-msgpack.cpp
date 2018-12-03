#include "benchmark-msgpack.h"

#include <msgpack.h>

void msgpack_encode_nil(benchmark::State &state) {
	msgpack_sbuffer sbuf;
	msgpack_sbuffer_init(&sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	for (auto _: state) {
		msgpack_sbuffer_clear(&sbuf);

		msgpack_pack_nil(&pk);

		assert(sbuf.size == 1);
		benchmark::ClobberMemory();
	}

	msgpack_sbuffer_destroy(&sbuf);
}

void msgpack_decode_nil(benchmark::State &state) {
	const uint8_t buf[1] = {0xc0};

	msgpack_unpacker unp;
	msgpack_unpacker_init(&unp, 1024);
	memcpy(msgpack_unpacker_buffer(&unp), buf, sizeof buf);
	msgpack_unpacker_buffer_consumed(&unp, sizeof buf);

	msgpack_unpacked und;
	msgpack_unpacked_init(&und);

	for (auto _: state) {
		msgpack_unpacker_reset(&unp);

		msgpack_unpacker_next(&unp, &und);
		assert(und.data.type == MSGPACK_OBJECT_NIL);

		benchmark::ClobberMemory();
	}

	msgpack_unpacked_destroy(&und);
	msgpack_unpacker_destroy(&unp);
}

void msgpack_encode_hello(benchmark::State &state) {
	msgpack_sbuffer sbuf;
	msgpack_sbuffer_init(&sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	const char *str1 = "compact";
	const char *str2 = "schema";

	for (auto _: state) {
		msgpack_sbuffer_clear(&sbuf);

		msgpack_pack_array(&pk, 2);
		msgpack_pack_str(&pk, strlen(str1));
		msgpack_pack_str_body(&pk, str1, strlen(str1));
		msgpack_pack_true(&pk);
		msgpack_pack_str(&pk, strlen(str2));
		msgpack_pack_str_body(&pk, str2, strlen(str2));
		msgpack_pack_int(&pk, 0);

		assert(sbuf.size == 18);
		benchmark::ClobberMemory();
	}

	msgpack_sbuffer_destroy(&sbuf);
}

void msgpack_decode_hello(benchmark::State &state) {
	const uint8_t buf[18] = "\x82\xa7" "compact" "\xc3\xa6" "schema";

	msgpack_unpacked und;
	msgpack_unpacked_init(&und);

	for (auto _: state) {
		// TODO: can we move the unpacker out of the loop?
		msgpack_unpacker unp;
		msgpack_unpacker_init(&unp, 1024);
		memcpy(msgpack_unpacker_buffer(&unp), buf, sizeof buf);
		msgpack_unpacker_buffer_consumed(&unp, sizeof buf);

		// TODO: how to unpack the contents of the array?
		msgpack_unpacker_next(&unp, &und);
		assert(und.data.type == MSGPACK_OBJECT_MAP);
		
		msgpack_unpacker_destroy(&unp);
		benchmark::ClobberMemory();
	}

	msgpack_unpacked_destroy(&und);
}
