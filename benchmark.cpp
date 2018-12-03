#include <benchmark/benchmark.h>

#include "benchmark-packmsg.h"
#include "benchmark-msgpack.h"
#include "benchmark-printf.h"

BENCHMARK(packmsg_encode_nil);
BENCHMARK(packmsg_decode_nil);
BENCHMARK(packmsg_encode_hello);
BENCHMARK(packmsg_decode_hello);

BENCHMARK(msgpack_encode_nil);
BENCHMARK(msgpack_decode_nil);
BENCHMARK(msgpack_encode_hello);
BENCHMARK(msgpack_decode_hello);

BENCHMARK(printf_encode_hello);
BENCHMARK(printf_decode_hello);

BENCHMARK_MAIN();
