#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "packmsg.h"

int main()
{
	uint8_t buf[1024];

	/* Example encoding */

	struct packmsg_output out = {buf, sizeof buf};

	packmsg_add_map(&out, 2);
	packmsg_add_str(&out, "compact");
	packmsg_add_bool(&out, true);
	packmsg_add_str(&out, "schema");
	packmsg_add_int32(&out, 0);

	assert(packmsg_output_ok(&out));

	size_t len = packmsg_output_size(&out, buf);
	printf("Buffer (%zu bytes): ", len);
	for (size_t i = 0; i < len; i++)
		printf("%02x", buf[i]);
	printf("\n");

	/* Example decoding */

	struct packmsg_input in = {buf, len};

	uint32_t count = packmsg_get_map(&in);

	printf("{");
	for (uint32_t i = 0; i < count; i++) {
		if (i)
			printf(", ");

		const char *str;
		uint32_t len = packmsg_get_str_raw(&in, &str);

		if (packmsg_is_bool(&in)) {
			bool val = packmsg_get_bool(&in);
			printf("\"%.*s\": %s", len, str, val ? "true" : "false");
		} else {
			int32_t val = packmsg_get_int32(&in);
			printf("\"%.*s\": %d", len, str, val);
		}
	}
	printf("}\n");

	assert(packmsg_done(&in));
}
