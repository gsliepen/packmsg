# PackMessage, a safe and fast header-only C library for little-endian MessagePack encoding and decoding

This library can encode and decode MessagePack objects, however it differs in one important point
from the official MessagePack specification: PackMessage stores all values in little-endian format.
PackMessage offers a simple streaming API for encoding and decoding.

PackMessage is *safe*:

* Reads from and writes to buffers are always bounds checked.
* String, binary and extension data can be read into buffers allocated by PackMessage using simple API calls.
* Any error will result in null values and pointers being returned, and/or application-allocated buffers for strings will be zero-terminated, so there is no undefined state.
* Once an encoding/decoding error occurs, all subsequent operations on the same buffer will also fail.
* The API is designed to follow the principle of least surprise, and makes it hard to use in a wrong way.

PackMessage is *fast*:

* Values are stored in little-endian format, since virtually all mainstream processors are little-endian, or they can switch between endianness and are probably running an operating system that has configured it to be little-endian. This saves the overhead of converting to and from big-endian format.
* No memory allocation is done unless requested.
* The application can get const pointers to string, binary and extension data pointing into the input buffer if desired, avoiding copies.
* The application does not have to check for errors after for every operation; it can be done once after encoding/decoding a buffer if desired.
* The library is header-only, allowing the compiler to inline all functions and better optimize your application.

## Usage

Copy packmsg.h to your project, and `#include "packmsg.h"` in your code.
See the include `example.c` for a quick demonstration of how to encode and decode.
Full documentation TBD.

## TODO

This is a work in progress. While PackMessage supports all features of the MessagePack format, there is still room for improvement:

* API documentation
* More elaborate examples
* C++ wrapper
* Benchmark more elaborate cases
* Benchmark other libraries for comparison
* Check portability
* Improve the build system
