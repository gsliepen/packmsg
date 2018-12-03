#pragma once

#include <benchmark/benchmark.h>

void packmsg_encode_nil(benchmark::State &state);
void packmsg_decode_nil(benchmark::State &state);
void packmsg_encode_hello(benchmark::State &state);
void packmsg_decode_hello(benchmark::State &state);
