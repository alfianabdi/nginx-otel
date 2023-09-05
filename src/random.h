// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <opentelemetry/nostd/span.h>
#include "fast_random_number_generator.h"

class Random
{
public:
  /**
   * @return an unsigned 64 bit random number
   */
  static uint64_t GenerateRandom64() noexcept;
  /**
   * Fill the passed span with random bytes.
   *
   * @param buffer A span of bytes.
   */
  static void GenerateRandomBuffer(opentelemetry::nostd::span<uint8_t> buffer) noexcept;

private:
  /**
   * @return a seeded thread-local random number generator.
   */
  static FastRandomNumberGenerator &GetRandomNumberGenerator() noexcept;
};
