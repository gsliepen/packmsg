#pragma once

#include <benchmark/benchmark.h>

void msgpack_encode_nil(benchmark::State &state);
void msgpack_decode_nil(benchmark::State &state);
void msgpack_encode_hello(benchmark::State &state);
void msgpack_decode_hello(benchmark::State &state);
