// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include "fork.h"

#include <pthread.h>

int AtFork(void (*prepare)(), void (*parent)(), void (*child)()) noexcept
{
  return ::pthread_atfork(prepare, parent, child);
}
