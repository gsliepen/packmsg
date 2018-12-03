#include "benchmark-printf.h"

#include "packmsg.h"

void printf_encode_hello(benchmark::State &state) {
	char buf[31];

	for (auto _: state) {
		int result = snprintf(buf, sizeof buf, "{\"%s\": %s, \"%s\": %d}", "compact", true ? "true" : "false", "schema", 0);

		assert(result == 30);
		benchmark::ClobberMemory();
	}
}

void printf_decode_hello(benchmark::State &state) {
	const char buf[] = "{\"compact\": true, \"schema\": 0}";

	for (auto _: state) {
		char key1[32];
		char val1[32];
		char key2[32];
		int val2;

		int result = sscanf(buf, "{\"%[^\"]\": %[^,], \"%[^\"]\": %d}", key1, val1, key2, &val2);
		
		assert(result == 4);
		benchmark::ClobberMemory();
	}
}
