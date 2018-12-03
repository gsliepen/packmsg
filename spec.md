# PackMessage specification

PackMessage is very similar to MessagePack, but stores data in little-endian
format. While big-endian has traditionally been the way to transmit data over
the network, almost all CPU architectures in use at this moment in time are
little-endian or support the little-endian format. Therefore, we can avoid
wasting CPU time by not reordering bytes.

The C API of the official PackMessage library is rather hard to use due to all
the memory management it wants to do. This also makes that library very slow.
In contrast, PackMessage does as little memory management as possible. The only
exception is that has functions for reading strings and binary data that return
a pointer to memory allocated by PackMessage.

# PackMessage API

PackMessage operates primarily on a buffer of a given size that is provided by
the application. All API calls take a pointer to a transparent struct that
holds a pointer into that buffer, and the remaining size. While reading from or
writing to a buffer, PackMessage increments the buffer pointer and decrements
the size. It ensures it will never read or write past the end of the buffer. It
sets the size to -1 whenever there was an error reading from or writing to the
buffer, which also prevents any further operations to fail.

When an error occurs during a function that reads a value, PackMessage always
returns the equivalent of 0. However, this is often a valid value. To check
whether a valid read was done, one should call `packmsg_is_ok()`. To check that
the whole buffer was read correctly at the end of all read calls, use
`packmsg_is_done()`. This scheme allows one to reduce the amount of error
checking.

Example encoding usage:

```c
#include <packmsg.h>

uint8_t buf[100];

struct packmsg_output out = {buf, sizeof buf};

packmsg_add_map(&out, 2);
packmsg_add_str(&out, "compact");
packmsg_add_bool(&out, true);
packmsg_add_str(&out, "schema");
packmsg_add_int32(&out, 0);

assert(packmsg_is_ok(&out));

printf("Bytes written: %zu\n", packmsg_output_size(&out, buf));
```

Example decoding usage, using the buffer generated above:

```c
struct packmsg_input in = {buf, sizeof buf};

uint32_t count = packmsg_get_map(&in);

printf("{");
for (int i = 0; i < count; i++) {
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

	free(str);
}
printf("}\n");

assert(packmsg_is_done(&in));
```


