// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <opentelemetry/version.h>

int AtFork(void (*prepare)(), void (*parent)(), void (*child)()) noexcept;
